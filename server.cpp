#include "server.hpp"
#include "encoder.hpp"

#include <QScopedPointer>

Server::Server(const Encoder::Key& my_key, Encoder::Key const& client_key)
	: IConnector(my_key, client_key),
	  socket(new QTcpServer()),
	  clients()
{

}

Server::~Server()
{
	if (socket)
		delete socket;
}

bool Server::start(QString add, int port)
{
	QString new_position("IP:" +QString::number(port));

	if (socket->isListening())
	{
		emit on_data_out("Server is already started", "red");
		return false;
	}

	connect(socket, SIGNAL(newConnection()), this, SLOT(on_connected()));
	if (! socket->listen(QHostAddress::Any, port))
	{
		emit on_data_out("Server cant listen on: " +new_position, "red");
		return false;
	}
	else
	{
		this->address = add; this->port = port;

		emit on_data_out("Server listening on: " +new_position, "");
		return true;
	}
}

bool Server::stop()
{
	if (! socket->isListening())
	{
		emit on_data_out("Server is already not stopped", "red");
		return false;
	}
	else
	{
		for (auto const& client : clients)
			client->close();
		clients.clear();

		this->address = ""; this->port = 0;
		emit on_data_out("Server stopped", "");
		return true;
	}
}

bool Server::send(QByteArray data)
{
	QByteArray encoded(Encoder::encode(data, this->my_key));

	for (auto const& client : clients)
		client->write(encoded), client->flush();

	return true;
}

void Server::on_data_ready()
{
	QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
	QByteArray msg(Encoder::decode(client->readAll(), this->he_key));

	emit on_data_out("Client: " +QString::fromUtf8(msg), "");
}

void Server::on_connected()
{
	QTcpSocket* client = socket->nextPendingConnection();

	if (client)
	{
		connect(client, SIGNAL(readyRead()), this, SLOT(on_client_data_ready()));
		connect(client, SIGNAL(disconnected()), this, SLOT(on_client_disconnected()));

		// TODO emplace back
		this->clients.push_back(std::unique_ptr<QTcpSocket>(client));
		emit on_data_out("Client connected", "");
		this->send(QString("Hi from Server").toUtf8());
	}
	else
		emit on_data_out("Weird error", "red");
}

void Server::on_disconnected()
{
	QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());

	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (client == clients[i].get())
			clients.erase(clients.begin() +i);
	}

	emit on_data_out("Client disconnected", "");
}
