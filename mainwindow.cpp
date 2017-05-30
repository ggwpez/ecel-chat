#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QShortcut>

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow), other_client(nullptr),
	is_server(false), is_client(is_server)
{
	ui->setupUi(this);
	ui->textBrowser->setStyleSheet("QTextBrowser { color: rgb(84, 165, 196); background: rgb(24, 24, 24); selection-background-color: rgb(25, 55, 84); }");
	ui->textEdit   ->setStyleSheet("QTextEdit    { color: rgb(84, 165, 196); background: rgb(38, 38, 38); selection-background-color: rgb(25, 55, 84); }");

	new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));
	QObject::connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_B), this), &QShortcut::activated, [=]()
	{
		if (window()->windowFlags() & Qt::FramelessWindowHint)
			window()->setWindowFlags(this->windowFlags() & ~Qt::FramelessWindowHint);
		else
			window()->setWindowFlags(Qt::FramelessWindowHint);
		this->window()->show();
	});
	QObject::connect(new QShortcut(QKeySequence(Qt::Key_F11), this), &QShortcut::activated, [=]()
	{ window()->setWindowState(window()->windowState() ^ Qt::WindowFullScreen); });
	QObject::connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_M), this), &QShortcut::activated, [=]()
	{ window()->showMinimized(); });
	qApp->installEventFilter(this);

	client = new QTcpSocket();
	server = new QTcpServer();

	client->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
	connect(client, SIGNAL(readyRead()), this, SLOT(client_data_ready()));
	connect(client, SIGNAL(disconnected()), this, SLOT(client_disconnected()));
	connect(server, SIGNAL(newConnection()), this, SLOT(connected()));
}

QString const me("THIS"), he("THEE"),
			  me_clr(""), he_clr("LightGreen");
int my_pos, my_kid;
QString my_key = "", he_key = "";

void MainWindow::start_server(QString host, int port)
{
	if (this->is_server)
	{
		QMessageBox::critical(this, tr("Network Error"),
									tr("Program is in client state, client and server state are Exclusiv"));
		return;
	}


	is_server = true;
	if (! server->listen(QHostAddress(host), port))
	{
		QMessageBox::critical(this, tr("Network Error"),
									tr("Unable to start the server: %1.")
									.arg(server->errorString()));
		return;
	}
	else
		printl("Server started");
}

void MainWindow::stop_server()
{
	if (is_server)
	{
		other_client->close();
		server->close();
		is_server = false;
	}
}

void MainWindow::start_client(QString server, int port)
{
	if (this->is_server)
	{
		QMessageBox::critical(this, tr("Network Error"),
									tr("Program is in server state, server and client state are Exclusiv"));
		return;
	}

	is_client = true;
	client->connectToHost(server, port, QIODevice::ReadWrite);

	if(client->waitForConnected(7500))
		printl("Connected");
	else
		printl("Couldnt connect");
}

void MainWindow::stop_client()
{
	if (is_client)
	{
		client->disconnectFromHost();
		client->close();
		is_client = false;
		printl("Client stopped");
	}
}

void MainWindow::connected()
{
	if (this->other_client)
	{
		QMessageBox::critical(this, tr("Network Error"),
									tr("More than one client is not supported"));

		return;
	}

	if ((this->other_client = server->nextPendingConnection()))
	{
		connect(other_client, SIGNAL(readyRead()), this, SLOT(client_data_ready()));
		connect(other_client, SIGNAL(disconnected()), this, SLOT(server_disconnected()));

		printl_me("connected");
		send("Hi from Server");
	}
	else
		printl("Error (this->other_client = server->nextPendingConnection()) == nullptr");
}

void MainWindow::server_disconnected()
{
	QTcpSocket* disco_socket = qobject_cast<QTcpSocket *>(QObject::sender());

	if (disco_socket != other_client)
	{
		QMessageBox::critical(this, tr("Internal State Error"),
									tr("Unknown client disconnected"));

		abort();
		return;
	}

	printl_he("disconnected");
	disco_socket->deleteLater();
}

void MainWindow::client_disconnected()
{
	//QTcpSocket* disco_socket = qobject_cast<QTcpSocket *>(QObject::sender());

	printl("Disconnected from Server");
	is_client = false;
}

void MainWindow::resizeEvent(QResizeEvent* e)
{
	this->QWidget::resizeEvent(e);
	int w(e->size().width()), th(e->size().height() *0.85f);

	ui->textBrowser->setGeometry(0, 0, w, th);
	ui->textEdit->setGeometry(0, th, w, e->size().height() -th);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* e)
{
	if (obj == ui->textEdit)
	{
		if (e->type() == QEvent::KeyPress)
			return press_le_key(static_cast<QKeyEvent*>(e));
		else
			return QObject::eventFilter(obj, e);
	}
	else
		return QObject::eventFilter(obj, e);
}

