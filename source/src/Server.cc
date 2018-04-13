/// \file Server.cc
/*
 *
 * Server.cc source template automatically generated by a class generator
 * Creation date : sam. d�c. 3 2016
 *
 * This file is part of DQM4HEP libraries.
 *
 * DQM4HEP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 *
 * DQM4HEP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DQM4HEP.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */

// -- dqm4hep headers
#include <dqm4hep/Internal.h>
#include <dqm4hep/Server.h>
#include <dqm4hep/Logging.h>

// -- std headers
#include <sys/utsname.h>
#include <unistd.h>

// -- dim headers
#include <dic.hxx>

namespace dqm4hep {

  namespace net {

    Server::Server(const std::string &name)
        : m_name(name),
          m_started(false),
          m_serverInfoHandler(this, "/" + m_name + "/info", this, &Server::handleServerInfoRequest) {
      DimServer::addClientExitHandler(this);
    }

    //-------------------------------------------------------------------------------------------------

    Server::~Server() {
      this->stop();
      this->clear();
    }

    //-------------------------------------------------------------------------------------------------

    const std::string &Server::name() const {
      return m_name;
    }

    //-------------------------------------------------------------------------------------------------

    void Server::start() {
      if (m_started)
        return;

      for (auto iter = m_serviceMap.begin(), endIter = m_serviceMap.end(); endIter != iter; ++iter) {
        if (!iter->second->isServiceConnected())
          iter->second->connectService();
      }

      for (auto iter = m_requestHandlerMap.begin(), endIter = m_requestHandlerMap.end(); endIter != iter; ++iter) {
        if (!iter->second->isHandlingRequest())
          iter->second->startHandlingRequest();
      }

      for (auto iter = m_commandHandlerMap.begin(), endIter = m_commandHandlerMap.end(); endIter != iter; ++iter) {
        if (!iter->second->isHandlingCommands())
          iter->second->startHandlingCommands();
      }

      if (!m_serverInfoHandler.isHandlingRequest())
        m_serverInfoHandler.startHandlingRequest();

      DimServer::start(const_cast<char *>(m_name.c_str()));

      for (auto timer : m_timers)
        timer->startTimer();

      m_started = true;
    }

    //-------------------------------------------------------------------------------------------------

    void Server::stop() {
      if (!m_started)
        return;

      for (auto iter = m_serviceMap.begin(), endIter = m_serviceMap.end(); endIter != iter; ++iter) {
        if (iter->second->isServiceConnected())
          iter->second->disconnectService();
      }

      for (auto iter = m_requestHandlerMap.begin(), endIter = m_requestHandlerMap.end(); endIter != iter; ++iter) {
        if (iter->second->isHandlingRequest())
          iter->second->stopHandlingRequest();
      }

      for (auto iter = m_commandHandlerMap.begin(), endIter = m_commandHandlerMap.end(); endIter != iter; ++iter) {
        if (iter->second->isHandlingCommands())
          iter->second->stopHandlingCommands();
      }

      if (m_serverInfoHandler.isHandlingRequest())
        m_serverInfoHandler.stopHandlingRequest();

      for (auto timer : m_timers)
        timer->stopTimer();

      m_timers.clear();

      DimServer::stop();
      m_started = false;
    }

    //-------------------------------------------------------------------------------------------------

    bool Server::isRunning() const {
      return m_started;
    }

    //-------------------------------------------------------------------------------------------------

    void Server::clear() {
      for (auto iter = m_serviceMap.begin(), endIter = m_serviceMap.end(); endIter != iter; ++iter)
        delete iter->second;

      for (auto iter = m_requestHandlerMap.begin(), endIter = m_requestHandlerMap.end(); endIter != iter; ++iter)
        delete iter->second;

      for (auto iter = m_commandHandlerMap.begin(), endIter = m_commandHandlerMap.end(); endIter != iter; ++iter)
        delete iter->second;

      m_serviceMap.clear();
      m_requestHandlerMap.clear();
      m_commandHandlerMap.clear();
      DimServer::stop();
      m_started = false;
    }

    //-------------------------------------------------------------------------------------------------

    Service *Server::createService(const std::string &name) {
      if (name.empty())
        throw std::runtime_error("Server::createService(): service name is invalid");

      auto findIter = m_serviceMap.find(name);

      if (findIter != m_serviceMap.end())
        return findIter->second;

      if (Server::serviceAlreadyRunning(name))
        throw std::runtime_error("Server::createService(): service '" + name + "' already running on network");

      // first insert nullptr, then create the service
      std::pair<ServiceMap::iterator, bool> inserted = m_serviceMap.insert(ServiceMap::value_type(name, nullptr));

      if (inserted.second) {
        Service *pService = new Service(this, name);
        inserted.first->second = pService;

        if (this->isRunning())
          pService->connectService();

        return pService;
      } else
        throw;
    }

