#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTcpServer>

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
	Ui::MainWindow *ui;
	QTcpSocket* client,* other_client;
	QTcpServer* server;
	bool bar_enabled = true;
	bool is_server = false, is_client = false;
	QString server_ip;
	int server_port;

protected:
	void resizeEvent(QResizeEvent* e);
	bool press_le_key(QKeyEvent* e);
	bool eventFilter(QObject* obj, QEvent* event);
	void interpret_input(QString str);
	void interpret_commands(QString str);
	void interpret_command(QString str);
	void send(QString str);
	void start_server(QString host, int port);
	void stop_server();
	void start_client(QString server, int port);
	void stop_client();

	void print(QString msg);
	void printl(QString msg, QString clr = "");
	void printl_me(QString str);
	void printl_he(QString str);

private slots:
	void server_data_ready();
	void client_data_ready();
	void connected();
	void server_disconnected();
	void client_disconnected();
};

#endif // MAINWINDOW_H
