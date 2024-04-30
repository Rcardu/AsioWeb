/*
 * @Author: Ricardo
 * @Date: 2024-04-23 18:03:01
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-04-25 12:43:35
 */
#include "Session.h"
#include "boost/asio/write.hpp"
#include "boost/uuid/random_generator.hpp"
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
      m_recv_head_node(std::make_shared<MsgNode>(HEAD_LENGTH)) {
  m_uid = boost::uuids::to_string(boost::uuids::random_generator()());
}

Session::~Session() { std::cout << "~Session to free\n"; }
void Session::start() {
  // STAR:服务器宏READ_ALL_MSG
#ifdef READ_ALL_MSG
  memset(m_data, 0, max_length);
  m_sock.async_read_some(
      Bsio::buffer(m_data, max_length), [this](auto &&PH1, auto &&PH2) {
        handle_read(std::forward<decltype(PH1)>(PH1),
                    std::forward<decltype(PH2)>(PH2), shared_from_this());
      });
#endif // READ_ALL_MSG
  // STAR:服务器宏READ_HEAD_MSG
#ifdef READ_HEAD_MSG
  m_recv_head_node->clear();
  boost::asio::async_read(
      m_sock, boost::asio::buffer(m_recv_head_node->m_data, HEAD_LENGTH),
      [this](auto &&PH1, auto &&PH2) {
        handle_read_head(std::forward<decltype(PH1)>(PH1),
                         std::forward<decltype(PH2)>(PH2), shared_from_this());
      });
#endif // READ_HEAD_MSG
}
void Session::close() {
  m_sock.close();
  m_b_close = true;
}

