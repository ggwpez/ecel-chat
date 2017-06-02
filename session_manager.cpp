#include "session_manager.hpp"

#include <QFile>
#include <QStringBuilder>
#include <QTextStream>
#include <QDebug>

bool SessionManager::save_to_file(const QString& cfg_path)
{
	QFile cfg(cfg_path);
	cfg.open(QIODevice::WriteOnly);

	QTextStream out(&cfg);

	for (auto const& session : sessions)
		out << *session << "\n";
	if (this->get_default_session())
		out << "/set_default_session," << this->get_default_session()->name << "\n";
	if (this->get_active_session())
		out << "/set_active_session," << this->get_default_session()->name << "\n";

	out << "/print,Sessions loaded" << "\n";
	emit print("Saved " +QString::number(sessions.size()) +" session(s) to '" +cfg_path +"'");
	return true;
}

bool SessionManager::add_session(QString const& name, std::shared_ptr<EcelKey> my_key, std::shared_ptr<EcelKey> he_key)
{
	if (exists_session(name))
		return false;

	sessions.push_back(std::make_unique<Session>(name, my_key, he_key));
	return true;
}

bool SessionManager::set_active_session(const QString& name)
{
	if (! exists_session(name))
		throw std::runtime_error("Session '" +name.toStdString() +"' not found");

	this->active_session = get_session(name);
	return true;
}

std::shared_ptr<Session> SessionManager::get_active_session() const
{
	return this->active_session;
}

void SessionManager::set_default_session(const QString& name)
{
	if (!(this->default_session = get_session(name)).get())
		throw std::runtime_error("Session '" +name.toStdString() +"' not found");
}

std::shared_ptr<Session> SessionManager::get_default_session() const
{
	return this->default_session;
}

bool SessionManager::exists_session(QString const& name)
{
	for (size_t i = 0; i < sessions.size(); ++i)
		if (sessions[i]->name == name)
			return true;

	return false;
}

std::shared_ptr<Session> SessionManager::get_session(QString const& name) const
{
	for (size_t i = 0; i < sessions.size(); ++i)
		if (sessions[i]->name == name)
			return sessions[i];

	throw std::runtime_error("Session '" +name.toStdString() +"' not found");
}

QString SessionManager::to_str() const
{
	QString str;

	for (auto const& s : sessions)
		str += "\n" +s->to_str();

	return str;
}
