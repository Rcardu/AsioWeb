/*
 * @Author: Ricardo
 * @Date: 2024-04-23 18:03:01
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-04-24 13:08:33
 */
#include "Session.h"
#include "boost/uuid/random_generator.hpp"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <functional>
#include <iostream>

namespace ICEY {

Session::Session(Bsio::io_context &ioc, Server *server)
    : m_sock(ioc), m_server(server) {
  m_uid = boost::uuids::to_string(boost::uuids::random_generator()());
}

Session::~Session() { std::cout << "~Session to free\n"; }
void Session::start() {
  memset(m_data, 0, max_length);
  m_sock.async_read_some(
      Bsio::buffer(m_data, max_length),
      [this, capture0 = shared_from_this()](auto &&PH1, auto &&PH2) {
        handle_read(std::forward<decltype(PH1)>(PH1),
                    std::forward<decltype(PH2)>(PH2), capture0);
      });
}
void Session::handle_read(const boost::system::error_code &error,
                          size_t bytes_transferend, Ptr self_ptr) {
  if (error) {
    std::cout << "handle read failed, error is" << error.what() << std::endl
              << "error code: " << error.value() << std::endl;
    m_server->delSession(m_uid);
    return;
  }
  std::cout << "read data is " << m_data << std::endl;
  Bsio::async_write(
      m_sock, Bsio::buffer(m_data, bytes_transferend),
      std::bind(&Session::handle_write, this, std::placeholders::_1, self_ptr));
}
void Session::handle_write(const boost::system::error_code &error,
                           Ptr self_ptr) {
  if (error) {
    std::cout << "handle write failed, error is" << error.what() << std::endl
              << "error code: " << error.value() << std::endl;
    m_server->delSession(m_uid);
    return;
  }
  memset(m_data, 0, max_length);
  m_sock.async_read_some(Bsio::buffer(m_data, max_length),
                         [this, self_ptr](auto &&PH1, auto &&PH2) {
                           handle_read(std::forward<decltype(PH1)>(PH1),
                                       std::forward<decltype(PH2)>(PH2),
                                       self_ptr);
                         });
}
MsgNode::MsgNode(char *msg, int max_len) {
  m_data = new char[max_len];
  memcpy(m_data, msg, max_len);
}
MsgNode::~MsgNode() { delete[] m_data; }

} // namespace ICEY
