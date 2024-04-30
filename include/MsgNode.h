/*
 * @Author: Ricardo
 * @Date: 2024-04-30 15:24:53
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-04-30 16:23:44
 */
#pragma once
#include <boost/asio.hpp>
#include <cstdint>
#include <iostream>
#include <string>

namespace ICEY {

class MsgNode {
public:
  MsgNode(uint16_t max_len)
      : m_total_len(max_len), m_data(new char[max_len + 1]) {
    m_data[m_total_len] = '\0';
  }
  ~MsgNode() {
    std::cout << "destruct MsgNode" << std::endl;
    delete[] m_data;
  }
  void clear() {
    ::memset(m_data, 0, m_total_len);
    m_cur_len = 0;
  }
  [[nodiscard]] char *getData() const { return m_data; }
  [[nodiscard]] uint16_t getCurlen() const { return m_cur_len; }
  [[nodiscard]] uint16_t getTotallen() const { return m_total_len; }
  void addCurlen(int len) { m_cur_len += len; }

  char &operator[](size_t idx) { return m_data[idx]; }
  const char &operator[](size_t idx) const { return m_data[idx]; }

protected:
  uint16_t m_cur_len{0};
  uint16_t m_total_len;
  char *m_data;
};

class RecvNode : public MsgNode {
public:
  RecvNode(int16_t max_len, int16_t msg_id);

private:
  uint16_t m_msg_id;
};
class SendNode : public MsgNode {
public:
  SendNode(const char *msg, int16_t max_len, int16_t msg_id);

private:
  uint16_t m_msg_id;
};
} // namespace ICEY
