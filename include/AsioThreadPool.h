/*
 * @Author: Ricardo
 * @Date: 2024-05-08 14:17:34
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-08 16:34:46
 */
#pragma once
#include "AsioIOServicePool.h"
#include "Singleton.h"
#include "boost/asio.hpp"
#include "boost/asio/io_context.hpp"

namespace ICEY {
class AsioThreadPool : public Singleton<AsioThreadPool> {
  friend class Singleton<AsioThreadPool>;

public:
  /**
   * @brief 析构函数
   */
  ~AsioThreadPool() {}
  AsioThreadPool(const AsioThreadPool &) = delete;
  AsioThreadPool &operator=(const AsioThreadPool &) = delete;
  /**
   * @brief 获取当前被注册的io_context（唯一一个）
   */
  boost::asio::io_context &GetIOService();
  /**
   * @brief 关闭线程池
   */
  void Stop();

private:
  /**
   * @brief 构造函数
   * @param[in] threadNum 要启用的线程数，基于系统本身的内核数
   */
  AsioThreadPool(int threadNum = std::thread::hardware_concurrency());

  // io_context服务
  boost::asio::io_context m_service;
  // 用来绑定io_context服务
  std::unique_ptr<boost::asio::io_context::work> m_work;
  // 线程池
  std::vector<std::thread> m_threads;
};

} // namespace ICEY
