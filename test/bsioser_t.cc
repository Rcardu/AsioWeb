/*
 * @Author: Ricardo
 * @Date: 2024-04-23 17:58:48
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-08 17:14:45
 */

#include "AsioIOServicePool.h"
#include "AsioThreadPool.h"
#include "Server.h"
#include "boost/asio/io_context.hpp"
#include <csignal>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

// STAR: 服务器启动宏C_SIGNAL_FLAG（服务器的退出方式）
#ifdef C_SIGNAL_FLAG
void sign_hander(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    std::unique_lock<std::mutex> lock_quit(mutex_quit);
    bstop = true;
    cond_quit.notify_one();
  }
}
#endif // C_SINGAL_FLAG

int main() {
  try {
// STAR: 服务器启动宏C_SIGNAL_FLAG（服务器的退出方式）
#ifdef C_SIGNAL_FLAG
    boost::asio::io_context io_context;
    std::thread net_work_thread([&io_context] {
      ICEY::Server s(io_context, 10086);
      io_context.run();
    });

    auto ret1 = signal(SIGINT, sign_hander);
    auto ret2 = signal(SIGTERM, sign_hander);
    while (!bstop) {
      std::unique_lock<std::mutex> lock_quit(mutex_quit);
      cond_quit.wait(lock_quit);
    }

    io_context.stop();
    net_work_thread.join();
#endif // C_SIGNAL_FLAG
// STAR:服务池宏C_SERVER_POOL
#ifdef C_SERVER_POOL
    auto pool = ICEY::AsioIOServicePool::GetInstance();
    boost::asio::io_context io_context;
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&io_context, pool](auto, auto) {
      io_context.stop();
      pool->Stop();
    });
    ICEY::Server s(io_context, 10086);
    io_context.run();
#endif // C_SERVER_POOL
// STAR:线程池C_THREAD_POOL
#ifdef C_THREAD_POOL
    auto pool = ICEY::AsioThreadPool::GetInstance();
    boost::asio::io_context io_context;
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&io_context, pool](auto, auto) {
      io_context.stop();
      pool->Stop();
      std::unique_lock<std::mutex> lock(mutex_quit);
      bstop = true;
      cond_quit.notify_one();
    });
    ICEY::Server s(pool->GetIOService(), 10086);
    {
      std::unique_lock<std::mutex> lock(mutex_quit);
      while (!bstop) {
        cond_quit.wait(lock);
      }
    }

#endif // C_THREAD_POOL 229seconds
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
