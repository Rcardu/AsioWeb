/*
 * @Author: Ricardo
 * @Date: 2024-05-08 14:26:09
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-08 14:34:01
 */
#include "AsioThreadPool.h"
#include "boost/asio/io_context.hpp"

namespace ICEY {
AsioThreadPool::AsioThreadPool(int threadNum)
    : m_work(new boost::asio::io_context::work(m_service)) {
  for (int i = 0; i < threadNum; ++i) {
    m_threads.emplace_back([this]() { m_service.run(); });
  }
}
boost::asio::io_context &AsioThreadPool::GetIOService() { return m_service; }
void AsioThreadPool::Stop() {
  m_work.reset();
  for (auto &thd : m_threads) {
    thd.join();
  }
}

} // namespace ICEY
