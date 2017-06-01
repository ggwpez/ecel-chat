#-------------------------------------------------
#
# Project created by QtCreator 2017-04-25T13:52:57
#
#-------------------------------------------------

QT       += core gui network widgets

lessThan(QT_MAJOR_VERSION, 5): error("requires Qt 5")

TARGET = chat
TEMPLATE = app


SOURCES += main.cpp\
		mainwindow.cpp \
	server.cpp \
	client.cpp \
	encoder.cpp

HEADERS  += mainwindow.h \
	server.hpp \
	client.hpp \
	iconnector.hpp \
	encoder.hpp \
    version.hpp

FORMS    += mainwindow.ui
QMAKE_CXXFLAGS += -std=c++11
