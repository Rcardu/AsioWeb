/*
 * @Author: Ricardo
 * @Date: 2024-05-07 15:15:16
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-07 15:54:29
 */

#include "LogicSystem.h"
#include "deconst.h"

namespace ICEY {
LogicSystem::LogicSystem() : m_b_stop(false) {
  registerCallBacks();
  m_worker_thread = std::thread(&LogicSystem::dealMsg, this);
}
LogicSystem::~LogicSystem() {
  m_b_stop = true;
  m_consume.notify_one();
  m_worker_thread.join();
}
void LogicSystem::registerCallBacks() {
  m_fun_callback[MSG_HELLO_WORD] = [this](auto &&PH1, auto &&PH2, auto &&PH3) {
    helloWorldCallBack(std::forward<decltype(PH1)>(PH1),
                       std::forward<decltype(PH2)>(PH2),
                       std::forward<decltype(PH3)>(PH3));
  };
}
void LogicSystem::helloWorldCallBack(std::shared_ptr<Session> session,
                                     const int16_t &msg_id,
                                     const std::string &msg_data) {
  Json::Reader reader;
  Json::Value root;
  reader.parse(msg_data, root);
  std::cout << "receive msg id is " << root["id"].asInt() << "msg data is "
            << root["data"].asString() << std::endl;
  root["data"] =
      "server has receive msg, msg data is " + root["data"].asString();

  std::string return_str = root.toStyledString();
  session->send(return_str, root["id"].asInt());
}
void LogicSystem::dealMsg() {
  for (;;) {
    std::unique_lock<std::mutex> unique_lk(m_mutex);

    // 判断队列为空则用条件变量等待
    while (m_msg_que.empty() && !m_b_stop) {
      m_consume.wait(unique_lk);
    }

    // 判断如果关闭状态，取出逻辑队列所有数据即时处理并退出循环
    if (m_b_stop) {
      while (!m_msg_que.empty()) {
        auto msg_node = m_msg_que.front();
        std::cout << "recv msg id is " << msg_node->m_recvnode->m_msg_id
                  << std::endl;
        auto call_back_iter =
            m_fun_callback.find(msg_node->m_recvnode->m_msg_id);
        if (call_back_iter == m_fun_callback.end()) {
          m_msg_que.pop();
          continue;
        }
        call_back_iter->second(msg_node->m_session,
                               msg_node->m_recvnode->m_msg_id,
                               std::string(msg_node->m_recvnode->getData(),
                                           msg_node->m_recvnode->getCurlen()));
        m_msg_que.pop();
      }
      break;
    }

    // 如果没有停服，并且队列中有数据
    auto msg_node = m_msg_que.front();
    std::cout << "recv msg id is " << msg_node->m_recvnode->m_msg_id
              << std::endl;
    auto call_back_iter = m_fun_callback.find(msg_node->m_recvnode->m_msg_id);
    if (call_back_iter == m_fun_callback.end()) {
      m_msg_que.pop();
      continue;
    }
    call_back_iter->second(msg_node->m_session, msg_node->m_recvnode->m_msg_id,
                           std::string(msg_node->m_recvnode->getData(),
                                       msg_node->m_recvnode->getCurlen()));
    m_msg_que.pop();
  }
}
void LogicSystem::postMsgToQueue(std::shared_ptr<LogicNode> msg) {
  std::unique_lock<std::mutex> unique_lk(m_mutex);
  m_msg_que.push(msg);

  if (m_msg_que.size() == 1) {
    m_consume.notify_one();
  }
}

} // namespace ICEY
