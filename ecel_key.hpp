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

#endif // ECEL_KEY_HPP
