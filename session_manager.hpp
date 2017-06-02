#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include "session.hpp"

#include <QObject>

class SessionManager : public QObject
{
	Q_OBJECT

public:
	SessionManager() = default;

	bool save_to_file(QString const& cfg_path);
	bool load_sessions(QString const& cfg_path);
	bool add_session(const QString& name, std::shared_ptr<EcelKey> my_key, std::shared_ptr<EcelKey> he_key);

	bool set_active_session(QString const& name);
	std::shared_ptr<Session> get_active_session() const;

	bool set_default_session(QString const& name);
	std::shared_ptr<Session> get_default_session() const;

	bool exists_session(QString const& name);
	std::shared_ptr<Session> get_session(QString const& name) const;
	QString to_str() const;

signals:
	void print(QString);

private:
	std::shared_ptr<Session> active_session, default_session;
	std::vector<std::shared_ptr<Session>> sessions;
};

#endif // SESSION_MANAGER_HPP
