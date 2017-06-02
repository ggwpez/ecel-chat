#include "session_manager.hpp"

#include <QFile>
#include <QStringBuilder>
#include <QDataStream>
#include <QDebug>

bool SessionManager::save_to_file(const QString& cfg_path)
{
	QFile cfg(cfg_path);
	cfg.open(QIODevice::WriteOnly);

	QDataStream out(&cfg);
	for (auto const& session : sessions)
		out << *session;

	return true;
}

bool SessionManager::load_sessions(const QString& cfg_path)
{
	sessions.clear();
	QFile cfg(cfg_path);
	if (! cfg.exists())
	{
		QMessageLogger().critical() << "Session file" << cfg_path << "not found";
		return false;
	}

	cfg.open(QIODevice::ReadOnly);

	QDataStream in(&cfg);
	while (! in.atEnd())
	{
		sessions.push_back(std::make_unique<Session>());
		in >> *sessions.back();
	}

	return true;
}

bool SessionManager::add_session(QString const& name, std::shared_ptr<EcelKey> my_key, std::shared_ptr<EcelKey> he_key)
{
	if (exists_session(name))
		return false;

	sessions.push_back(std::make_unique<Session>(name, my_key, he_key));
	return true;
}

bool SessionManager::exists_session(QString const& name)
{
	for (size_t i = 0; i < sessions.size(); ++i)
		if (sessions[i]->name == name)
			return true;

	return false;
}

Session const& SessionManager::get_session(QString const& name)
{
	for (size_t i = 0; i < sessions.size(); ++i)
		if (sessions[i]->name == name)
			return *sessions[i];

	throw QString("Session not found");
}

QString SessionManager::to_str()
{
	QString str;

	for (auto const& s : sessions)
		str += "\n" +s->to_str();

	return str;
}
