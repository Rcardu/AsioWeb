/*
 * @Author: Ricardo
 * @Date: 2024-04-23 18:10:36
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-04-24 13:16:58
 * @Title: 服务器类
 */
#pragma once
#include "Session.h"
#include "boost/asio/io_context.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace ICEY {

class Session;
class Server {
public:
  using Btcp = boost::asio::ip::tcp;

  /**
   * @brief 构造函数
   * @param[in] ioc boost::asio 的上下文
   * @param[in] port 端口
   */
  Server(boost::asio::io_context &ioc, short port);
  ~Server() { std::cout << "~Server free .\n"; }
  /**
   * @brief 删除当前uid所对应的Session
   * @param[in] uid 需要删除的Session的uid
   */
  void delSession(const std::string &uid);

private:
  /**
   * @brief 开始接收连接
   */
  void start_accept();
  /**
   * @brief 异步接收连接
   * @param[in] ptr 需要处理接收到的连接的Session对象
   * @param[in] error boost::system 提供的错误码
   */
  void handle_accept(std::shared_ptr<Session> ptr,
                     const boost::system::error_code &error);

private:
  // 上下文
  boost::asio::io_context &m_ioc;
  // boost::asio::ip::tcp 的连接接收器
  Btcp::acceptor m_acceptor;
  // 用来存储每个session
  std::map<std::string, std::shared_ptr<Session>> m_sessions;
};
} // namespace ICEY
