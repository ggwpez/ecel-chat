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
#include <QtConcurrent/QtConcurrent>
#include <QDateTime>
#include <QFuture>
#include <QThread>
#include <QTimer>
#include <iostream>
#include <functional>
#include <map>

MainWindow::MainWindow(QString cmd, QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow), sessions(), events("./events/")
{
	ui->setupUi(this);
	qApp->installEventFilter(this);

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

	connect(&sessions, &SessionManager::print, [=](QString str) { this->printl("SessionMng: " +str, "orange"); });
	connect(&events, SIGNAL(interpret_commands(QString)), this, SLOT(interpret_commands(QString)));

	QFile tmp("ecel");
	if (! tmp.exists())
	{
		printl("Could not find ecel. Pls copy it to:", "red");
		interpret_commands("/system,pwd");
	}

	interpret_commands("/load,config.txt");
	interpret_commands(cmd);
}

QString const me("THIS"), he("THEE"),
			  me_clr(""), he_clr("LightGreen");

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
		this->connection->send(str);
		printl_me(str);
	}
}

void MainWindow::start(char which, QString add, int port)
{
	if (! connection)
	{
		connection = (which == 's') ? dynamic_cast<IConnector*>(new Server(sessions))
									: dynamic_cast<IConnector*>(new Client(sessions));

		connect(connection, &IConnector::on_thee_msg,	  [=](QString str) { this->printl_he(str); });
		connect(connection, &IConnector::on_thee_msg,	  [=](QString str) { this->events.fire_event("on_msg", {"thee", str}); });
		connect(connection, &IConnector::on_error_msg,	  [=](QString str) { this->printl(str, "red"); });
		connect(connection, &IConnector::on_internal_msg, [=](QString str) { this->printl(str, "orange"); });

		try
		{ connection->start(add, port); }
		catch (std::exception const& e)
		{
			printl("Exception: " +QString(e.what()), "red");
			delete connection;
			connection = nullptr;
		}
	}
	else
		printl("Server or Client are already running", "red");
}

void MainWindow::stop(char which)
{
	(void)which;

	if (! connection)
		throw std::runtime_error("Nothing to close");
	else
	{
		connection->deleteLater();
		connection->stop();
		connection = nullptr;
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
	QStringList l = str.split(QRegExp("(\\;|\\n)"), QString::SkipEmptyParts);

	for (QString str : l)
	{
		try
		{ interpret_command(str.remove(0,1).trimmed()); }
		catch (std::exception const& e)
		{ printl("Exception: " +QString(e.what()), "red"); }
	}
}

void MainWindow::interpret_command(QString str)
{
	std::map<QString, std::function<void(QStringList const& v)>> fptrs;
	fptrs["set_ui_browser_font"] = [=](QStringList const& v)  { ui->textBrowser->setFont(QFont(v[0])); };
	fptrs["set_ui_edit_font"   ] = [=](QStringList const& v)  { ui->textEdit->setFont(QFont(v[0])); };

/*	QPalette p;
	p.setColor(QPalette::Base, Qt::black);
	p.setColor(QPalette::Text, Qt::white);
	ui->textBrowser->setPalette(p);*/

	fptrs["set_ui_window_title"	] = [=](QStringList const& v) { window()->setWindowTitle(v[0]); };
	fptrs["set_default_session"	] = [=](QStringList const& v) { sessions.set_default_session(v[0]); };
	fptrs["set_active_session"	] = [=](QStringList const& v) { sessions.set_active_session(v[0]); };
	fptrs["save_to_file"		] = [=](QStringList const& v) { sessions.save_to_file(v[0]); };
	fptrs["get_sessions"		] = [=](QStringList const&  ) { printl(sessions.to_str(), "orange"); };
	fptrs["disconnect"			] = [=](QStringList const&  ) { stop ('c'); };
	fptrs["connect"				] = [=](QStringList const& v) { start('c', v[0], v[1].toInt()); };
	fptrs["version"				] = [=](QStringList const&  ) { printl("Build: " +version_str, "orange"); };
	fptrs["print"				] = [=](QStringList const& v) { printl(v[0]); };
	fptrs["clear"				] = [=](QStringList const&  ) { ui->textBrowser->clear(); };
	fptrs["ls"					] = [=](QStringList const&  ) { interpret_command("server, start, 127.0.0.1, 8090"); };
	fptrs["lc"					] = [=](QStringList const&  ) { interpret_command("connect, 127.0.0.1, 8090"); };

	QStringList l = str.split(QRegExp("\\,"));
	for (QString& str : l)
		str = str.trimmed();

	QString cmd = l[0].toLower();

	printl("EXEC '" +str +"'", "orange");
	if (cmd == "load")
	{
		QFile f(l[1]);
		if (f.exists())
		{
			f.open(QIODevice::ReadOnly);
			interpret_commands(QString::fromUtf8(f.readAll()));
		}
		else
			printl("File '" +l[1] +"' not found", "red");
	}
	else if (cmd == "server")
	{
		if (l[1] == "start")
			start('s', l[2], l[3].toInt());
		else if (l[1] == "stop")
			stop('s');
	}
	else if (cmd == "system")
	{
		QProcess* p(new QProcess());
		QThread* d(new QThread());
		l.removeFirst();
		cmd = l[0];
		l.removeFirst();
		p->moveToThread(d);

		connect(p, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
					[=, p, d](int code, QProcess::ExitStatus status)
					{
						QTimer::singleShot(0, qApp, [=,p,d](){
						printl("###TEST");
						if (status != QProcess::ExitStatus::NormalExit)
							throw std::runtime_error("Command crashed");
						else if (code)
							throw std::runtime_error("Command exited with " +std::to_string(code));
						else
							printl(QString::fromUtf8(p->readAll()), "orange");

						delete p;
						delete d;
					});}
				);
		connect(p, &QProcess::errorOccurred,
					[=, p, d](QProcess::ProcessError code)
					{
						QTimer::singleShot(0, qApp, [=,p,d](){
						printl("###TEST");
						throw std::runtime_error("Command crashed with ProcessError code: " +std::to_string(int(code))
												 +"Please have a look at: http://doc.qt.io/qt-5/qprocess.html#ProcessError-enum");

						delete p;
						delete d;
					});}
				);

		p->start(cmd, l);
	}
	else if (cmd == "make_session")
	{
		auto key1(std::make_shared<EcelKey>(l[4], l[2].toLongLong())),	// TODO it are len_t not long long's
			 key2(std::make_shared<EcelKey>(l[5], l[3].toLongLong())); 	// temps for strict evaluation order

		sessions.add_session(l[1], key1, key2);
	}
	else
	{
		auto const& f(fptrs.find(cmd));

		if (f != fptrs.end())
		{
			l.removeFirst();
			f->second(l);
		}
		else
			printl("Unknown Command: " +cmd, "red");
	}
}

void MainWindow::printl(QString msg, QString clr)
{
	QString time(QDateTime::currentDateTime().toString("[hh:mm:ss]"));

	QScrollBar* bar = ui->textBrowser->verticalScrollBar();
	bool scrolled_to_end = (bar->value() == bar->maximum());

	if (clr.length())
		ui->textBrowser->insertHtml("<font color=white>" +time +"</font> <font color=\"" +clr +"\">" +msg +"</font><br>");
	else
		ui->textBrowser->insertHtml("<font color=white>" +time + "</font> " +msg +"<br>");

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
	sessions.save_to_file("sessions.txt");
	delete ui;

	if (connection)
		delete connection;
}
