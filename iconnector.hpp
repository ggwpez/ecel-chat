#ifndef ICONNECTOR_HPP
#define ICONNECTOR_HPP

#include "encoder.hpp"

#include <QByteArray>
#include <QString>
#include <QObject>

class IConnector : public QObject
{
	Q_OBJECT

public:
	IConnector(EcelKey const& my_key, EcelKey const& he_key)
		: my_key(my_key), he_key(he_key)
	{  }

	virtual bool start(QString add, int port) = 0;
	virtual bool stop() = 0;
	virtual bool send(QByteArray data) = 0;
	virtual ~IConnector() = 0;

signals:
	// Messages received from THEE
	void on_thee_msg(QString str);
	// Messages, like 'server listening now'
	void on_internal_msg(QString);
	void on_error(QString str);

protected slots:
	virtual void on_data_ready() = 0;
	virtual void on_connected() = 0;
	virtual void on_disconnected() = 0;

protected:
	EcelKey const& my_key;
	EcelKey const& he_key;
};

inline IConnector::~IConnector() { }

#endif // ICONNECTOR_HPP
