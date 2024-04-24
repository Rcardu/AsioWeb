/*
 * @Author: Ricardo
 * @Date: 2024-04-24 12:46:35
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-04-24 12:50:53
 */

#include "boost/asio/io_context.hpp"
#include <boost/asio.hpp>
#include <exception>
#include <iostream>

using namespace std;
using namespace boost::asio::ip;

const int MAX_LENGTH = 1024;

int main() {

  try {
    boost::asio::io_context ioc;
    tcp::endpoint ep(address::from_string("127.0.0.1"), 10086);
    tcp::socket sock(ioc);
    boost::system::error_code error = boost::asio::error::host_not_found;
    sock.connect(ep, error);
    if (error) {
      cout << "connect failed, code is " << error.value() << " error msg is "
           << error.message();
      return 0;
    }
    std::cout << "Enter message: ";
    char request[MAX_LENGTH];
    std::cin.getline(request, MAX_LENGTH);
    size_t request_length = strlen(request);
    boost::asio::write(sock, boost::asio::buffer(request, request_length));
    char reply[MAX_LENGTH];
    size_t reply_length =
        boost::asio::read(sock, boost::asio::buffer(reply, request_length));
    std::cout << "Reply is: ";
    std::cout.write(reply, reply_length);
    std::cout << "\n";
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << endl;
  }
  return 0;
}
