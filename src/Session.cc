/*
 * @Author: Ricardo
 * @Date: 2024-04-23 18:03:01
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-08 17:16:33
 */
#include "Session.h"
#include "LogicSystem.h"
#include "MsgNode.h"
#include "boost/asio/bind_executor.hpp"
#include "boost/asio/detail/socket_holder.hpp"
#include "boost/asio/write.hpp"
#include "boost/uuid/random_generator.hpp"
#include "deconst.h"
#include "msg.pb.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <memory>
#include <mutex>

namespace ICEY {

Session::Session(Bsio::io_context &ioc, Server *server)
    : m_sock(ioc), m_server(server),
      m_recv_head_node(std::make_shared<MsgNode>(HEAD_TOTAL_LEN)),
      m_strand(ioc.get_executor()) {
  m_uid = boost::uuids::to_string(boost::uuids::random_generator()());
}

Session::~Session() { std::cout << "~Session to free\n"; }
void Session::start() {
// STAR:服务器宏READ_ALL_MSG
#ifdef READ_ALL_MSG
  memset(m_data, 0, MAX_LENGTH);
// STAR:服务器宏，使用线程池USE_STRAND_LIST
#ifdef USE_STRAND_LIST
  m_sock.async_read_some(
      Bsio::buffer(m_data, MAX_LENGTH),
      boost::asio::bind_executor(m_strand, [this](auto &&PH1, auto &&PH2) {
        handle_read(std::forward<decltype(PH1)>(PH1),
                    std::forward<decltype(PH2)>(PH2), shared_from_this());
      }));
#endif // USE_STRAND_LIST
// STAR:服务器宏，使用服务池NOUSE_STRAND_LIST
#ifdef NOUSE_STRAND_LIST
  m_sock.async_read_some(
      Bsio::buffer(m_data, MAX_LENGTH), [this](auto &&PH1, auto &&PH2) {
        handle_read(std::forward<decltype(PH1)>(PH1),
                    std::forward<decltype(PH2)>(PH2), shared_from_this());
      });

#endif // NOUSE_STRAND_LIST
#endif // READ_ALL_MSG
  // STAR:服务器宏READ_HEAD_MSG
#ifdef READ_HEAD_MSG
  m_recv_head_node->clear();
// STAR:服务器宏，使用线程池USE_STRAND_LIST
#ifdef USE_STRAND_LSIT
  boost::asio::async_read(
      m_sock, boost::asio::buffer(m_recv_head_node->getData(), HEAD_LENGTH),
      boost::asio::bind_executor(m_strand, [this](auto &&PH1, auto &&PH2) {
        handle_read_head(std::forward<decltype(PH1)>(PH1),
                         std::forward<decltype(PH2)>(PH2), shared_from_this());
      }));
#endif // USE_STRAND_LIST
// STAR:服务器宏，使用服务池NOUSE_STRAND_LIST
#ifdef NOUSE_STRAND_LIST
  boost::asio::async_read(
      m_sock, boost::asio::buffer(m_recv_head_node->getData(), HEAD_LENGTH),
      [this](auto &&PH1, auto &&PH2) {
        handle_read_head(std::forward<decltype(PH1)>(PH1),
                         std::forward<decltype(PH2)>(PH2), shared_from_this());
      });
#endif // NOUSE_STRAND_LIST
#endif // READ_HEAD_MSG
}
void Session::close() {
  m_sock.close();
  m_b_close = true;
}

void Session::send(const char *msg, int max_length, int16_t msgid) {

  std::lock_guard<std::mutex> lock(m_send_lock);

  int send_que_size = m_send_que.size();
  if (send_que_size > MAX_SENDQUE) {
    std::cout << "Session: " << m_uid << "send que fulled, size is"
              << MAX_SENDQUE << std::endl;
    return;
  }
  m_send_que.push(std::make_shared<SendNode>(msg, max_length, msgid));
  if (send_que_size > 0) {
    return;
  }

  auto &msgnode = m_send_que.front();
// STAR:服务器宏，使用线程池USE_STRAND_LIST
#ifdef USE_STRAND_LIST
  boost::asio::async_write(
      m_sock, boost::asio::buffer(msgnode->getData(), msgnode->getTotallen()),
      boost::asio::bind_executor(
          m_strand, [this, msgnode](auto &&PH1, auto && /*PH2*/) {
            handle_write(std::forward<decltype(PH1)>(PH1), shared_from_this());
          }));
#endif // USE_STRAND_LIST
// STAR:服务器宏，使用服务池NOUSE_STRAND_LIST
#ifdef NOUSE_STRAND_LIST
  boost::asio::async_write(
      m_sock, boost::asio::buffer(msgnode->getData(), msgnode->getTotallen()),
      [this, msgnode](auto &&PH1, auto && /*PH2*/) {
        handle_write(std::forward<decltype(PH1)>(PH1), shared_from_this());
      });

#endif // NOUSE_STRAND_LIST
}
void Session::send(const std::string &msg, int16_t msgid) {
  std::lock_guard<std::mutex> lock(m_send_lock);

  int send_que_size = m_send_que.size();
  if (send_que_size > MAX_SENDQUE) {
    std::cout << "Session: " << m_uid << "send que fulled, size is"
              << MAX_SENDQUE << std::endl;
    return;
  }
  m_send_que.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
  if (send_que_size > 0) {
    return;
  }
  auto &msgnode = m_send_que.front();
// STAR:服务器宏，使用线程池USE_STRAND_LIST
#ifdef USE_STRAND_LIST
  boost::asio::async_write(
      m_sock, boost::asio::buffer(msgnode->getData(), msgnode->getTotallen()),
      boost::asio::bind_executor(
          m_strand, [this, msgnode](auto &&PH1, auto && /*PH2*/) {
            handle_write(std::forward<decltype(PH1)>(PH1), shared_from_this());
          }));
#endif // USE_STRAND_LIST
// STAR:服务器宏，使用服务池NOUSE_STRAND_LIST
#ifdef NOUSE_STRAND_LIST
  boost::asio::async_write(
      m_sock, boost::asio::buffer(msgnode->getData(), msgnode->getTotallen()),
      [this, msgnode](auto &&PH1, auto && /*PH2*/) {
        handle_write(std::forward<decltype(PH1)>(PH1), shared_from_this());
      });

#endif // NOUSE_STRAND_LIST
}
void Session::PrintRecvData(char *data, int length) {
  std::stringstream ss;
  std::string result = "0x";
  for (int i = 0; i < length; i++) {
    std::string hexstr;
    ss << std::hex << std::setw(2) << std::setfill('0') << int(data[i])
       << std::endl;
    ss >> hexstr;
    result += hexstr;
  }
  std::cout << "receive raw data is : " << result << std::endl;
}
void Session::handle_read(const boost::system::error_code &error,
                          size_t bytes_transferend, Ptr self_ptr) {
  if (error) {
    std::cout << "handle read failed, error is" << error.what() << std::endl
              << "error code: " << error.value() << std::endl;
    m_server->delSession(self_ptr->m_uid);
    return;
  }
  // printRecvData(m_data, bytes_transferend);
  // std::chrono::milliseconds dura(2000);
  // std::this_thread::sleep_for(dura);
  // 已经移动的字符数
  int copy_len{0};
  while (bytes_transferend > 0) {
    if (!m_b_head_parse) {
      // 收到的数据不足头部大小
      if (bytes_transferend + m_recv_head_node->getCurlen() < HEAD_TOTAL_LEN) {
        memcpy(m_recv_head_node->getData() + m_recv_head_node->getCurlen(),
               m_data + copy_len, bytes_transferend);
        m_recv_head_node->addCurlen(bytes_transferend);
        ::memset(m_data, 0, MAX_LENGTH);
// STAR:服务器宏，使用线程池，USE_STRAND_LIST
#ifdef USE_STRAND_LIST
        m_sock.async_read_some(
            boost::asio::buffer(m_data, MAX_LENGTH),
            boost::asio::bind_executor(
                m_strand, [this, self_ptr](auto &&PH1, auto &&PH2) {
                  handle_read(std::forward<decltype(PH1)>(PH1),
                              std::forward<decltype(PH2)>(PH2), self_ptr);
                }));
#endif // USE_STRAND_LIST
// STAR:服务器宏，使用服务池，NOUSE_STRAND_LIST
#ifdef NOUSE_STRAND_LIST
        m_sock.async_read_some(boost::asio::buffer(m_data, MAX_LENGTH),
                               [this, self_ptr](auto &&PH1, auto &&PH2) {
                                 handle_read(std::forward<decltype(PH1)>(PH1),
                                             std::forward<decltype(PH2)>(PH2),
                                             self_ptr);
                               });

#endif // NOUSE_STRAND_LIST
        return;
      }
      // 收到的数据比头部多
      // 头部剩余未复制的长度
      int head_remain = HEAD_TOTAL_LEN - m_recv_head_node->getCurlen();
      memcpy(m_recv_head_node->getData() + m_recv_head_node->getCurlen(),
             m_data + copy_len, head_remain);
      // 更新已处理的data长度和剩余未处理的长度
      copy_len += head_remain;
      bytes_transferend -= head_remain;
      // 获取头部ID数据
      int16_t msg_id = 0;
      memcpy(&msg_id, m_recv_head_node->getData(), HEAD_ID_LEN);
      msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
      std::cout << "msg id is " << msg_id << std::endl;
      // id
      if (msg_id > MAX_LENGTH) {
        std::cout << "invalid msg_is is " << msg_id << std::endl;
        m_server->delSession(self_ptr->m_uid);
        return;
      }
      // 获取头部长度数据
      int16_t msg_len = 0;
      memcpy(&msg_len, m_recv_head_node->getData() + HEAD_ID_LEN,
             HEAD_DATA_LEN);
      // 网络字节序转为本地字节序
      msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
      std::cout << "data_len is: " << msg_len << std::endl;
      // 头部长度非法
      if (msg_len > MAX_LENGTH) {
        std::cout << "invalid data length is: " << msg_len << std::endl;
        m_server->delSession(m_uid);
        return;
      }
      m_recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);

      // 消息长度小于规定的长度，说明数据未收全，先将部分消息放到节点里
      if (bytes_transferend < msg_len) {
        memcpy(m_recv_msg_node->getData() + m_recv_msg_node->getCurlen(),
               m_data + copy_len, bytes_transferend);
        m_recv_msg_node->addCurlen(bytes_transferend);
        ::memset(m_data, 0, MAX_LENGTH);
// STAR:服务器宏，使用线程池，USE_STRAND_LIST
#ifdef USE_STRAND_LIST
        m_sock.async_read_some(
            boost::asio::buffer(m_data, MAX_LENGTH),
            boost::asio::bind_executor(
                m_strand, [this, self_ptr](auto &&PH1, auto &&PH2) {
                  handle_read(std::forward<decltype(PH1)>(PH1),
                              std::forward<decltype(PH2)>(PH2), self_ptr);
                }));
#endif // USE_STRAND_LIST
// STAR:服务器宏，使用服务池，NOUSE_STRAND_LIST
#ifdef NOUSE_STRAND_LIST
        m_sock.async_read_some(boost::asio::buffer(m_data, MAX_LENGTH),
                               [this, self_ptr](auto &&PH1, auto &&PH2) {
                                 handle_read(std::forward<decltype(PH1)>(PH1),
                                             std::forward<decltype(PH2)>(PH2),
                                             self_ptr);
                               });
#endif // NOUSE_STRAND_LIST
        m_b_head_parse = true;
        return;
      }
      memcpy(m_recv_msg_node->getData() + m_recv_msg_node->getCurlen(),
             m_data + copy_len, msg_len);
      m_recv_msg_node->addCurlen(msg_len);
      copy_len += msg_len;
      bytes_transferend -= msg_len;
      m_recv_msg_node->getData()[m_recv_msg_node->getTotallen()] = '\0';
      LogicSystem::GetInstance()->postMsgToQueue(
          std::make_shared<LogicNode>(shared_from_this(), m_recv_msg_node));
// STAR:服务器宏THREAD_SEND_S
#ifdef THREAD_SEND_S
      std::cout << "receive data is " << m_recv_msg_node->getData()
                << std::endl;
      // 此处可以调用Send发送测试
      send(m_recv_msg_node->getData(), m_recv_msg_node->getTotallen(), msg_id);
#endif // THREAD_SEND_S
// STAR:服务器宏PROTO_SEND_S
#ifdef PROTO_SEND_S
      MsgData msg_data;
      std::string receive_data;
      msg_data.ParseFromString(std::string(m_recv_msg_node->getData(),
                                           m_recv_msg_node->getTotallen()));
      std::cout << "receive data id is " << msg_data.id() << " msg data is "
                << msg_data.data() << std::endl;
      std::string return_str =
          "server has received msg, msg data is " + msg_data.data();
      MsgData msg_return;
      msg_return.set_id(msg_data.id());
      msg_return.set_data(return_str);
      msg_return.SerializeToString(&return_str);
      send(return_str, msg_data.id());
#endif // PROTO_SEND_S
// STAR:服务器宏JSON_SEND_S
#ifdef JSON_SEND_S
      Json::Reader reader;
      Json::Value root;
      reader.parse(std::string(m_recv_msg_node->getData(),
                               m_recv_msg_node->getTotallen()),
                   root);
      std::cout << "receive msg id is " << root["id"].asInt() << " msg data is "
                << root["data"].asString() << std::endl;
      root["data"] =
          "server has received msg, msg data is " + root["data"].asString();
      std::string return_str = root.toStyledString();
      send(return_str, root["id"].asInt());
#endif // JSON_SEND_S
       // 继续轮询剩余未处理数据
      m_b_head_parse = false;
      m_recv_head_node->clear();
      if (bytes_transferend <= 0) {
        ::memset(self_ptr->m_data, 0, MAX_LENGTH);
// STAR:服务器宏，使用线程池，USE_STRAND_LIST
#ifdef USE_STRAND_LIST
        m_sock.async_read_some(
            boost::asio::buffer(m_data, MAX_LENGTH),
            boost::asio::bind_executor(
                m_strand, [this, self_ptr](auto &&PH1, auto &&PH2) {
                  handle_read(std::forward<decltype(PH1)>(PH1),
                              std::forward<decltype(PH2)>(PH2), self_ptr);
                }));
#endif // USE_STRAND_LIST
// STAR:服务器宏，使用服务池,NOUSE_STRAND_LSIT
#ifdef NOUSE_STRAND_LIST
        m_sock.async_read_some(boost::asio::buffer(m_data, MAX_LENGTH),
                               [this, self_ptr](auto &&PH1, auto &&PH2) {
                                 handle_read(std::forward<decltype(PH1)>(PH1),
                                             std::forward<decltype(PH2)>(PH2),
                                             self_ptr);
                               });

#endif // NOUSE_STRAND_LIST
        return;
      }
      continue;
    }
    // 已经处理完头部，处理上次未接受完的消息数据
    // 接收的数据仍不足剩余未处理的
    int remain_msg =
        m_recv_msg_node->getTotallen() - m_recv_msg_node->getCurlen();
    if (bytes_transferend < remain_msg) {
      memcpy(m_recv_msg_node->getData() + m_recv_msg_node->getCurlen(),
             m_data + copy_len, bytes_transferend);
      m_recv_msg_node->addCurlen(bytes_transferend);
      ::memset(m_data, 0, MAX_LENGTH);
// STAR:服务器宏，使用线程池，USE_STRAND_LIST
#ifdef USE_STRAND_LIST
      m_sock.async_read_some(
          boost::asio::buffer(m_data, MAX_LENGTH),
          boost::asio::bind_executor(
              m_strand, [this, self_ptr](auto &&PH1, auto &&PH2) {
                handle_read(std::forward<decltype(PH1)>(PH1),
                            std::forward<decltype(PH2)>(PH2), self_ptr);
              }));
#endif // USE_STRAND_LIST
// STAR:服务器宏，使用服务池，NOUSE_STRAND_LIST
#ifdef NOUSE_STRAND_LIST
      m_sock.async_read_some(boost::asio::buffer(m_data, MAX_LENGTH),
                             [this, self_ptr](auto &&PH1, auto &&PH2) {
                               handle_read(std::forward<decltype(PH1)>(PH1),
                                           std::forward<decltype(PH2)>(PH2),
                                           self_ptr);
                             });

#endif // NOUSE_STRAND_LIST
      return;
    }
    memcpy(m_recv_msg_node->getData() + m_recv_msg_node->getCurlen(),
           m_data + copy_len, remain_msg);
    m_recv_msg_node->addCurlen(remain_msg);
    bytes_transferend -= remain_msg;
    copy_len += remain_msg;
    m_recv_msg_node->getData()[m_recv_msg_node->getTotallen()] = '\0';
    LogicSystem::GetInstance()->postMsgToQueue(
        std::make_shared<LogicNode>(shared_from_this(), m_recv_msg_node));
// STAR:服务器宏THREAD_SEND_S
#ifdef THREAD_SEND_S
    std::cout << "receive data is " << m_recv_msg_node->getData() << std::endl;
    send(m_recv_msg_node->getData(), m_recv_msg_node->getTotallen(), msgid);
#endif // THREAD_SEND_S
// STAR:服务器宏PROTO_SEND_S
#ifdef PROTO_SEND_S
    MsgData msg_data;
    std::string receive_data;
    msg_data.ParseFromString(std::string(m_recv_msg_node->getData(),
                                         m_recv_msg_node->getTotallen()));
    std::cout << "receive data id is " << msg_data.id() << "msg data is "
              << msg_data.data() << std::endl;
    std::string return_str =
        "server has received msg, msg data is " + msg_data.data();
    MsgData msg_return;
    msg_return.set_id(msg_data.id());
    msg_return.set_data(return_str);
    msg_return.SerializeToString(&return_str);
    // 此处调用send测试
    send(return_str, msg_data.id());
#endif // PROTO_SEND_S
// STAR:服务器宏JSON_SEND_S
#ifdef JSON_SEND_S
    Json::Reader reader;
    Json::Value root;
    reader.parse(
        std::string(m_recv_msg_node->getData(), m_recv_msg_node->getTotallen()),
        root);
    std::cout << "receive msg is " << root["id"].asInt() << " msg data is "
              << root["data"].asString() << std::endl;
    root["data"] =
        "server has received msg, msg data is " + root["data"].asString();
    std::string return_str = root.toStyledString();
    send(return_str, root["id"].asInt());
#endif // JSON_SEND_S
    // 继续轮询未处理的数据
    m_b_head_parse = false;
    m_recv_head_node->clear();
    if (bytes_transferend <= 0) {
      ::memset(m_data, 0, MAX_LENGTH);
// STAR:服务器宏，使用线程池，USE_STRAND_LIST
#ifdef USE_STRAND_LIST
      m_sock.async_read_some(
          boost::asio::buffer(m_data, MAX_LENGTH),
          boost::asio::bind_executor(
              m_strand, [this, self_ptr](auto &&PH1, auto &&PH2) {
                handle_read(std::forward<decltype(PH1)>(PH1),
                            std::forward<decltype(PH2)>(PH2), self_ptr);
              }));
#endif // USE_STRAND_LIST
// STAR:服务器宏，使用服务池，NOUSE_STRAND_LIST
#ifdef NOUSE_STRAND_LIST
      m_sock.async_read_some(boost::asio::buffer(m_data, MAX_LENGTH),
                             [this, self_ptr](auto &&PH1, auto &&PH2) {
                               handle_read(std::forward<decltype(PH1)>(PH1),
                                           std::forward<decltype(PH2)>(PH2),
                                           self_ptr);
                             });

#endif // NOUSE_STRAND_LIST
      return;
    }
  }
}

