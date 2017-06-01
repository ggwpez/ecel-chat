#ifndef ENCODER_HPP
#define ENCODER_HPP

#include "ecel_key.hpp"

#include <QByteArray>

class Encoder
{
public:
	Encoder() = delete;

	static QByteArray encode(QByteArray const& data, const EcelKey& key);
	static QByteArray decode(QByteArray const& data, const EcelKey& key);
};

#endif // ENCODER_HPP
