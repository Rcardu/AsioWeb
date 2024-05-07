/*
 * @Author: Ricardo
 * @Date: 2024-04-23 18:03:04
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-07 16:03:26
 * @Title: 会话类
 */

#pragma once
#include "MsgNode.h"
#include "Server.h"
#include "boost/asio/io_context.hpp"
#include "deconst.h"
#include <boost/asio.hpp>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

namespace ICEY {

class Server;
class MsgNode;
class SendNode;
class RecvNode;

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
   * @brief 关闭Socket
   */
  void close();
  /**
   * @brief 发送数据
   * @param[in] msg 要发送的数据
   * @param[in] max_length 要发送的数据的最大长度
   */
  void send(const char *msg, int max_length, int16_t msgid);
  /**
   * @brief 发送数据
   * @param[in] msg 要发送的数据
   */
  void send(const std::string &msg, int16_t msgid);
  /**
   * @brief 获取当前Session的uid
   * @return 返回uid
   */
  std::string getUid() const { return m_uid; }

  /**
   * @brief 打印接收到的二进制数据
   * @param[in] data 接收到的数据
   * @param[in] length 接收到的数据的长度
   */
  static void PrintRecvData(char *data, int length);

private:
  /**
   * @brief 读回调
   * @param[in] error boost::asio错误码
   * @param[in] bytes_transferend 要接受的数据大小
   * @param[in] self_ptr 当前Session的智能指针对象
   */
  void handle_read(const boost::system::error_code &error,
                   size_t bytes_transferend, Ptr self_ptr);
  void handle_read_head(const boost::system::error_code &error,
                        size_t bytes_transferend, Ptr self_ptr);
  void handle_read_msg(const boost::system::error_code &error,
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
  char m_data[MAX_LENGTH];
  // Server类对象指针
  Server *m_server;
  // uid
  std::string m_uid;
  // 发送队列
  std::queue<std::shared_ptr<SendNode>> m_send_que;
  // 维持有序性的锁
  std::mutex m_send_lock;
  // 收到的消息结构
  std::shared_ptr<RecvNode> m_recv_msg_node;
  // 头部是否解析完成
  bool m_b_head_parse{false};
  // 收到的头部结构
  std::shared_ptr<MsgNode> m_recv_head_node;
  // socket是否已被关闭
  bool m_b_close{false};
};
class LogicNode {
  friend class LogicSystem;

public:
  /**
   * @brief 构造函数
   * @param[in] session 连接的信息
   * @param[in] recvnode 消息发送信息
   */
  LogicNode(std::shared_ptr<Session> sessin,
            std::shared_ptr<RecvNode> recvnode);

private:
  // 含有连接信息的会话指针对象
  std::shared_ptr<Session> m_session;
  // 含有要发送的已初始化好的消息节点指针对象
  std::shared_ptr<RecvNode> m_recvnode;
};

// STAR:消息节点宏MSG_PROV_S
#ifdef MSG_PROV_S
class MsgNode {
  friend class Session;

public:
  using Ptr = std::shared_ptr<MsgNode>;

  /**
   * @brief 构造函数（发送）
   * @param[in] msg 要发送的数据
   * @param[in] max_len 要发送的数据的长度
   */
  MsgNode(const char *msg, int16_t max_len);
  /**
   * @brief 构造函数（接收）
   * @param [in] max_len 要发送的数据的长度
   */
  explicit MsgNode(int16_t max_len);
  /**
   * @brief 析构函数
   */
  ~MsgNode();

  /**
   * @brief 清除m_data中的数据
   */
  void clear() {
    ::memset(m_data, 0, m_total_len);
    m_cur_len = 0;
  }

private:
  // 当前已处理的数据的长度
  int m_cur_len;
  // 数据的总长度
  int m_total_len;
  // 数据域，已接受或者已发送的数据都放在此空间内
  char *m_data;
};
#endif // MSG_PROV_S

} // namespace ICEY