void Session::handle_read_head(const boost::system::error_code &error,
                               size_t bytes_transferend, Ptr self_ptr) {
  if (error) {
    std::cout << "handle read head failed, error is " << error.what()
              << std::endl
              << "error code: " << error.value() << std::endl;
    close();
    m_server->delSession(self_ptr->m_uid);
    return;
  }
  if (bytes_transferend < HEAD_LENGTH) {
    std::cout << "read head length error";
    close();
    m_server->delSession(self_ptr->m_uid);
    return;
  }
  // 头部已经收到，解析头部
  int16_t data_len = 0;
  memcpy(&data_len, m_recv_head_node->getData(), HEAD_LENGTH);
  // data_len =
  // boost::asio::detail::socket_ops::network_to_host_short(data_len);
  std::cout << "data len is " << data_len << std::endl;
  // 数据太多，不合规
  if (data_len > MAX_LENGTH) {
    std::cout << "invalid data length is " << data_len << std::endl;
    m_server->delSession(self_ptr->m_uid);
    return;
  }

  m_recv_msg_node = std::make_shared<RecvNode>(data_len, 1001);
// STAR:服务器宏，使用线程池，USE_STRAND_LIST
#ifdef USE_STRAND_LSIT
  boost::asio::async_read(
      m_sock,
      boost::asio::buffer(m_recv_msg_node->getData(),
                          m_recv_msg_node->getTotallen()),
      boost::asio::bind_executor(m_strand, [this](auto &&PH1, auto &&PH2) {
        handle_read_msg(std::forward<decltype(PH1)>(PH1),
                        std::forward<decltype(PH2)>(PH2), shared_from_this());
      }));
#endif // USE_STRAND_LIST
// STAR:服务器宏，使用服务池，NOUSE_STRAND_LIST
#ifdef NOUSE_STRAND_LIST
  boost::asio::async_read(m_sock,
                          boost::asio::buffer(m_recv_msg_node->getData(),
                                              m_recv_msg_node->getTotallen()),
                          [this](auto &&PH1, auto &&PH2) {
                            handle_read_msg(std::forward<decltype(PH1)>(PH1),
                                            std::forward<decltype(PH2)>(PH2),
                                            shared_from_this());
                          });

#endif // NOUSE_STRAND_LIST
}
void Session::handle_read_msg(const boost::system::error_code &error,
                              size_t bytes_transferend, Ptr self_ptr) {
  if (error) {
    std::cout << "handle read msg failed, error is " << error.what()
              << std::endl
              << "error code: " << error.value() << std::endl;
    std::cout << "receive message is:" << std::endl;
    close();
    m_server->delSession(self_ptr->m_uid);
    return;
  }
  PrintRecvData(m_data, bytes_transferend);
  std::chrono::milliseconds dura(2000);
  std::this_thread::sleep_for(dura);
  m_recv_msg_node->getData()[m_recv_msg_node->getTotallen()] = '\0';
  std::cout << "receive data is " << m_recv_msg_node->getData() << std::endl;
  send(m_recv_msg_node->getData(), m_recv_msg_node->getTotallen());
  // 再次接收头部数据
  m_recv_head_node->clear();
  // STAR: 服务器宏，使用线程池，USE_STRAND_LIST
#ifdef USE_STRAND_LIST
  boost::asio::async_read(
      m_sock, boost::asio::buffer(m_recv_head_node->getData(), HEAD_LENGTH),
      boost::asio::bind_executor(m_strand, [this](auto &&PH1, auto &&PH2) {
        handle_read_head(std::forward<decltype(PH1)>(PH1),
                         std::forward<decltype(PH2)>(PH2), shared_from_this());
      }));
#endif // USE_STRAND_LIST
// STAR:服务器宏，使用服务池，NOUSE_STRAND_LIST
#ifdef NOUSE_STRAND_LIST
  boost::asio::async_read(
      m_sock, boost::asio::buffer(m_recv_head_node->getData(), HEAD_LENGTH),
      [this](auto &&PH1, auto &&PH2) {
        handle_read_head(std::forward<decltype(PH1)>(PH1),
                         std::forward<decltype(PH2)>(PH2), shared_from_this());
      });

#endif // NOUSE_STRAND_LIST
}

