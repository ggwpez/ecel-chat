#include "client.hpp"
#include <QHostAddress>
#include <QMetaEnum>

Client::Client(const SessionManager& session)
	: IConnector(session),
	  socket(new QTcpSocket())
{
	connect(socket, SIGNAL(readyRead()), this, SLOT(on_data_ready()));
	connect(socket, SIGNAL(connected()), this, SLOT(on_connected()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(on_error(QAbstractSocket::SocketError)));
}

Client::~Client()
{
	if (socket)
		delete socket;
}

void Client::start(QString add, int port)
{
	if (socket->isOpen())
		throw std::runtime_error("Client is already started");

	this->address = add;
	this->port = port;

	socket->connectToHost(QHostAddress(add), port, QIODevice::ReadWrite);
}

void Client::stop()
{
	if (socket->isOpen())
	{
		this->address = ""; this->port = 0;

		socket->disconnectFromHost();
		socket->close();
	}
	else
		throw std::runtime_error("Client already stopped");
}

void Client::send(QString data)
{
	if (! data.size())
		return;

	QByteArray encoded(Encoder::encode(data.toUtf8(), *session.get_active_session()->my_key));

	if (socket->write(encoded) != encoded.size())
		throw std::runtime_error("Send to socket failed");

	socket->flush();
}

void Client::on_data_ready()
{
	QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
	QByteArray msg(Encoder::decode(client->readAll(), *session.get_active_session()->he_key));

	emit on_thee_msg(QString::fromUtf8(msg));
}

void Client::on_connected()
{
	emit on_internal_msg("Server connected");
}

void Client::on_disconnected()
{
	//socket->connectToHost(QHostAddress(this->address), port, QIODevice::ReadWrite);
	//this->address = "", this->port = 0;
	//socket->close();
	emit on_internal_msg("Server disconnected");
}

#include <QTimer>
void Client::on_error(QAbstractSocket::SocketError e)
{
	QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();

	if (e != QAbstractSocket::SocketError::RemoteHostClosedError)
	{
		QTimer::singleShot(5000, [=]()
			{ socket->connectToHost(QHostAddress(this->address), port, QIODevice::ReadWrite); });

		emit on_error_msg(QString(metaEnum.valueToKey(e)) +" Autoreconnect is enabled, trying again in 5 sec");
	}
}
