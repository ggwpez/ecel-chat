#ifndef ECEL_KEY_HPP
#define ECEL_KEY_HPP

#include "ecel/defines.h"
#undef fail

#include <QString>
#include <QTemporaryFile>

struct EcelKey
{
	EcelKey() = default;
	EcelKey(QString path, unsigned long long pos);

	QTemporaryFile file;
	kid_t kid;
	len_t pos;
};

#endif // ECEL_KEY_HPP
