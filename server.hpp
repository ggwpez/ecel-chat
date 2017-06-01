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
	Server(Encoder::Key const& my_key, const Encoder::Key& client_key);
	~Server();

	bool start(QString add, int port) override;
	bool stop() override;
	bool send(QByteArray data) override;

public slots:
	void on_data_ready() override;
	void on_connected() override;
	void on_disconnected() override;

protected:
	QTcpServer* socket;
	std::vector<QTcpSocket*> clients;

	QString address;
	int port;
};

#endif // SERVER_HPP
