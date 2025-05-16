#pragma once

#include <string>
#include <vector>
class ServerConfig
{
private:
    std::string              _host;
    int                      _port;
    std::string              _index;
    std::string              _root;
    std::vector<std::string> _serverNames = {};
public:
	ServerConfig();
	ServerConfig(const std::string &configFile);
	std::string getHost() const;
	std::string getIndex() const;
	std::string getRoot() const;
	std::vector<std::string> getServerNames() const;

	void setHost(const std::string &host);
	void setIndex(const std::string &index);
	void setRoot(const std::string &root);
	void setServerNames(const std::vector<std::string> &serverNames);
	void addServerName(const std::string &serverName);
};