#include "mainwindow.h"
#include <QtGui>
#include <QApplication>
#include <QProcess>

int main(int argc, char** argv)
{
	QString cmd = "";
	for (int i = 1; i < argc; ++i)
		cmd += argv[i];

	QApplication a(argc, argv);
	MainWindow w(cmd);

	w.show();

	return a.exec();
}