bool MainWindow::press_le_key(QKeyEvent* e)
{
	if (e->key() == Qt::Key_Return && !(e->modifiers() & Qt::ShiftModifier))
		return interpret_input(ui->textEdit->toPlainText()), ui->textEdit->clear(), true;
	else
		return false;
}

void MainWindow::send(QString str)
{
	if (! my_key.length())
	{
		printl("Pls set leys with /set_keys", "red");
		return;
	}

	QByteArray data;
	// Encrypt
	{
		QProcess ecel_make_msg, ecel_encrypt;

		ecel_make_msg.start("./ecel --create-msg --pos=" +QString::number(my_pos) +" --kid=" +QString::number(my_kid));
		ecel_make_msg.write(str.toUtf8());
		ecel_make_msg.closeWriteChannel();

		if (! ecel_make_msg.waitForFinished(1000))
		{
			ecel_make_msg.terminate();
			printl("Ecel returned not 0" +QString(ecel_make_msg.readAllStandardError()), "red");
			return;
		}

		ecel_encrypt.start("./ecel --encrypt=2 --key=" +my_key);
		data = ecel_make_msg.readAll();
		ecel_encrypt.write(data);
		ecel_encrypt.closeWriteChannel();

		if (! ecel_encrypt.waitForFinished(1000))
		{
			ecel_encrypt.terminate();
			printl("Ecel returned not 0" +ecel_make_msg.readAllStandardError(), "red");
			return;
		}

		data = ecel_encrypt.readAll();
	}
	qDebug() << "Pos: " << (my_pos += str.length());

	if (is_client)
	{
		client->write(data);
		client->flush();
		printl_me(str);
	}
	else if (is_server)
	{
		other_client->write(data);
		other_client->flush();
		printl_me(str);
	}
	else
		printl("Thou is not connected");
}

void MainWindow::interpret_input(QString str)
{
	if (str.startsWith("/"))
		interpret_command(str.remove(0, 1).toLower());
	else
		send(str);
}

void MainWindow::interpret_command(QString str)
{
	QStringList l = str.split(QRegExp("\\s+"), QString::SkipEmptyParts);
	QString& cmd = l[0];

	printl("EXEC '" +str +"'", "orange");
	if (cmd == "server")
	{
		if (l[1] == "start")
			start_server(l[2], l[3].toInt());
		else if (l[1] == "stop")
			stop_server();
	}
	else if (cmd == "connect")
		start_client(l[1], l[2].toInt());
	else if (cmd == "disconnect")
		stop_client();
	else if (cmd == "clear")
		ui->textBrowser->clear();
	else if (cmd == "ls")
	{
		interpret_command("server start 127.0.0.1 8080");
		interpret_command("set_keys 0 0 0000 0001");
	}
	else if (cmd == "lc")
	{
		interpret_command("connect 127.0.0.1 8080");
		interpret_command("set_keys 1 0 0001 0000");
	}
	else if (cmd == "set_keys")
	{
		my_kid = l[1].toInt();
		my_pos = l[2].toInt();
		my_key = l[3];
		he_key = l[4];
	}
	else if (cmd == "get_keys")
	{
		printl("MyKid " +QString::number(my_kid) +" MyPos " +QString::number(my_pos) +" MyKey " +my_key +" HeKey " +he_key, "orange");
	}
	else
		printl("Unknown Command: " +cmd, "red");
}

void MainWindow::client_data_ready()
{
	if (! he_key.length())
	{
		printl("Pls set leys with /set_keys", "red");
		return;
	}

	QTcpSocket* connection = qobject_cast<QTcpSocket*>(sender());

	QByteArray msg(connection->readAll());
	// Decrypt
	{
		QProcess ecel;

		ecel.start("./ecel --encrypt=2 --key=" +he_key +" --strip-msg-head");
		ecel.write(msg);

		ecel.closeWriteChannel();

		if (! ecel.waitForFinished(1000))
		{
			ecel.terminate();
			printl("Ecel returned not 0", "red");
			return;
		}
		else
			msg = ecel.readAll();
	}

	printl_he(QString::fromUtf8(msg));
}

void MainWindow::server_data_ready()
{

}

void MainWindow::print(QString msg)
{
	ui->textBrowser->insertPlainText(msg);
}

void MainWindow::printl(QString msg, QString clr)
{
	if (clr.length())
		ui->textBrowser->insertHtml("<font color=\"" +clr +"\">" +msg +"</font><br>");
	else
		ui->textBrowser->insertHtml(msg +"<br>");
}

void MainWindow::printl_me(QString str)
{
	printl(me +" " +str, me_clr);
}

void MainWindow::printl_he(QString str)
{
	printl(he +" " +str, he_clr);
}

MainWindow::~MainWindow()
{
	delete ui;
	delete client;
}
