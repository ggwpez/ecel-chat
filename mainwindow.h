#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>

#include "client.hpp"
#include "server.hpp"
#include "session_manager.hpp"
#include "event_manager.hpp"

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QString cmd, QWidget *parent = 0);
	~MainWindow();

private:
	Ui::MainWindow* ui;

	bool bar_enabled = true;
	IConnector* connection = nullptr;
	SessionManager sessions;
	QString active_session;
	EventManager events;

protected:
	void resizeEvent(QResizeEvent* e);
	bool press_le_key(QKeyEvent* e);
	bool eventFilter(QObject* obj, QEvent* event);
	void interpret_input(QString str);
	Q_SLOT void interpret_commands(QString str);
	void interpret_command(QString str);
	void send(QString str);
	void start(char which, QString add, int port);
	void stop(char which);

	void printl(QString msg, QString clr = "");
	void printl_me(QString str);
	void printl_he(QString str);
};

#endif // MAINWINDOW_H
