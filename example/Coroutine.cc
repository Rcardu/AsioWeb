/*
 * @Author: Ricardo
 * @Date: 2024-05-08 17:52:10
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-08 19:15:24
 */
#include "boost/asio/this_coro.hpp"
#include "boost/asio/use_awaitable.hpp"
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <winrt/Windows.Foundation.h>

#include <iostream>

using boost::asio::ip::tcp;
namespace this_coro = boost::asio::this_coro;

boost::asio::awaitable<void> echo(tcp::socket socket) {
  try {
    char data[1024];
    for (;;) {
      std::size_t ret = co_await socket.async_read_some(
          boost::asio::buffer(data), boost::asio::use_awaitable);
      co_await boost::asio::async_write(socket, boost::asio::buffer(data, ret),
                                        boost::asio::use_awaitable);
    }
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

boost::asio::awaitable<void> listener() {
  auto executor = co_await this_coro::executor;
  tcp::acceptor acceptor(executor, {tcp::v4(), 10086});
  for (;;) {
    tcp::socket socket =
        co_await acceptor.async_accept(boost::asio::use_awaitable);
    boost::asio::co_spawn(executor, echo(std::move(socket)),
                          boost::asio::detached);
  }
}
int main() {
  try {
    boost::asio::io_context io_context(1);
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { io_context.stop(); });

    co_spawn(io_context, listener(), boost::asio::detached);
    io_context.run();

  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
