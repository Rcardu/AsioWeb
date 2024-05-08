/*
 * @Author: Ricardo
 * @Date: 2024-05-08 11:58:19
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-08 16:32:00
 */
#pragma once
#include "Singleton.h"
#include "boost/asio.hpp"
#include "boost/asio/io_context.hpp"
#include <vector>

namespace ICEY {
/// 服务池类，每个线程独立注册一个context来进行通信
class AsioIOServicePool : public Singleton<AsioIOServicePool> {
  friend Singleton<AsioIOServicePool>;

  using IOService = boost::asio::io_context;
  using Work = boost::asio::io_context::work;
  using WorkPtr = std::unique_ptr<Work>;

public:
  /**
   * @brief 析构函数
   */
  ~AsioIOServicePool();
  AsioIOServicePool(const AsioIOServicePool &) = delete;
  AsioIOServicePool &operator=(const AsioIOServicePool &) = delete;

  // 使用round-robin的方式返回一个io_context
  /**
   * @brief 获取一个io_context
   */
  boost::asio::io_context &GetIOService();
  /**
   * @brief 关闭服务池
   */
  void Stop();

private:
  /**
   * @brief 构造函数
   * @param[in] size 要创建的服务池的大小，默认取决于系统内核数
   */
  AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());

  // io_context容器，用来存储每个被注册的io_context
  std::vector<IOService> m_ioServices;
  // 基于boost::asio::io_context::work，用来使还没有被初始化的io_context不会退出
  std::vector<WorkPtr> m_works;
  // 线程容器，用来获取每个io_context，并进行通信
  std::vector<std::thread> m_threads;
  // 每个io_context的索引
  std::size_t m_nextIOService;
};

} // namespace ICEY
