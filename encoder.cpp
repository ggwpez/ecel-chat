#include "encoder.hpp"
#include <QProcess>
#include <QDebug>

QByteArray Encoder::encode(const QByteArray& data, const Key& key)
{
	QProcess ecel_make_msg, ecel_encrypt;

	ecel_make_msg.start("./ecel --create-msg --pos=" +QString::number(key.pos) +" --kid=" +QString::number(key.kid));
	ecel_make_msg.write(data);
	ecel_make_msg.closeWriteChannel();

	if (! ecel_make_msg.waitForFinished(1000))
	{
		ecel_make_msg.terminate();
		qDebug () << ("Ecel returned not 0\n" +QString::fromUtf8(ecel_make_msg.readAllStandardError()), "red");
		return QByteArray();
	}

	ecel_encrypt.start("./ecel --encrypt=2 --key=" +key.file.fileName());
	ecel_encrypt.write(ecel_make_msg.readAll());
	ecel_encrypt.closeWriteChannel();

	if (! ecel_encrypt.waitForFinished(1000))
	{
		ecel_encrypt.terminate();
		qDebug () << ("Ecel returned not 0\n" +QString::fromUtf8(ecel_make_msg.readAllStandardError()), "red");
		return QByteArray();
	}

	return ecel_encrypt.readAll();
}

QByteArray Encoder::decode(const QByteArray& data, const Key& key)
{
	QProcess ecel;

	ecel.start("./ecel --encrypt=2 --key=" +key.file.fileName() +" --strip-msg-head");
	ecel.write(data);
	ecel.closeWriteChannel();

	if (! ecel.waitForStarted(3000))
	{
		qDebug () << ("Ecel could not be started\n" +ecel.errorString(), "red");
		return QByteArray();
	}

	if (! ecel.waitForFinished(3000))
	{
		ecel.terminate();
		qDebug() << ("Ecel returned not 0\n" +QString::fromUtf8(ecel.readAllStandardError()) +"\n" + ecel.errorString(), "red");
		return QByteArray();
	}

	return ecel.readAll();
}

Encoder::Key::Key(QString path, unsigned long long pos)
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
