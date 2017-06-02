#ifndef ECEL_KEY_HPP
#define ECEL_KEY_HPP

#include "ecel/defines.h"
#undef fail

#include <QString>
#include <QTemporaryFile>
#include <QDataStream>

class EcelKey : public QObject
{
	Q_OBJECT

public:
	EcelKey() {}
	EcelKey(QString const& path, len_t pos);

	void load_file();
	QString to_str();

	QString path;
	QTemporaryFile file;
	kid_t kid;
	len_t pos;
};

inline QDataStream& operator<< (QDataStream& ds, EcelKey const& obj)
{
	ds << obj.path << quint64(obj.pos);

	return ds;
}

inline QDataStream& operator>> (QDataStream& ds, EcelKey& obj)
{
	quint64 pos;

	ds >> obj.path >> pos;
	obj.load_file();
	obj.pos = pos;

	return ds;
}

#endif // ECEL_KEY_HPP
