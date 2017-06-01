#include "encoder.hpp"
#include <QProcess>

QByteArray Encoder::encode(const QByteArray& data, const Key& key)
{
	QProcess ecel_kid, ecel_make_msg, ecel_encrypt;

	ecel_kid.start("./ecel --get-kid --key=" +key.file.fileName());
	if (! ecel_kid.waitForFinished(1000))
	{
		ecel_kid.terminate();
		//printl("Ecel returned not 0\n" +QString::fromUtf8(ecel_kid.readAllStandardError()), "red");
		return QByteArray();
	}

	ecel_make_msg.start("./ecel --create-msg --pos=" +QString::number(key.pos) +" --kid=" +QString::fromUtf8(ecel_kid.readAll()));
	ecel_make_msg.write(data);
	ecel_make_msg.closeWriteChannel();

	if (! ecel_make_msg.waitForFinished(1000))
	{
		ecel_make_msg.terminate();
		//printl("Ecel returned not 0\n" +QString::fromUtf8(ecel_make_msg.readAllStandardError()), "red");
		return QByteArray();
	}

	ecel_encrypt.start("./ecel --encrypt=2 --key=" +key.file.fileName());
	ecel_encrypt.write(ecel_make_msg.readAll());
	ecel_encrypt.closeWriteChannel();

	if (! ecel_encrypt.waitForFinished(1000))
	{
		ecel_encrypt.terminate();
		//printl("Ecel returned not 0\n" +QString::fromUtf8(ecel_make_msg.readAllStandardError()), "red");
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
		//printl("Ecel could not be started\n" +ecel.errorString(), "red");
		return QByteArray();
	}

	if (! ecel.waitForFinished(3000))
	{
		ecel.terminate();
		//printl("Ecel returned not 0\n" +QString::fromUtf8(ecel.readAllStandardError()) +"\n" + ecel.errorString(), "red");
		return QByteArray();
	}

	return ecel.readAll();
}
