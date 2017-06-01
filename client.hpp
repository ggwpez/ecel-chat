#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "iconnector.hpp"
#include "encoder.hpp"

#include <QObject>
#include <QScopedPointer>
#include <QTcpSocket>

class Client : public IConnector
{
public:
	Client(EcelKey const& my_key, EcelKey const& server_key);
	~Client();

	virtual bool start(QString add, int port) override;
	virtual bool stop() override;
	virtual bool send(QByteArray data) override;

protected slots:
	void on_data_ready() override;
	void on_connected() override;
	void on_disconnected() override;

protected:
	QTcpSocket* socket;

	QString address;
	int port;
};

#endif // CLIENT_HPP
