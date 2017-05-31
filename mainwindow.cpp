#include "mainwindow.h"
#include "ui_mainwindow.h"

QString version_str("0.01");

#include <QProcess>
#include <QObject>
#include <QMetaObject>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QShortcut>
#include <QFile>
#include <QTemporaryFile>
#include <QDateTime>
#include <QScrollBar>
#include <iostream>

MainWindow::MainWindow(QString cmd, QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow), other_client(nullptr),
	is_server(false), is_client(is_server)
{
	ui->setupUi(this);
	ui->textBrowser->setStyleSheet("QTextBrowser { color: rgb(84, 165, 196); background: rgb(24, 24, 24); selection-background-color: rgb(25, 55, 84); }");
	ui->textEdit   ->setStyleSheet("QTextEdit    { color: rgb(84, 165, 196); background: rgb(38, 38, 38); selection-background-color: rgb(25, 55, 84); }");
	ui->textBrowser->setOpenLinks(false);
	ui->textBrowser->setReadOnly(true);
	ui->textBrowser->setUndoRedoEnabled(false);

	new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));
	connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_B), this), &QShortcut::activated, [=]()
		{
			if (this->window()->windowFlags() & Qt::FramelessWindowHint)
				this->window()->setWindowFlags(this->windowFlags() & ~Qt::FramelessWindowHint);
			else
				this->window()->setWindowFlags(Qt::FramelessWindowHint);
			this->window()->show();
		});
	connect(new QShortcut(QKeySequence(Qt::Key_F11), this), &QShortcut::activated, [=]()
		{ this->window()->setWindowState(window()->windowState() ^ Qt::WindowFullScreen); });
	connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_M), this), &QShortcut::activated, [=]()
		{ this->window()->showMinimized(); });
	connect(new QShortcut(QKeySequence(Qt::Key_Escape), this), &QShortcut::activated, [=]()
		{ this->ui->textEdit->clear(); });

	qApp->installEventFilter(this);

	client = new QTcpSocket();
	server = new QTcpServer();

	client->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
	connect(client, SIGNAL(readyRead()), this, SLOT(client_data_ready()));
	connect(client, SIGNAL(disconnected()), this, SLOT(client_disconnected()));
	connect(server, SIGNAL(newConnection()), this, SLOT(connected()));

	interpret_commands(cmd);
}

QString const me("THIS"), he("THEE"),
			  me_clr(""), he_clr("LightGreen");
int my_pos, my_kid;
QFile tmp, tmp2;
QTemporaryFile my_key, he_key;

void MainWindow::start_server(QString host, int port)
{
	if (this->is_server)
	{
		QMessageBox::critical(this, tr("Network Error"),
									tr("Program is in client state, client and server state are Exclusiv"));
		return;
	}


	is_server = true;
	if (! server->listen(QHostAddress::Any, port))
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

	server_ip = server;
	server_port = port;
	is_client = true;
	client->connectToHost(QHostAddress(server), port, QIODevice::ReadWrite);

	if(client->waitForConnected(7500))
		printl("Connected");
	else
		printl("Couldnt connect");
}

void MainWindow::stop_client()
{
	if (is_client)
	{
		is_client = false;
		client->disconnectFromHost();
		client->close();
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
	printl("Connection lost");

	if (is_client)
	{
		start_client(server_ip, server_port);
	}
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
	if (e->type() == QEvent::KeyPress && obj == ui->textEdit)
		return press_le_key(static_cast<QKeyEvent*>(e));
	else if (e->type() == QEvent::FocusIn)	// TODO check what obj must equal here, i have no idea but it gets called 3-6 times
		return this->ui->textEdit->setFocus(), 1;

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
	if (! my_key.isOpen())
	{
		printl("Pls set keys with /set_keys", "red");
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
			printl("Ecel returned not 0\n" +QString::fromUtf8(ecel_make_msg.readAllStandardError()), "red");
			return;
		}

		ecel_encrypt.start("./ecel --encrypt=2 --key=" +my_key.fileName());
		data = ecel_make_msg.readAll();
		ecel_encrypt.write(data);
		ecel_encrypt.closeWriteChannel();

		if (! ecel_encrypt.waitForFinished(1000))
		{
			ecel_encrypt.terminate();
			printl("Ecel returned not 0\n" +QString::fromUtf8(ecel_make_msg.readAllStandardError()), "red");
			return;
		}

		data = ecel_encrypt.readAll();
	}
	this->setWindowTitle("Pos: "  +QString::number(my_pos += str.length()));

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
		interpret_commands(str);
	else
		send(str);
}

