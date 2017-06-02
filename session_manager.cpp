#include "session_manager.hpp"

#include <QFile>
#include <QDataStream>

void SessionManager::load_sessions(const QString& cfg_path)
{
	sessions.clear();
	QFile cfg(cfg_path);
	cfg.open(QIODevice::ReadOnly);

	QDataStream in(&cfg);
	while (! in.atEnd())
	{
		sessions.push_back(std::make_unique<Session>());
		in >> *sessions.back();
	}
}

Session const& SessionManager::get_session(QString const& name)
{
	for (size_t i = 0; i < sessions.size(); ++i)
		if (sessions[i]->name == name)
			return *sessions[i];

	throw QString("Session not found");
}
