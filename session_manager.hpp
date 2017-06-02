#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include "session.hpp"

class SessionManager
{
public:
	SessionManager() = default;

	void load_sessions(QString const& cfg_path);
	Session const& get_session(QString const& name);

private:
	std::vector<std::unique_ptr<Session>> sessions;
};

#endif // SESSION_MANAGER_HPP
