#include "server.hpp"
#include "encoder.hpp"
#include "version.hpp"

#include <QScopedPointer>

Server::Server(const SessionManager& session)
	: IConnector(session),
	  socket(std::make_unique<QTcpServer>()),
	  clients()
{

}

void Server::start(QString add, int port)
{
	QString new_position("IP:" +QString::number(port));

	if (socket->isListening())
		throw std::runtime_error("Server is already started");

	connect(socket.get(), SIGNAL(newConnection()), this, SLOT(on_connected()));
	if (! socket->listen(QHostAddress::Any, port))
		throw std::runtime_error("Server cant listen on: " +new_position.toStdString());
	else
	{
		this->address = add; this->port = port;

		emit on_internal_msg("Server listening on: " +new_position);
	}
}

void Server::stop()
{
	if (! socket->isListening())
		throw std::runtime_error("Server is already not stopped");
	else
	{
		for (auto const& client : clients)
			client->close();
		clients.clear();

		this->address = ""; this->port = 0;
		emit on_internal_msg("Server stopped");
	}
}

void Server::send(QString data)
{
	QByteArray encoded(Encoder::encode(data.toUtf8(), *session.get_active_session()->my_key));

	for (auto const& client : clients)
		client->write(encoded), client->flush();
}

void Server::on_data_ready()
{
	QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
	EcelKey& key = *session.get_active_session()->he_key;
	QByteArray msg(Encoder::decode(client->readAll(), key));

	emit on_thee_msg(QString::fromUtf8(msg));
}

void Server::on_connected()
{
	QTcpSocket* client = socket->nextPendingConnection();

	if (client)
	{
		connect(client, SIGNAL(readyRead()), this, SLOT(on_data_ready()));
		connect(client, SIGNAL(disconnected()), this, SLOT(on_disconnected()));

		this->clients.push_back(client);
		emit on_internal_msg("Client connected");
	}
	else
		throw std::runtime_error("Weird error");
}

void Server::on_disconnected()
{
	QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());

	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (client == clients[i])
			clients.erase(clients.begin() +i);
	}

	emit on_internal_msg("Client disconnected");
}

void Server::on_error(QAbstractSocket::SocketError e)
{
	emit on_error_msg("Unhandled Exception in server");
}