    //-------------------------------------------------------------------------------------------------

    bool Server::isServiceRegistered(const std::string &name) const {
      return (m_serviceMap.find(name) != m_serviceMap.end());
    }

    //-------------------------------------------------------------------------------------------------

    bool Server::isRequestHandlerRegistered(const std::string &name) const {
      return (m_requestHandlerMap.find(name) != m_requestHandlerMap.end());
    }

    //-------------------------------------------------------------------------------------------------

    bool Server::isCommandHandlerRegistered(const std::string &name) const {
      return (m_commandHandlerMap.find(name) != m_commandHandlerMap.end());
    }

    //-------------------------------------------------------------------------------------------------

    void Server::startService(const std::string &name) {
      Service *pService = this->service(name);

      if (nullptr != pService && !pService->isServiceConnected())
        pService->connectService();
    }

    //-------------------------------------------------------------------------------------------------

    void Server::stopService(const std::string &name) {
      Service *pService = this->service(name);

      if (nullptr != pService && pService->isServiceConnected())
        pService->disconnectService();
    }

    //-------------------------------------------------------------------------------------------------

    void Server::startRequestHandler(const std::string &name) {
      RequestHandler *pRequestHandler = this->requestHandler(name);

      if (nullptr != pRequestHandler && !pRequestHandler->isHandlingRequest())
        pRequestHandler->startHandlingRequest();
    }

    //-------------------------------------------------------------------------------------------------

    void Server::stopRequestHandler(const std::string &name) {
      RequestHandler *pRequestHandler = this->requestHandler(name);

      if (nullptr != pRequestHandler && pRequestHandler->isHandlingRequest())
        pRequestHandler->stopHandlingRequest();
    }

    //-------------------------------------------------------------------------------------------------

    void Server::startCommandHandler(const std::string &name) {
      CommandHandler *pCommandHandler = this->commandHandler(name);

      if (nullptr != pCommandHandler && !pCommandHandler->isHandlingCommands())
        pCommandHandler->startHandlingCommands();
    }

    //-------------------------------------------------------------------------------------------------

    void Server::stopCommandHandler(const std::string &name) {
      CommandHandler *pCommandHandler = this->commandHandler(name);

      if (nullptr != pCommandHandler && pCommandHandler->isHandlingCommands())
        pCommandHandler->stopHandlingCommands();
    }

    //-------------------------------------------------------------------------------------------------

    Service *Server::service(const std::string &name) const {
      auto findIter = m_serviceMap.find(name);
      return (findIter == m_serviceMap.end() ? nullptr : findIter->second);
    }

    //-------------------------------------------------------------------------------------------------

    RequestHandler *Server::requestHandler(const std::string &name) const {
      auto findIter = m_requestHandlerMap.find(name);
      return (findIter == m_requestHandlerMap.end() ? nullptr : findIter->second);
    }

    //-------------------------------------------------------------------------------------------------

    CommandHandler *Server::commandHandler(const std::string &name) const {
      auto findIter = m_commandHandlerMap.find(name);
      return (findIter == m_commandHandlerMap.end() ? nullptr : findIter->second);
    }

    //-------------------------------------------------------------------------------------------------

    core::Signal<int> &Server::onClientExit() {
      return m_clientExitSignal;
    }

    //-------------------------------------------------------------------------------------------------

    int Server::clientId() const {
      return DimServer::getClientId();
    }

    //-------------------------------------------------------------------------------------------------

    std::string Server::dnsNode() {
      char *pDnsNode = DimServer::getDnsNode();

      if (pDnsNode)
        return pDnsNode;

      pDnsNode = getenv("DIM_DNS_NODE");
      return (pDnsNode ? pDnsNode : "");
    }

    //-------------------------------------------------------------------------------------------------

    int Server::dnsPort() {
      return DimServer::getDnsPort();
    }

    //-------------------------------------------------------------------------------------------------

    std::vector<std::string> Server::runningServers() {
      std::vector<std::string> runningServers;

      DimBrowser browser;
      browser.getServers();
      char *pServer, *pNode;

      while (browser.getNextServer(pServer, pNode)) {
        std::string server(pServer);
        runningServers.push_back(server);
      }

      return runningServers;
    }

    //-------------------------------------------------------------------------------------------------

    bool Server::isServerRunning(const std::string &serverName) {
      DimBrowser browser;
      browser.getServers();
      char *pServer, *pNode;

      while (browser.getNextServer(pServer, pNode)) {
        std::string server(pServer);

        if (server == serverName)
          return true;
      }

      return false;
    }

    //-------------------------------------------------------------------------------------------------

