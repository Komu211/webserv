#include "ServerConfig.hpp"

ServerConfig::ServerConfig()
{
    _host = "localhost";
    _port = 8080;
    _index = "index.html";
    _root = ".";
    _serverNames = {"localhost"};
}

// Main parameterized ctor (parses server_block_str, inherits the rest from global_config)
ServerConfig::ServerConfig(const std::string &server_block_str, const GlobalConfig &global_config)
{
    // TODO
    (void)server_block_str; // ! test
    (void)global_config; // ! test
}

// ! Need to update because server can listen to multiple host:ports combinations
void ServerConfig::setHost(const std::string &host)
{
    _host = host;
}

// ! Need to update because server can listen to multiple host:ports combinations
int ServerConfig::getPort() const
{
    return _port;
}

// ! Need to update because there can be multiple index files
void ServerConfig::setIndex(const std::string &index)
{
    _index = index;
}

void ServerConfig::setRoot(const std::string &root)
{
    _root = root;
}

// ! Need to update because server can listen to multiple host:ports combinations
void ServerConfig::setPort(int newPort)
{
    _port = newPort;
}

void ServerConfig::setServerNames(const std::vector<std::string> &serverNames)
{
    _serverNames = serverNames;
}

void ServerConfig::addServerName(const std::string &serverName)
{
    _serverNames.push_back(serverName);
}

// ! Need to update because server can listen to multiple host:ports combinations
std::string ServerConfig::getHost() const
{
    return _host;
}

// ! Need to update because there can be multiple index files
std::string ServerConfig::getIndex() const
{
    return _index;
}

std::string ServerConfig::getRoot() const
{
    return _root;
}

// ! Inefficient: making a copy of vector on every return (use const std::vector<>&)
std::vector<std::string> ServerConfig::getServerNames() const
{
    return _serverNames;
}