#include "event_manager.hpp"
#include <QDebug>
#include <QDirIterator>

std::map<QString, std::vector<QString>> event_args =
 {
	{ "on_connected",    { "client"		   } },
	{ "on_disconnected", { "client"        } },
	{ "on_msg",          { "sender", "msg" } }
 };

// std::map<QString, std::pair<std::vector<QString>, QString>> events;
EventManager::EventManager(QString const& script_dir)
{
	QDirIterator it(script_dir);

	while (it.hasNext())
	{
		QString file(it.next()),
				event_name(file.split(QRegExp("/")).last());

		if (event_args.find(event_name) != event_args.end())
		{
			QFile script(file);
			script.open(QIODevice::ReadOnly);

			events[event_name] = std::make_pair(event_args[event_name], QString::fromUtf8(script.readAll()));
		}
	}
}

void EventManager::fire_event(const QString& name, const std::vector<QString>& args)
{
	auto const& it(events.find(name));

	if (it == events.end())
		emit interpret_commands("/print,Internal Error: Event '" +name +"' not found");
	else if (it->second.first.size() != args.size())
		emit interpret_commands("/print,Internal Error: Event '" +name +"' awaits " +QString::number(it->first.size()) +" args, bug only got " +QString::number(args.size()));
	else if (! it->second.second.size())
		qDebug() << "No script registered for event" << name;
	else
	{
		QString script(it->second.second);

		for (size_t i = 0; i < args.size(); ++i)
			script.replace("$" +it->second.first[i], args[i]);		// substitute the $args

		qDebug() << "Event" << name << "executing script:" << script;
		emit interpret_commands(script);
	}
}