    void Server::handleServerInfoRequest(const Buffer & /*request*/, Buffer &response) {
      // get the list of services, request handlers and command handlers
      core::StringVector serviceList, requestHandlerList, commandHandlerList;

      for (auto service : m_serviceMap) {
        serviceList.push_back(service.second->name());
      }

      for (auto request : m_requestHandlerMap) {
        requestHandlerList.push_back(request.second->name());
      }

      for (auto command : m_commandHandlerMap) {
        commandHandlerList.push_back(command.second->name());
      }

      // get host info
      core::StringMap hostInfo;
      core::fillHostInfo(hostInfo);

      core::json jsonResponse = {{"server", {{"name", m_name}}},
                                 {"host", hostInfo},
                                 {"services", serviceList},
                                 {"requestHandlers", requestHandlerList},
                                 {"commandHandlers", commandHandlerList}};

      std::string serializedJson = jsonResponse.dump();
      auto model = response.createModel<std::string>();
      response.setModel(model);
      model->move(std::move(serializedJson));
    }

    //-------------------------------------------------------------------------------------------------

    bool Server::serviceAlreadyRunning(const std::string &name) {
      if(DimServer::inCallback()) {
        dqm_warning( "Server::serviceAlreadyRunning: can't check for duplicated service on network !" );
        return false;
      }
      DimBrowser browser;
      int nServices = browser.getServices(name.c_str());

      if (nServices == 0)
        return false;

      int serviceType;
      char *serviceName, *format;

      while (1) {
        serviceType = browser.getNextService(serviceName, format);

        if (serviceType == 0)
          break;

        if (serviceType == DimSERVICE)
          return true;
      }

      return false;
    }

    //-------------------------------------------------------------------------------------------------

    bool Server::requestHandlerAlreadyRunning(const std::string &name) {
      if(DimServer::inCallback()) {
        dqm_warning( "Server::requestHandlerAlreadyRunning: can't check for duplicated request handler on network !" );
        return false;
      }
      DimBrowser browser;
      int nServices = browser.getServices(name.c_str());

      if (nServices == 0)
        return false;

      int serviceType;
      char *serviceName, *format;

      while (1) {
        serviceType = browser.getNextService(serviceName, format);

        if (serviceType == 0)
          break;

        if (serviceType == DimRPC)
          return true;
      }

      return false;
    }

    //-------------------------------------------------------------------------------------------------

    bool Server::commandHandlerAlreadyRunning(const std::string &name) {
      if(DimServer::inCallback()) {
        dqm_warning( "Server::commandHandlerAlreadyRunning: can't check for duplicated command on network !" );
        return false;
      }
      DimBrowser browser;
      int nServices = browser.getServices(name.c_str());

      if (nServices == 0)
        return false;

      int serviceType;
      char *serviceName, *format;

      while (1) {
        serviceType = browser.getNextService(serviceName, format);

        if (serviceType == 0)
          break;

        if (serviceType == DimCOMMAND)
          return true;
      }

      return false;
    }

    //-------------------------------------------------------------------------------------------------

    void Server::clientExitHandler() {
      int clientID(DimServer::getClientId());
      std::cout << "Client " << clientID << " exits" << std::endl;
      m_clientExitSignal.process(clientID);
    }

    //-------------------------------------------------------------------------------------------------

    void Server::removeTimer(std::shared_ptr<Timer> timer) {
      auto findIter = std::find(m_timers.begin(), m_timers.end(), timer);

      if (m_timers.end() != findIter)
        m_timers.erase(findIter);
    }

    //-------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------

    Server::Timer::Timer(Server *server) : DimTimer(), m_pServer(server) {
      /* nop */
    }

    //-------------------------------------------------------------------------------------------------

    void Server::Timer::setPeriod(int nSeconds) {
      m_period = nSeconds;
    }

    //-------------------------------------------------------------------------------------------------

    int Server::Timer::period() const {
      return m_period;
    }

    //-------------------------------------------------------------------------------------------------

    void Server::Timer::startTimer() {
      DimTimer::start(m_period);
    }

    //-------------------------------------------------------------------------------------------------

    void Server::Timer::stopTimer() {
      DimTimer::stop();
    }

    //-------------------------------------------------------------------------------------------------

    void Server::Timer::setSingleShot(bool single) {
      m_singleShot = single;
    }

    //-------------------------------------------------------------------------------------------------

    bool Server::Timer::singleShot() const {
      return m_singleShot;
    }

    //-------------------------------------------------------------------------------------------------

    core::Signal<void> &Server::Timer::onTimeout() {
      return m_timeoutSignal;
    }

    //-------------------------------------------------------------------------------------------------

    void Server::Timer::timerHandler() {
      m_timeoutSignal.process();

      if (!this->singleShot()) {
        this->startTimer();
      } else {
        m_pServer->removeTimer(this->shared_from_this());
      }
    }
  }
}
