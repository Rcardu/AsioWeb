/*
 * @Author: Ricardo
 * @Date: 2024-04-24 12:46:35
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-05-08 17:22:36
 */

#include "Session.h"
#include "boost/asio/detail/socket_holder.hpp"
#include "msg.pb.h"
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <chrono>
#include <cstring>
#include <exception>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <thread>

using namespace std;
using namespace boost::asio::ip;

std::vector<std::thread> vec_threads;
int main() {

  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 100; i++) {
    vec_threads.emplace_back([]() {
      try {
        boost::asio::io_context ioc;
        tcp::endpoint ep(address::from_string("127.0.0.1"), 10086);
        tcp::socket sock(ioc);
        boost::system::error_code error = boost::asio::error::host_not_found;
        sock.connect(ep, error);
        if (error) {
          cout << "connect failed, code is " << error.value()
               << " error msg is " << error.message();
          return 0;
        }
        int i = 0;
        while (i < 500) {
          Json::Value root;
          root["id"] = 1001;
          root["data"] = "hello world";
          std::string request = root.toStyledString();
          size_t request_length = request.length();
          char send_data[MAX_LENGTH] = {0};
          int16_t msgid = 1001;
          int msgid_host =
              boost::asio::detail::socket_ops::host_to_network_short(msgid);
          memcpy(send_data, &msgid_host, HEAD_ID_LEN);
          ;
          // 转为网络字节序
          int request_host_length =
              boost::asio::detail::socket_ops::host_to_network_short(
                  request_length);
          memcpy(send_data + HEAD_ID_LEN, &request_host_length, HEAD_DATA_LEN);
          memcpy(send_data + HEAD_TOTAL_LEN, request.c_str(), request_length);
          boost::asio::write(
              sock,
              boost::asio::buffer(send_data, request_length + HEAD_TOTAL_LEN));
          std::cout << "begin to receive..." << std::endl;
          char reply_head[HEAD_TOTAL_LEN];
          size_t reply_length = boost::asio::read(
              sock, boost::asio::buffer(reply_head, HEAD_TOTAL_LEN));
          msgid = 0;
          memcpy(&msgid, reply_head, HEAD_ID_LEN);
          int16_t msglen = 0;
          memcpy(&msglen, reply_head + HEAD_ID_LEN, HEAD_DATA_LEN);
          // 转为本地字节序
          msgid = boost::asio::detail::socket_ops::network_to_host_short(msgid);
          msglen =
              boost::asio::detail::socket_ops::network_to_host_short(msglen);
          char msg[MAX_LENGTH] = {0};
          size_t msg_length =
              boost::asio::read(sock, boost::asio::buffer(msg, msglen));
          Json::Reader reader;
          reader.parse(std::string(msg, msg_length), root);
          std::cout << "msg id is " << root["id"] << " msg is " << root["data"]
                    << std::endl;
          // std::this_thread::sleep_for(std::chrono::seconds(1));
          // getchar();
          i++;
        }

      } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
      }
      return 0;
    });
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }
  for (auto &thd : vec_threads) {
    thd.join();
  }
  auto end = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);

  std::cout << "Time spent " << duration.count() << "seconds\n";
  getchar();

  try {
// STAR:客户端宏JSON_SEND_S
#ifdef JSON_SEND_S
    Json::Value root;
    root["id"] = 1001;
    root["data"] = "hello world";
    std::string request = root.toStyledString();
    size_t request_length = request.length();
    char send_data[MAX_LENGTH] = {0};
    int16_t msgid = 1001;
    int msgid_host =
        boost::asio::detail::socket_ops::host_to_network_short(msgid);
    memcpy(send_data, &msgid_host, HEAD_ID_LEN);
    ;
    // 转为网络字节序
    int request_host_length =
        boost::asio::detail::socket_ops::host_to_network_short(request_length);
    memcpy(send_data + HEAD_ID_LEN, &request_host_length, HEAD_DATA_LEN);
    memcpy(send_data + HEAD_TOTAL_LEN, request.c_str(), request_length);
    boost::asio::write(
        sock, boost::asio::buffer(send_data, request_length + HEAD_TOTAL_LEN));
    std::cout << "begin to receive..." << std::endl;
    char reply_head[HEAD_TOTAL_LEN];
    size_t reply_length = boost::asio::read(
        sock, boost::asio::buffer(reply_head, HEAD_TOTAL_LEN));
    msgid = 0;
    memcpy(&msgid, reply_head, HEAD_ID_LEN);
    int16_t msglen = 0;
    memcpy(&msglen, reply_head + HEAD_ID_LEN, HEAD_DATA_LEN);
    // 转为本地字节序
    msgid = boost::asio::detail::socket_ops::network_to_host_short(msgid);
    msglen = boost::asio::detail::socket_ops::network_to_host_short(msglen);
    char msg[MAX_LENGTH] = {0};
    size_t msg_length =
        boost::asio::read(sock, boost::asio::buffer(msg, msglen));
    Json::Reader reader;
    reader.parse(std::string(msg, msg_length), root);
    std::cout << "msg id is " << root["id"] << " msg is " << root["data"]
              << std::endl;
    getchar();
#endif // JSON_SEND_S
// STAR:客户端宏PTOTO_SEND_S
#ifdef PROTO_SEND_S
    MsgData msgdata;
    msgdata.set_id(1001);
    msgdata.set_data("hello world");
    std::string request;
    msgdata.SerializeToString(&request);
    short request_length = request.length();
    char send_data[MAX_LENGTH] = {0};
    short request_host_length =
        boost::asio::detail::socket_ops::host_to_network_short(request_length);
    memcpy(send_data, &request_host_length, 2);
    memcpy(send_data + 2, request.c_str(), request_length);
    boost::asio::write(sock,
                       boost::asio::buffer(send_data, request_length + 2));
    std::cout << "begin to receive..." << std::endl;
    char reply_head[HEAD_LENGTH];
    size_t reply_length =
        boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_LENGTH));
    short msglen = 0;
    memcpy(&msglen, reply_head, HEAD_LENGTH);
    // 转为本地字节序
    msglen = boost::asio::detail::socket_ops::network_to_host_short(msglen);
    char msg[MAX_LENGTH] = {0};
    size_t msg_length =
        boost::asio::read(sock, boost::asio::buffer(msg, msglen));
    MsgData recvdata;
    recvdata.ParseFromArray(msg, msglen);
    std::cout << "msg id is " << recvdata.id() << " msg is " << recvdata.data()
              << std::endl;
    getchar();
#endif // PROTO_SEND_S
// STAR:客户端宏THREAD_SEND_S
#ifdef THREAD_SEND_S
    thread send_thread([&sock] {
      for (;;) {
        this_thread::sleep_for(std::chrono::milliseconds(2));
        const char *request = "hello word!";
        size_t request_length = strlen(request);
        // uint16_t request_host_length =
        //     boost::asio::detail::socket_ops::host_to_network_short(
        //         request_length);
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
        char reply_head[HEAD_LENGTH];
        size_t reply_length = boost::asio::read(
            sock, boost::asio::buffer(reply_head, HEAD_LENGTH));
        uint16_t msglen = 0;
        memcpy(&msglen, reply_head, HEAD_LENGTH);
        // msglen =
        // boost::asio::detail::socket_ops::network_to_host_short(msglen);
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
#endif // THREAD_SEND_S
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << endl;
  }
  return 0;
}
