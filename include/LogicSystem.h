/*
 * @Author: Ricardo
 * @Date: 2024-05-07 14:50:42
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-07 16:09:24
 * @Title: 逻辑类
 */
#pragma once
#include "Session.h"
#include "Singleton.h"
#include "deconst.h"
#include <functional>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <map>
#include <queue>
#include <thread>

namespace ICEY {
class Session;
class LogicNode;

// 回调函数包装器
using FunCallBack = std::function<void(std::shared_ptr<Session>,
                                       const int16_t &, const std::string &)>;

class LogicSystem : public Singleton<LogicSystem> {
  friend class Singleton<LogicSystem>;

public:
  /**
   * @brief 析构函数
   */
  ~LogicSystem();
  /**
   * @brief 添加任务到队列
   * @param[in] msg 要添加的任务
   */
  void postMsgToQueue(std::shared_ptr<LogicNode> msg);

private:
  /**
   * @brief 构造函数
   */
  LogicSystem();
  /**
   * @brief 注册任务函数
   */
  void registerCallBacks();
  /**
   * @brief 回调函数，即被注册的任务函数
   * @param[in] session 会话
   * @param[in] msg_id 会话id
   * @param[in] msg_data 会话信息
   */
  void helloWorldCallBack(std::shared_ptr<Session> session,
                          const int16_t &msg_id, const std::string &msg_data);
  /**
   * @brief 进行消息发送
   */
  void dealMsg();

private:
  // 逻辑队列
  std::queue<std::shared_ptr<LogicNode>> m_msg_que;
  // 保证逻辑队列线程安全的互斥量
  std::mutex m_mutex;
  // 表示消费者条件变量，用来控制当逻辑队列为空时保证线程暂时挂起等待，不干扰其他线程
  std::condition_variable m_consume;
  // 表示工作线程，用来从逻辑队列中取数据并执行回调函数
  std::thread m_worker_thread;
  // 表示收到外部的停止信号，逻辑类要终止工作线程并退出
  bool m_b_stop;
  // 维护一个map用来查询每个会话id对应的回调函数
  std::map<uint16_t, FunCallBack> m_fun_callback;
};
} // namespace ICEY
