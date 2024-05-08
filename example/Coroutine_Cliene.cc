/*
 * @Author: Ricardo
 * @Date: 2024-05-08 19:16:53
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-08 19:27:33
 */
#include "boost/asio/error.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/system/detail/error_code.hpp"
#include <boost/asio.hpp>
#include <iostream>

using namespace boost::asio::ip;

int main() {

  try {
    boost::asio::io_context io_context;
    tcp::endpoint remote_ep(address::from_string("127.0.0.1"), 10086);
    tcp::socket sock(io_context);
    boost::system::error_code error = boost::asio::error::host_not_found;
    sock.connect(remote_ep, error);
    if (error) {
      std::cout << "connect failed, code is " << error.value()
                << " error msg is " << error.what() << std::endl;
      return 0;
    }

    std::cout << "Enter messag: ";
    char request[1024];
    std::cin.getline(request, 1024);
    size_t request_length = strlen(request);
    boost::asio::write(sock, boost::asio::buffer(request, request_length));

    char reply[1024];
    size_t reply_length =
        boost::asio::read(sock, boost::asio::buffer(reply, request_length));

    std::cout << "reply is " << std::string(reply, reply_length) << std::endl;
    getchar();

  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