void Session::send(const char *msg, int max_length) {

  std::lock_guard<std::mutex> lock(m_send_lock);

  int send_que_size = m_send_que.size();
  if (send_que_size > MAX_SENDQUE) {
    std::cout << "Session: " << m_uid << "send que fulled, size is"
              << MAX_SENDQUE << std::endl;
    return;
  }
  m_send_que.push(std::make_shared<MsgNode>(msg, max_length));
  if (send_que_size > 0) {
    return;
  }

  auto &msgnode = m_send_que.front();
  boost::asio::async_write(
      m_sock, boost::asio::buffer(msgnode->m_data, msgnode->m_total_len),
      [this, msgnode](auto &&PH1, auto && /*PH2*/) {
        handle_write(std::forward<decltype(PH1)>(PH1), shared_from_this());
      });
}
void Session::send(const std::string &msg) {
  std::lock_guard<std::mutex> lock(m_send_lock);

  int send_que_size = m_send_que.size();
  if (send_que_size > MAX_SENDQUE) {
    std::cout << "Session: " << m_uid << "send que fulled, size is"
              << MAX_SENDQUE << std::endl;
    return;
  }
  m_send_que.push(std::make_shared<MsgNode>(msg.c_str(), msg.length()));
  if (send_que_size > 0) {
    return;
  }
  auto &msgnode = m_send_que.front();
  boost::asio::async_write(
      m_sock, boost::asio::buffer(msgnode->m_data, msgnode->m_total_len),
      [this, msgnode](auto &&PH1, auto && /*PH2*/) {
        handle_write(std::forward<decltype(PH1)>(PH1), shared_from_this());
      });
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
      if (bytes_transferend + m_recv_head_node->m_cur_len < HEAD_LENGTH) {
        memcpy(m_recv_head_node->m_data + m_recv_head_node->m_cur_len,
               m_data + copy_len, bytes_transferend);
        m_recv_head_node->m_cur_len += bytes_transferend;
        ::memset(m_data, 0, MAX_LENGTH);
        m_sock.async_read_some(boost::asio::buffer(m_data, MAX_LENGTH),
                               [this, self_ptr](auto &&PH1, auto &&PH2) {
                                 handle_read(std::forward<decltype(PH1)>(PH1),
                                             std::forward<decltype(PH2)>(PH2),
                                             self_ptr);
                               });
        return;
      }
      // 收到的数据比头部多
      // 头部剩余未复制的长度
      int head_remain = HEAD_LENGTH - m_recv_head_node->m_cur_len;
      memcpy(m_recv_head_node->m_data + m_recv_head_node->m_cur_len,
             m_data + copy_len, head_remain);
      // 更新已处理的data长度和剩余未处理的长度
      copy_len += head_remain;
      bytes_transferend -= head_remain;
      // 获取头部长度
      uint16_t data_len = 0;
      memcpy(&data_len, m_recv_head_node->m_data, HEAD_LENGTH);
      // 网络字节序转为本地字节序
      data_len =
          boost::asio::detail::socket_ops::network_to_host_short(data_len);
      std::cout << "data_len is: " << data_len << std::endl;
      // 头部长度非法
      if (data_len > MAX_LENGTH) {
        std::cout << "invalid data length is: " << data_len << std::endl;
        m_server->delSession(m_uid);
        return;
      }
      m_recv_msg_node = std::make_shared<MsgNode>(data_len);

      // 消息长度小于规定的长度，说明数据未收全，先将部分消息放到节点里
      if (bytes_transferend < data_len) {
        memcpy(m_recv_msg_node->m_data + m_recv_msg_node->m_cur_len,
               m_data + copy_len, bytes_transferend);
        m_recv_msg_node->m_cur_len += bytes_transferend;
        ::memset(m_data, 0, MAX_LENGTH);
        m_sock.async_read_some(boost::asio::buffer(m_data, MAX_LENGTH),
                               [this, self_ptr](auto &&PH1, auto &&PH2) {
                                 handle_read(std::forward<decltype(PH1)>(PH1),
                                             std::forward<decltype(PH2)>(PH2),
                                             self_ptr);
                               });
        m_b_head_parse = true;
        return;
      }
      memcpy(m_recv_msg_node->m_data + m_recv_msg_node->m_cur_len,
             m_data + copy_len, data_len);
      m_recv_msg_node->m_cur_len += data_len;
      copy_len += data_len;
      bytes_transferend -= data_len;
      m_recv_msg_node->m_data[m_recv_msg_node->m_total_len] = '\0';
// STAR:服务器宏THREAD_SEND_S
#ifdef THREAD_SEND_S
      std::cout << "receive data is " << m_recv_msg_node->m_data << std::endl;
      // 此处可以调用Send发送测试
      send(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len);
#endif // THREAD_SEND_S
// STAR:服务器宏PROTO_SEND_S
#ifdef PROTO_SEND_S
      MsgData msg_data;
      std::string receive_data;
      msg_data.ParseFromString(
          std::string(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len));
      std::cout << "receive data id is " << msg_data.id() << " msg data is "
                << msg_data.data() << std::endl;
      std::string return_str =
          "server has received msg, msg data is " + msg_data.data();
      MsgData msg_return;
      msg_return.set_id(msg_data.id());
      msg_return.set_data(return_str);
      msg_return.SerializeToString(&return_str);
      send(return_str);
#endif // PROTO_SEND_S
// STAR:服务器宏JSON_SEND_S
#ifdef JSON_SEND_S
      Json::Reader reader;
      Json::Value root;
      reader.parse(
          std::string(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len),
          root);
      std::cout << "receive msg id is " << root["id"].asInt() << " msg data is "
                << root["data"].asString() << std::endl;
      root["data"] =
          "server has received msg, msg data is " + root["data"].asString();
      std::string return_str = root.toStyledString();
      send(return_str);
#endif // JSON_SEND_S
       // 继续轮询剩余未处理数据
      m_b_head_parse = false;
      m_recv_head_node->clear();
      if (bytes_transferend <= 0) {
        ::memset(self_ptr->m_data, 0, MAX_LENGTH);
        m_sock.async_read_some(boost::asio::buffer(m_data, MAX_LENGTH),
                               [this, self_ptr](auto &&PH1, auto &&PH2) {
                                 handle_read(std::forward<decltype(PH1)>(PH1),
                                             std::forward<decltype(PH2)>(PH2),
                                             self_ptr);
                               });
        return;
      }
      continue;
    }
    // 已经处理完头部，处理上次未接受完的消息数据
    // 接收的数据仍不足剩余未处理的
    int remain_msg = m_recv_msg_node->m_total_len - m_recv_msg_node->m_cur_len;
    if (bytes_transferend < remain_msg) {
      memcpy(m_recv_msg_node->m_data + m_recv_msg_node->m_cur_len,
             m_data + copy_len, bytes_transferend);
      m_recv_msg_node->m_cur_len += bytes_transferend;
      ::memset(m_data, 0, MAX_LENGTH);
      m_sock.async_read_some(boost::asio::buffer(m_data, MAX_LENGTH),
                             [this, self_ptr](auto &&PH1, auto &&PH2) {
                               handle_read(std::forward<decltype(PH1)>(PH1),
                                           std::forward<decltype(PH2)>(PH2),
                                           self_ptr);
                             });
      return;
    }
    memcpy(m_recv_msg_node->m_data + m_recv_msg_node->m_cur_len,
           m_data + copy_len, remain_msg);
    m_recv_msg_node->m_cur_len += remain_msg;
    bytes_transferend -= remain_msg;
    copy_len += remain_msg;
    m_recv_msg_node->m_data[m_recv_msg_node->m_total_len] = '\0';