void MainWindow::interpret_commands(QString str)
{
	QStringList l = str.split(QRegExp("\\s*;s*"), QString::SkipEmptyParts);

	for (QString str : l)
		interpret_command(str.remove(0,1).trimmed());	// TODO ugly and unsafe
}

void MainWindow::interpret_command(QString str)
{
	QStringList l = str.split(QRegExp("\\s*,s*"), QString::SkipEmptyParts);
	for (QString& str : l)
		str = str.trimmed();

	QString cmd = l[0].toLower();
	// /set_keys, 0, 0, /home/vados/.keys/keys/final/0000.key, /home/vados/.keys/keys/final/0001.key
	printl("EXEC '" +str +"'", "orange");
	if (cmd == "version" || cmd == "ver")
	{
		printl("Version " +version_str);
	}
	else if (cmd == "server")
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
		interpret_command("server, start, 127.0.0.1, 8080");
	}
	else if (cmd == "lc")
	{
		interpret_command("connect, 127.0.0.1, 8080");
	}
	else if (cmd == "set_keys")
	{
		my_kid = l[1].toInt();
		my_pos = l[2].toInt();
		{
			QFile q1(l[3]), q2(l[4]);
			q1.open(QIODevice::ReadOnly);
			q2.open(QIODevice::ReadOnly);

			if (! q1.isOpen() || ! q2.isOpen())
				return printl("Key file not found", "red");

			my_key.open();
			my_key.write(q1.readAll());
			he_key.open();
			he_key.write(q2.readAll());
		}
	}
	else if (cmd == "get_keys")
	{
		printl("MyKid " +QString::number(my_kid) +" MyPos " +QString::number(my_pos) +" MyKey " +my_key.fileName() +" HeKey " +he_key.fileName(), "orange");
	}
	else if (cmd == "get_pos")
	{
		printl("MyPos " +QString::number(my_pos));
	}
	else
		printl("Unknown Command: " +cmd, "red");
}

void MainWindow::client_data_ready()
{
	if (! he_key.isOpen())
	{
		printl("Pls set keys with /set_keys", "red");
		return;
	}

	QTcpSocket* connection = qobject_cast<QTcpSocket*>(sender());

	QByteArray msg(connection->readAll());
	// Decrypt
	{
		QProcess ecel;

		ecel.start("./ecel --encrypt=2 --key=" +he_key.fileName() +" --strip-msg-head");
		ecel.write(msg);
		ecel.closeWriteChannel();

		if (! ecel.waitForStarted(3000))
		{
			printl("Ecel could not be started\n" +ecel.errorString(), "red");
			return;
		}

		if (! ecel.waitForFinished(3000))
		{
			ecel.terminate();
			printl("Ecel returned not 0\n" +QString::fromUtf8(ecel.readAllStandardError()) +"\n" + ecel.errorString(), "red");
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
	QString time(QDateTime::currentDateTime().toString("[hh:mm:ss]"));

	QScrollBar* bar = ui->textBrowser->verticalScrollBar();
	bool scrolled_to_end = (bar->value() == bar->maximum());

	if (clr.length())
		ui->textBrowser->insertHtml(time +" <font color=\"" +clr +"\">" +msg +"</font><br>");
	else
		ui->textBrowser->insertHtml(time + " " +msg +"<br>");

	// Scroll
	{
		if (scrolled_to_end)
			bar->setValue(bar->maximum());
		else
			;
	}
}

void MainWindow::printl_me(QString str)
{
	printl(me +": " +str, me_clr);
}

void MainWindow::printl_he(QString str)
{
	printl(he +": " +str, he_clr);
}

MainWindow::~MainWindow()
{
	delete ui;
	delete client;
}
