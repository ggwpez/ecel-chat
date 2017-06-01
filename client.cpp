#include "client.hpp"
#include <QHostAddress>

Client::Client(Encoder::Key const& my_key, const Encoder::Key& server_key)
	: IConnector(my_key, server_key),
	  socket(new QTcpSocket())

{

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
		emit on_data_out("Client is already started", "red");
		return false;
	}

	socket->connectToHost(QHostAddress(add), port, QIODevice::ReadWrite);

	if (socket->waitForConnected(7500))
	{
		this->address = add; this->port = port;

		emit on_data_out("Connected to server", "");
		return true;
	}
	else
	{
		emit on_data_out("Could not connect to server", "red");
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
		emit on_data_out("Client already stopped", "red");
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

	emit on_data_out("Server: " +QString::fromUtf8(msg), "");
}

void Client::on_connected()
{
	this->address = "", this->port = 0;
	emit on_data_out("Server connected", "");
}

void Client::on_disconnected()
{
	this->address = "", this->port = 0;
	emit on_data_out("Server disconnected", "");
}
