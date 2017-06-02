#ifndef ICONNECTOR_HPP
#define ICONNECTOR_HPP

#include "encoder.hpp"
#include "session.hpp"
#include "session_manager.hpp"

#include <QByteArray>
#include <QString>
#include <QObject>

class IConnector : public QObject
{
	Q_OBJECT

public:
	IConnector(SessionManager const& session)
		: session(session)
	{  }

	virtual void start(QString add, int port) = 0;
	virtual void stop() = 0;
	virtual void send(QString data) = 0;
	virtual ~IConnector() = 0;

signals:
	// Messages received from THEE
	void on_thee_msg(QString str);
	// Messages, like 'server listening now'
	void on_internal_msg(QString);

protected slots:
	virtual void on_data_ready() = 0;
	virtual void on_connected() = 0;
	virtual void on_disconnected() = 0;

protected:
	SessionManager const& session;
};

inline IConnector::~IConnector() { }

#endif // ICONNECTOR_HPP
