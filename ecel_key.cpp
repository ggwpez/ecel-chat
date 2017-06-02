#include "ecel_key.hpp"

#include <QFile>
#include <QDebug>
#include <QProcess>
#include <exception>
#include <qexception.h>

EcelKey::EcelKey(const QString& path, len_t pos)
	: path(path), pos(pos)
{
	load_file();
}

void EcelKey::load_file()
{
	QFile tmp(path);
	tmp.open(QIODevice::ReadOnly);

	if (! tmp.isOpen())
		throw std::runtime_error("File '" +path.toStdString() +"' not found");

	// /set_keys,137000,/media/vados/KEY-STICK-000/0000.key,/media/vados/KEY-STICK-000/0000.key
	this->file.open();
	this->file.write(tmp.readAll());

	QProcess ecel_kid;
	ecel_kid.start("./ecel --get=key_kid --key=" +file.fileName());
	if (! ecel_kid.waitForFinished(1000))
	{
		ecel_kid.terminate();
		throw std::runtime_error("Ecel returned not 0\n" +QString::fromUtf8(ecel_kid.readAllStandardError()).toStdString());
	}

	this->kid = QString::fromUtf8(ecel_kid.readAll()).toLongLong();
}

QString EcelKey::to_str()
{
	return "EcelKey(" +QString::number(kid) +"," +path +"," +QString::number(pos) +"," +file.fileName() +")";
}
