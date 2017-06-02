#-------------------------------------------------
#
# Project created by QtCreator 2017-04-25T13:52:57
#
#-------------------------------------------------

QT       += core gui network widgets
CONFIG -= c++11

lessThan(QT_MAJOR_VERSION, 5): error("requires Qt 5")

TARGET = chat
TEMPLATE = app


SOURCES += main.cpp\
		mainwindow.cpp \
	server.cpp \
	client.cpp \
	encoder.cpp \
	ecel_key.cpp \
    session.cpp \
    session_manager.cpp

HEADERS  += mainwindow.h \
	server.hpp \
	client.hpp \
	iconnector.hpp \
	encoder.hpp \
	version.hpp \
	ecel_key.hpp \
    session.hpp \
    session_manager.hpp

FORMS    += mainwindow.ui
QMAKE_CXXFLAGS += -std=c++14
