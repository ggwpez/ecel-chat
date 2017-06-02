#ifndef KEY_MANAGER_HPP
#define KEY_MANAGER_HPP

#include "ecel_key.hpp"

#include <memory>
#include <vector>
#include <QString>
#include <QTextStream>

class Session : public QObject
{
	Q_OBJECT

public:
	Session(Session const&) = delete;
	explicit Session(QObject *parent = 0);
	Session(QString const& name, std::shared_ptr<EcelKey> my_key, std::shared_ptr<EcelKey> he_key);
	QString to_str() const;

	QString name;
	std::shared_ptr<EcelKey> my_key, he_key;
};

QTextStream& operator<< (QTextStream& ds, Session const& obj);

#endif // KEY_MANAGER_HPP
