/*
 * @Author: Ricardo
 * @Date: 2024-04-30 15:24:53
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-07 16:05:18
 * @Title: 消息节点类
 */
#pragma once
#include "LogicSystem.h"
#include "Session.h"
#include <boost/asio.hpp>
#include <cstdint>
#include <iostream>
#include <string>

namespace ICEY {

class LogicSystem;
class MsgNode {
public:
  /**
   * @brief 构造函数
   * @param[in] max_len 消息长度
   */
  MsgNode(uint16_t max_len)
      : m_total_len(max_len), m_data(new char[max_len + 1]) {
    m_data[m_total_len] = '\0';
  }
  /**
   * @brief 析构函数
   */
  ~MsgNode() {
    std::cout << "destruct MsgNode" << std::endl;
    delete[] m_data;
  }
  /**
   * @brief 清空当前节点内的数据
   */
  void clear() {
    ::memset(m_data, 0, m_total_len);
    m_cur_len = 0;
  }
  /**
   * @brief 获取当前节点的数据指针
   * @return 返回此节点的数据指针
   */
  [[nodiscard]] char *getData() const { return m_data; }
  /**
   * @brief 过去当前节点数据的大小
   * @return 返回当前节点数据的大小
   */
  [[nodiscard]] uint16_t getCurlen() const { return m_cur_len; }
  /**
   * @brief 获取当前节点数据的总大小
   * @return 返回当前数据的总大小
   */
  [[nodiscard]] uint16_t getTotallen() const { return m_total_len; }
  /**
   * @brief 当前数据的长度增加
   * @param[in] len 要增加的长度
   */
  void addCurlen(int len) { m_cur_len += len; }

  /**
   * @brief 重载[]运算符
   * @param[in] idx 需要返回的索引
   * @return 返回以idx为索引的数据的字符
   */
  char &operator[](size_t idx) { return m_data[idx]; }
  /**
   * @brief 重载[]运算符 const限定
   * @param[in] idx 需要返回的索引
   * @return 返回以idx为索引的数据的字符
   */
  const char &operator[](size_t idx) const { return m_data[idx]; }

protected:
  // 当前数据长度
  uint16_t m_cur_len{0};
  // 总共可以保存的长度
  uint16_t m_total_len;
  // 数据节点指针
  char *m_data;
};

/// 接收节点
class RecvNode : public MsgNode {
  friend class LogicSystem;

public:
  /**
   * @brief 构造函数
   * @param[in] max_len 数据总长度
   * @param[in] smg_id 数据id
   */
  RecvNode(int16_t max_len, int16_t msg_id);

private:
  // 消息id
  uint16_t m_msg_id;
};

/// 发送节点
class SendNode : public MsgNode {
  friend class LogicSystem;

public:
  /**
   * @brief 构造函数
   * @param[in] msg 要保存的数据
   * @param[in] max_len 数据总长度
   * @param[in] smg_id 数据id
   */
  SendNode(const char *msg, int16_t max_len, int16_t msg_id);

private:
  // 消息id
  uint16_t m_msg_id;
};
} // namespace ICEY
