/*
 * @Author: Ricardo
 * @Date: 2024-04-23 18:03:04
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-04-24 13:11:38
 */
#pragma once
#include "Server.h"
#include "boost/asio/io_context.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <string>

namespace ICEY {
class Server;
namespace Bsio = boost::asio;
class Session : public std::enable_shared_from_this<Session> {

public:
  using Atcp = boost::asio::ip::tcp;
  using Ptr = std::shared_ptr<Session>;

  /**
   * @brief 构造函数
   * @param[in] ioc boost::asio 提供的上下文接口
   * @param[in] server 服务器指针
   */
  Session(Bsio::io_context &ioc, Server *server);
  /**
   * @brief 析构函数
   */
  ~Session();

  /**
   * @brief socket
   * @return 返回一个Asio的网络套接字对象
   */
  Atcp::socket &socket() { return m_sock; }
  /**
   * @brief Session启功，开始进行发送接收操作
   */
  void start();
  /**
   * @brief 获取当前Session的uid
   * @return 返回uid
   */
  std::string getUid() const { return m_uid; }

private:
  /**
   * @brief 读回调
   * @param[in] error boost::asio错误码
   * @param[in] bytes_transferend 要接受的数据大小
   * @param[in] self_ptr 当前Session的智能指针对象
   */
  void handle_read(const boost::system::error_code &error,
                   size_t bytes_transferend, Ptr self_ptr);
  /**
   * @brief 写回调
   * @param[in] error boost::asio错误码
   * @param[in] self_ptr 当前Session的智能指针对象
   */
  void handle_write(const boost::system::error_code &error, Ptr self_ptr);

private:
  // socket
  boost::asio::ip::tcp::socket m_sock;
  // 设置数据最大长度
  enum { max_length = 1024 };
  // 设置buffer
  char m_data[max_length];
  // Server类对象指针
  Server *m_server;
  // uid
  std::string m_uid;
};
class MsgNode {
  friend class Session;

public:
  MsgNode(char *msg, int max_len);
  ~MsgNode();

private:
  // 当前已处理的数据的长度
  int m_cur_len;
  // 数据的总长度
  int m_max_len;
  // 数据域，已接受或者已发送的数据都放在此空间内
  char *m_data;
};
} // namespace ICEY
