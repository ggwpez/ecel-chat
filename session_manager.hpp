#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include "session.hpp"

class SessionManager
{
public:
	SessionManager() = default;

	bool save_to_file(QString const& cfg_path);
	bool load_sessions(QString const& cfg_path);
	bool add_session(const QString& name, std::shared_ptr<EcelKey> my_key, std::shared_ptr<EcelKey> he_key);

	bool exists_session(QString const& name);
	Session const& get_session(QString const& name);
	QString to_str();

private:
	std::vector<std::unique_ptr<Session>> sessions;
};

#endif // SESSION_MANAGER_HPP
