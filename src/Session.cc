/*
 * @Author: Ricardo
 * @Date: 2024-04-23 18:03:01
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-04-24 13:08:33
 */
#include "Session.h"
#include "boost/asio/write.hpp"
#include "boost/uuid/random_generator.hpp"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
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
  memset(m_data, 0, max_length);
  m_sock.async_read_some(
      Bsio::buffer(m_data, max_length), [this](auto &&PH1, auto &&PH2) {
        handle_read(std::forward<decltype(PH1)>(PH1),
                    std::forward<decltype(PH2)>(PH2), shared_from_this());
      });
}
void Session::send(char *msg, int max_length) {
  // true当前发送队列里有数据，即上一次数据没有发送完
  // false上一次的数据发送完毕，发送缓冲区为空
  bool pending = false;
  {
    std::lock_guard<std::mutex> lock(m_send_lock);
    if (m_send_que.size() > 0) {
      pending = true;
    }
    m_send_que.push(std::make_shared<MsgNode>(msg, max_length));
    if (pending) {
      return;
    }
    auto &msgnode = m_send_que.front();

    boost::asio::async_write(
        m_sock, boost::asio::buffer(msgnode->m_data, msgnode->m_total_len),
        std::bind(&Session::handle_write, this, std::placeholders::_1,
                  shared_from_this()));
  }
}
void Session::printRecvData(char *data, int length) {
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
  printRecvData(m_data, bytes_transferend);
  std::chrono::milliseconds dura(2000);
  std::this_thread::sleep_for(dura);
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
      int16_t data_len = 0;
      memcpy(&data_len, m_recv_head_node->m_data, HEAD_LENGTH);
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
      std::cout << "receive data is " << m_recv_msg_node->m_data << std::endl;
      // 此处可以调用Send发送测试
      send(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len);
      // 继续轮询剩余未处理数据
      m_b_head_parse = false;
      m_recv_head_node->clear();
      if (bytes_transferend <= 0) {
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
    std::cout << "receive data is " << m_recv_msg_node->m_data << std::endl;
    // 此处调用send测试
    send(m_recv_msg_node->m_data, m_recv_msg_node->m_total_len);
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
          std::bind(&Session::handle_write, this, std::placeholders::_1,
                    self_ptr));
    }
  }
}
MsgNode::MsgNode(char *msg, int16_t max_len)
    : m_total_len(max_len + HEAD_LENGTH), m_data(new char[m_total_len + 1]()),
      m_cur_len(0) {

  memcpy(m_data, &max_len, HEAD_LENGTH);
  memcpy(m_data + HEAD_LENGTH, msg, max_len);
  m_data[m_total_len] = '\0';
}
MsgNode::MsgNode(int16_t max_len)
    : m_total_len(max_len), m_data(new char[m_total_len + 1]()), m_cur_len(0) {}
MsgNode::~MsgNode() { delete[] m_data; }
} // namespace ICEY
