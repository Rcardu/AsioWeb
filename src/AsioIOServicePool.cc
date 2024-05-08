/*
 * @Author: Ricardo
 * @Date: 2024-05-08 12:11:46
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-08 12:35:00
 */
#include "AsioIOServicePool.h"

#include <memory>

namespace ICEY {

AsioIOServicePool::AsioIOServicePool(std::size_t size)
    : m_ioServices(size), m_works(size), m_nextIOService(0) {
  for (std::size_t i = 0; i < size; ++i) {
    m_works[i] = std::make_unique<Work>(m_ioServices[i]);
  }
  // 遍历多个ioservice，创建多个线程，每个线程内部启动ioservice
  /**
   * @code
   * for(std::size_t i = 0 ;i < m_ioServices.size(); ++i){
   *   m_threads.emplace_back([this, i]{m_ioServices[i].run();});
   * }
   */
  for (auto &m_ioService : m_ioServices) {
    m_threads.emplace_back([&m_ioService] { m_ioService.run(); });
  }
}

AsioIOServicePool::~AsioIOServicePool() {
  std::cout << "AsioIOServicePool destruct " << std::endl;
}
boost::asio::io_context &AsioIOServicePool::GetIOService() {
  auto &service = m_ioServices[m_nextIOService++];
  if (m_nextIOService == m_ioServices.size()) {
    m_nextIOService = 0;
  }
  return service;
}
void AsioIOServicePool::Stop() {
  for (auto &work : m_works) {
    work.reset();
  }

  for (auto &thd : m_threads) {
    thd.join();
  }
}

} // namespace ICEY
