#include "ServerConfig.hpp"

#include <string>

ServerConfig::ServerConfig()
{
    _host = "localhost";
    _port = 8080;
    _index = "index.html";
    _root = ".";
    _serverNames = {"localhost"};
}

void ServerConfig::setHost(const std::string &host)
{
    _host = host;
}

void ServerConfig::setIndex(const std::string &index)
{
    _index = index;
}

void ServerConfig::setRoot(const std::string &root)
{
    _root = root;
}

void ServerConfig::setServerNames(const std::vector<std::string> &serverNames)
{
    _serverNames = serverNames;
}

void ServerConfig::addServerName(const std::string &serverName)
{
    _serverNames.push_back(serverName);
}

std::string ServerConfig::getHost() const
{
    return _host;
}

std::string ServerConfig::getIndex() const
{
    return _index;
}

std::string ServerConfig::getRoot() const
{
    return _root;
}

std::vector<std::string> ServerConfig::getServerNames() const
{
    return _serverNames;
}