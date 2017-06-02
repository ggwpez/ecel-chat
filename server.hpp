#ifndef SERVER_HPP
#define SERVER_HPP

#include "iconnector.hpp"
#include "encoder.hpp"

#include <vector>
#include <memory>
#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QScopedPointer>

class Server : public IConnector
{
public:
	Server(SessionManager const& session);

	bool start(QString add, int port) override;
	bool stop() override;
	bool send(QString data) override;

public slots:
	void on_data_ready() override;
	void on_connected() override;
	void on_disconnected() override;

protected:
	std::unique_ptr<QTcpServer> socket;
	std::vector<QTcpSocket*> clients;

	QString address;
	int port;
};

#endif // SERVER_HPP
