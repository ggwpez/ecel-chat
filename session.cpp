#include "session.hpp"
#include <QTextStream>

Session::Session(QObject* parent)
	: QObject(parent)
{

}

Session::Session(const QString& name, std::shared_ptr<EcelKey> my_key, std::shared_ptr<EcelKey> he_key)
	: name(name), my_key(my_key), he_key(he_key)
{

}

QString Session::to_str() const
{
	return "Session(" +name +"," +my_key->to_str() +"," +he_key->to_str() +")";
}

QTextStream& operator<< (QTextStream& ds, Session const& obj)
{
	ds << "/make_session," << obj.name << "," << obj.my_key->pos << "," << obj.he_key->pos << "," << obj.my_key->path << "," << obj.he_key->path;
	return ds;
}
