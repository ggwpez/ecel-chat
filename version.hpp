#ifndef VERSION_HPP
#define VERSION_HPP

#include <QString>

#define version_str (QString("'%1, %2'").arg(__DATE__).arg(__TIME__))

#endif // VERSION_HPP