void Session::handle_write(const boost::system::error_code &error,
                           Ptr self_ptr) {
  if (error) {
    std::cout << "handle write failed, error is" << error.what() << std::endl
              << "error code: " << error.value() << std::endl;
    m_server->delSession(self_ptr->m_uid);
    return;
  }
  {
    std::lock_guard<std::mutex> lock(m_send_lock);
    // 当此函数被调用时，说明队列的首元素已经被发送完毕
    m_send_que.pop();
    if (!m_send_que.empty()) {
      auto &msgnode = m_send_que.front();
// STAR:服务器宏，使用线程池，USE_STRAND_LIST
#ifdef USE_STRAND_LIST
      boost::asio::async_write(
          m_sock,
          boost::asio::buffer(msgnode->getData(), msgnode->getTotallen()),
          boost::asio::bind_executor(m_strand, [this, msgnode](auto &&PH1,
                                                               auto &&) {
            handle_write(std::forward<decltype(PH1)>(PH1), shared_from_this());
          }));
#endif // USE_STRAND_LISE
// STAR:服务器宏，使用服务池，NOUSE_STRAND_LIST
#ifdef NOUSE_STRAND_LIST
      boost::asio::async_write(
          m_sock,
          boost::asio::buffer(msgnode->getData(), msgnode->getTotallen()),
          [this, msgnode](auto &&PH1, auto &&) {
            handle_write(std::forward<decltype(PH1)>(PH1), shared_from_this());
          });
#endif // NOUSE_STRAND_LISE
    }
  }
}
LogicNode::LogicNode(std::shared_ptr<Session> session,
                     std::shared_ptr<RecvNode> recvnode)
    : m_session(session), m_recvnode(recvnode) {}
// STAR:消息节点宏MSG_PROV_S
#ifdef MSG_PROV_S
MsgNode::MsgNode(const char *msg, int16_t max_len)
    : m_total_len(max_len + HEAD_LENGTH), m_data(new char[m_total_len + 1]()),
      m_cur_len(0) {
  // 转为网络字节序
  int max_len_host =
      boost::asio::detail::socket_ops::host_to_network_short(max_len);
  memcpy(m_data, &max_len_host, HEAD_LENGTH);
  memcpy(m_data + HEAD_LENGTH, msg, max_len);
  m_data[m_total_len] = '\0';
}
MsgNode::MsgNode(int16_t max_len)
    : m_total_len(max_len), m_data(new char[m_total_len + 1]()), m_cur_len(0) {}
MsgNode::~MsgNode() { delete[] m_data; }
#endif // MSG_PROV_S
} // namespace ICEY
