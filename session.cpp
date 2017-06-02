#include "session.hpp"
#include <QDataStream>

Session::Session(QObject* parent)
	: QObject(parent)
{

}

QDataStream& operator<< (QDataStream& ds, Session const& obj)
{
	ds << obj.name << *obj.my_key << *obj.he_key;
	return ds;
}

QDataStream& operator>> (QDataStream& ds, Session& obj)
{
	ds >> obj.name >> *(obj.my_key = std::make_unique<EcelKey>()) >> *(obj.he_key = std::make_unique<EcelKey>());
	return ds;
}
