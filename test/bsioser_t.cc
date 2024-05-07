/*
 * @Author: Ricardo
 * @Date: 2024-04-23 17:58:48
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-04-23 19:28:09
 */

#include "Server.h"
#include "boost/asio/io_context.hpp"
#include <csignal>
#include <iostream>
#include <mutex>
#include <thread>

bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

void sign_hander(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    std::unique_lock<std::mutex> lock_quit(mutex_quit);
    bstop = true;
    cond_quit.notify_one();
  }
}

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
#endif
    boost::asio::io_context io_context;
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&io_context](auto, auto) { io_context.stop(); });
    ICEY::Server s(io_context, 10086);
    io_context.run();

  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
