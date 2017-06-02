#include "session.hpp"
#include <QDataStream>

Session::Session(QObject* parent)
	: QObject(parent)
{

}

Session::Session(const QString& name, std::shared_ptr<EcelKey> my_key, std::shared_ptr<EcelKey> he_key)
	: name(name), my_key(my_key), he_key(he_key)
{

}

QString Session::to_str()
{
	return "Session(" +name +"," +my_key->to_str() +"," +he_key->to_str() +")";
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
