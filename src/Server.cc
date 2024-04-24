/*
 * @Author: Ricardo
 * @Date: 2024-04-23 19:06:31
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-04-24 12:59:32
 */

#include "Server.h"
#include <iostream>

namespace ICEY {

Server::Server(boost::asio::io_context &ioc, short port)
    : m_ioc(ioc), m_acceptor(ioc, Btcp::endpoint(Btcp::v4(), port)) {
  std::cout << "Server start success, on port: " << port << std::endl;
  start_accept();
}

void Server::delSession(const std::string &uid) { m_sessions.erase(uid); }
void Server::start_accept() {
  auto new_session = std::make_shared<Session>(m_ioc, this);
  m_acceptor.async_accept(
      new_session->socket(), [this, new_session](auto &&PH1) {
        handle_accept(new_session, std::forward<decltype(PH1)>(PH1));
      });
}
void Server::handle_accept(std::shared_ptr<Session> ptr,
                           const boost::system::error_code &error) {
  if (error) {
    std::cout << "session accept failed, error is " << error.what()
              << std::endl;
  }
  ptr->start();
  m_sessions.insert(std::make_pair(ptr->getUid(), ptr));
  start_accept();
}

} // namespace ICEY
