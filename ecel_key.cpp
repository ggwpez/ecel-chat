#include "ecel_key.hpp"

#include <QFile>
#include <QDebug>
#include <QProcess>

EcelKey::EcelKey(QString path, unsigned long long pos)
{
	QFile tmp(path);
	tmp.open(QIODevice::ReadOnly);

	if (! tmp.isOpen())
	{
		qDebug() << "Key file not found";
		return;
	}

	// /set_keys,137000,/media/vados/KEY-STICK-000/0000.key,/media/vados/KEY-STICK-000/0000.key
	this->file.open();
	this->file.write(tmp.readAll());

	QProcess ecel_kid;
	ecel_kid.start("./ecel --get=key_kid --key=" +file.fileName());
	if (! ecel_kid.waitForFinished(1000))
	{
		ecel_kid.terminate();
		qDebug () << ("Ecel returned not 0\n" +QString::fromUtf8(ecel_kid.readAllStandardError()), "red");
	}

	this->kid = QString::fromUtf8(ecel_kid.readAll()).toLongLong();
	this->pos = pos;
}
