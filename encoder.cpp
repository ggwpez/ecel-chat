#include "encoder.hpp"
#include <QProcess>
#include <QDebug>

QByteArray Encoder::encode(const QByteArray& data, EcelKey& key)
{
	QProcess ecel_make_msg, ecel_encrypt;

	ecel_make_msg.start("./ecel --create-msg --pos=" +QString::number(key.pos) +" --kid=" +QString::number(key.kid));
	ecel_make_msg.write(data);
	ecel_make_msg.closeWriteChannel();

	if (! ecel_make_msg.waitForFinished(1000))
	{
		ecel_make_msg.terminate();
		throw std::runtime_error("Ecel returned not 0\n" +QString::fromUtf8(ecel_make_msg.readAllStandardError()).toStdString());
	}

	ecel_encrypt.start("./ecel --encrypt=2 --key=" +key.file.fileName());
	ecel_encrypt.write(ecel_make_msg.readAll());
	ecel_encrypt.closeWriteChannel();

	if (! ecel_encrypt.waitForFinished(1000))
	{
		ecel_encrypt.terminate();
		throw std::runtime_error("Ecel returned not 0\n" +QString::fromUtf8(ecel_make_msg.readAllStandardError()).toStdString());
	}

	key.pos += data.size();				// TODO this only works with 1:1 encryption size
	return ecel_encrypt.readAll();
}

QByteArray Encoder::decode(const QByteArray& data, EcelKey& key)
{
	QProcess ecel;

	ecel.start("./ecel --encrypt=2 --key=" +key.file.fileName() +" --strip-msg-head");
	ecel.write(data);
	ecel.closeWriteChannel();

	if (! ecel.waitForStarted(3000))
		throw std::runtime_error("Ecel could not be started\n" +ecel.errorString().toStdString());

	if (! ecel.waitForFinished(3000))
	{
		ecel.terminate();
		throw std::runtime_error("Ecel returned not 0\n" +QString::fromUtf8(ecel.readAllStandardError()).toStdString() +"\n" + ecel.errorString().toStdString());
	}

	QByteArray ret(ecel.readAll());
	return ret;
}
