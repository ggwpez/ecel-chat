#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#include <map>
#include <vector>

#include <QObject>
#include <QString>

class EventManager : public QObject
{
	Q_OBJECT

public:
	EventManager(QString const& script_dir);

signals:
	void interpret_commands(QString const& cmds);

public slots:
	/*void on_connected(QString const& client);
	void on_disconnected(QString const& client);
	void on_msg(QString const& sender, QString const& msg);*/
	void fire_event(QString const& name, std::vector<QString> const& args);

private:
		  // name			 // arg names          // script
	std::map<QString, std::pair<std::vector<QString>, QString>> events;
};

#endif // EVENT_MANAGER_HPP
