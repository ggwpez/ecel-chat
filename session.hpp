#ifndef KEY_MANAGER_HPP
#define KEY_MANAGER_HPP

#include "ecel_key.hpp"

#include <memory>
#include <vector>
#include <QString>

class Session : public QObject
{
	Q_OBJECT

public:
	Session(Session const&) = delete;
	explicit Session(QObject *parent = 0);

	QString name;
	std::unique_ptr<EcelKey> my_key, he_key;
};

QDataStream& operator<< (QDataStream& ds, Session const& obj);
QDataStream& operator>> (QDataStream& ds, Session& obj);


#endif // KEY_MANAGER_HPP
