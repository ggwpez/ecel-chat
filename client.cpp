#include "client.hpp"
#include <QHostAddress>

Client::Client(const EcelKey& my_key, const EcelKey& server_key)
	: IConnector(my_key, server_key),
	  socket(new QTcpSocket())

{
	connect(socket, SIGNAL(readyRead()), this, SLOT(on_data_ready()));
	connect(socket, SIGNAL(connected()), this, SLOT(on_connected()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
}

Client::~Client()
{
	if (socket)
		delete socket;
}

bool Client::start(QString add, int port)
{
	if (socket->isOpen())
	{
		emit on_error("Client is already started");
		return false;
	}

	socket->connectToHost(QHostAddress(add), port, QIODevice::ReadWrite);

	if (socket->waitForConnected(7500))
	{
		this->address = add; this->port = port;

		emit on_internal_msg("Connected to server");
		return true;
	}
	else
	{
		emit on_error("Could not connect to server");
		return false;
	}
}

bool Client::stop()
{
	if (socket->isOpen())
	{
		socket->disconnectFromHost();
		socket->close();

		this->address = ""; this->port = 0;
		return true;
	}
	else
	{
		emit on_error("Client already stopped");
		return false;
	}
}

bool Client::send(QByteArray data)
{
	QByteArray encoded(Encoder::encode(data, this->my_key));

	socket->write(encoded), socket->flush();

	return true;
}

void Client::on_data_ready()
{
	QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
	QByteArray msg(Encoder::decode(client->readAll(), this->he_key));

	emit on_thee_msg(QString::fromUtf8(msg));
}

void Client::on_connected()
{
	this->address = "", this->port = 0;
	emit on_internal_msg("Server connected");
}

void Client::on_disconnected()
{
	this->address = "", this->port = 0;
	socket->close();
	emit on_internal_msg("Server disconnected");
}