// STAR:服务器宏THREAD_SEND_S
#ifdef THREAD_SEND_S
    std::cout << "receive data is " << m_recv_msg_node->m_data << std::endl;
    send(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len);
#endif // THREAD_SEND_S
// STAR:服务器宏PROTO_SEND_S
#ifdef PROTO_SEND_S
    MsgData msg_data;
    std::string receive_data;
    msg_data.ParseFromString(
        std::string(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len));
    std::cout << "receive data id is " << msg_data.id() << "msg data is "
              << msg_data.data() << std::endl;
    std::string return_str =
        "server has received msg, msg data is " + msg_data.data();
    MsgData msg_return;
    msg_return.set_id(msg_data.id());
    msg_return.set_data(return_str);
    msg_return.SerializeToString(&return_str);
    // 此处调用send测试
    send(return_str);
#endif // PROTO_SEND_S
// STAR:服务器宏JSON_SEND_S
#ifdef JSON_SEND_S
    Json::Reader reader;
    Json::Value root;
    reader.parse(
        std::string(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len),
        root);
    std::cout << "receive msg is " << root["id"].asInt() << " msg data is "
              << root["data"].asString() << std::endl;
    root["data"] =
        "server has received msg, msg data is " + root["data"].asString();
    std::string return_str = root.toStyledString();
    send(return_str);
#endif // JSON_SEND_S
    // 继续轮询未处理的数据
    m_b_head_parse = false;
    m_recv_head_node->clear();
    if (bytes_transferend <= 0) {
      ::memset(m_data, 0, MAX_LENGTH);
      m_sock.async_read_some(boost::asio::buffer(m_data, MAX_LENGTH),
                             [this, self_ptr](auto &&PH1, auto &&PH2) {
                               handle_read(std::forward<decltype(PH1)>(PH1),
                                           std::forward<decltype(PH2)>(PH2),
                                           self_ptr);
                             });
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
  uint16_t data_len = 0;
  memcpy(&data_len, m_recv_head_node->m_data, HEAD_LENGTH);
  // data_len =
  // boost::asio::detail::socket_ops::network_to_host_short(data_len);
  std::cout << "data len is " << data_len << std::endl;
  // 数据太多，不合规
  if (data_len > MAX_LENGTH) {
    std::cout << "invalid data length is " << data_len << std::endl;
    m_server->delSession(self_ptr->m_uid);
    return;
  }

  m_recv_msg_node = std::make_shared<MsgNode>(data_len);
  boost::asio::async_read(m_sock,
                          boost::asio::buffer(m_recv_msg_node->m_data,
                                              m_recv_msg_node->m_total_len),
                          [this](auto &&PH1, auto &&PH2) {
                            handle_read_msg(std::forward<decltype(PH1)>(PH1),
                                            std::forward<decltype(PH2)>(PH2),
                                            shared_from_this());
                          });
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
  m_recv_msg_node->m_data[m_recv_msg_node->m_total_len] = '\0';
  std::cout << "receive data is " << m_recv_msg_node->m_data << std::endl;
  send(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len);
  // 再次接收头部数据
  m_recv_head_node->clear();
  boost::asio::async_read(
      m_sock, boost::asio::buffer(m_recv_head_node->m_data, HEAD_LENGTH),
      [this](auto &&PH1, auto &&PH2) {
        handle_read_head(std::forward<decltype(PH1)>(PH1),
                         std::forward<decltype(PH2)>(PH2), shared_from_this());
      });
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
      boost::asio::async_write(
          m_sock, boost::asio::buffer(msgnode->m_data, msgnode->m_total_len),
          [this, msgnode](auto &&PH1, auto &&) {
            handle_write(std::forward<decltype(PH1)>(PH1), shared_from_this());
          });
    }
  }
}
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
} // namespace ICEY
