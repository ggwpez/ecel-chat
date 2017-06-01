#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.hpp"

#include <QProcess>
#include <QObject>
#include <QMetaObject>
#include <QResizeEvent>
#include <QShortcut>
#include <QScrollBar>
#include <QKeyEvent>
#include <QFile>
#include <QTemporaryFile>
#include <QDateTime>
#include <iostream>

MainWindow::MainWindow(QString cmd, QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->textBrowser->setStyleSheet("QTextBrowser { color: rgb(84, 165, 196); background: rgb(24, 24, 24); selection-background-color: rgb(25, 55, 84); }");
	ui->textEdit   ->setStyleSheet("QTextEdit    { color: rgb(84, 165, 196); background: rgb(38, 38, 38); selection-background-color: rgb(25, 55, 84); }");

	new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));
	connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_B), this), &QShortcut::activated, [=]()
		{
			if (this->window()->windowFlags() & Qt::FramelessWindowHint)
				this->window()->setWindowFlags(this->windowFlags() & ~Qt::FramelessWindowHint);
			else
				this->window()->setWindowFlags(Qt::FramelessWindowHint);
			this->window()->show();
		});
	connect(new QShortcut(QKeySequence(Qt::Key_F11), this), &QShortcut::activated, [=]() { this->window()->setWindowState(window()->windowState() ^ Qt::WindowFullScreen); });
	connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_M), this), &QShortcut::activated, [=]() { this->window()->showMinimized(); });
	connect(new QShortcut(QKeySequence(Qt::Key_Escape), this), &QShortcut::activated, [=]() { this->ui->textEdit->clear(); });

	qApp->installEventFilter(this);

	interpret_commands(cmd);
}

QString const me("THIS"), he("THEE"),
			  me_clr(""), he_clr("LightGreen");
Encoder::Key* my_key,* he_key;

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
		return this->ui->textEdit->setFocus(), QObject::eventFilter(obj, e);

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
	if (! this->connection)
	{
		printl("Not connected", "red");
		return;
	}
	else
	{
		// TODO check success
		this->connection->send(str.toUtf8());
		printl_me(str);
	}
}

bool MainWindow::start(char which, QString add, int port)
{
	if (! connection)
	{
		connection = (which == 's') ? dynamic_cast<IConnector*>(new Server(*my_key, *he_key)) : dynamic_cast<IConnector*>(new Client(*my_key, *he_key));
		connect(connection, SIGNAL(on_data_out(QString,QString)), this, SLOT(printl(QString,QString)));
	}

	return connection && connection->start(add, port);
}

bool MainWindow::stop(char which)
{
	(void)which;

	if (! connection)
	{
		printl("Nothing to close", "red");
		return false;
	}
	else
	{
		bool ret(connection->stop());
		delete connection;
		connection = nullptr;
		return ret;
	}
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

	printl("EXEC '" +str +"'", "orange");
	if (cmd == "version" || cmd == "ver")
		printl("Build " +version);
	else if (cmd == "server")
	{
		if (l[1] == "start")
			start('s', l[2], l[3].toInt());
		else if (l[1] == "stop")
			stop('s');
	}
	else if (cmd == "connect")
		start('c', l[1], l[2].toInt());
	else if (cmd == "disconnect")
		stop('c');
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
		my_key = new Encoder::Key(l[2], l[1].toLongLong());
		he_key = new Encoder::Key(l[3], 0);
	}
	else if (cmd == "get_keys")
	{
		printl("MyKid " +QString::number(my_key->kid) +" MyPos " +QString::number(my_key->pos) +" MyKey " +my_key->file.fileName() +" HeKey " +he_key->file.fileName(), "orange");
	}
	else if (cmd == "get_pos")
	{
		printl("MyPos " +QString::number(my_key->pos));
	}
	else
		printl("Unknown Command: " +cmd, "red");
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
	if (connection)
		delete connection;
	if (my_key)
		delete my_key;
	if (he_key)
		delete he_key;
}
