/*
 * @Author: Ricardo
 * @Date: 2024-04-24 12:46:35
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-04-24 12:50:53
 */

#include "Session.h"
#include "boost/asio/io_context.hpp"
#include <boost/asio.hpp>
#include <chrono>
#include <exception>
#include <iostream>
#include <thread>


using namespace std;
using namespace boost::asio::ip;

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
    thread send_thread([&sock] {
      for (;;) {
        this_thread::sleep_for(std::chrono::milliseconds(2));
        const char *request = "hello word!";
        size_t request_length = strlen(request);
        char send_data[MAX_LENGTH] = {0};
        memcpy(send_data, &request_length, 2);
        memcpy(send_data + 2, request, request_length);
        boost::asio::write(sock,
                           boost::asio::buffer(send_data, request_length + 2));
      }
    });
    thread recv_thread([&sock] {
      for (;;) {
        this_thread::sleep_for(std::chrono::milliseconds(2));
        std::cout << "begin to recive..." << std::endl;
        char reply_head[MAX_LENGTH];
        size_t reply_length = boost::asio::read(
            sock, boost::asio::buffer(reply_head, HEAD_LENGTH));
        int16_t msglen = 0;
        memcpy(&msglen, reply_head, HEAD_LENGTH);
        char msg[MAX_LENGTH] = {0};
        size_t mag_length =
            boost::asio::read(sock, boost::asio::buffer(msg, msglen));
        std::cout << "Reply is: ";
        std::cout.write(msg, msglen) << std::endl;
        std::cout << "Reply len is: " << msglen;
        std::cout << "\n";
      }
    });
    send_thread.join();
    recv_thread.join();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << endl;
  }
  return 0;
}
