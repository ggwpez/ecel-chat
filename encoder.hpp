#ifndef ENCODER_HPP
#define ENCODER_HPP

#include <QTemporaryFile>
#include <QByteArray>

class Encoder
{
public:
	struct Key
	{
		Key() = default;

		QTemporaryFile file;
		unsigned long long pos;
	};

	Encoder() = delete;

	static QByteArray encode(QByteArray const& data, Key const& key);
	static QByteArray decode(QByteArray const& data, Key const& key);
};

#endif // ENCODER_HPP
