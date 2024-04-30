/*
 * @Author: Ricardo
 * @Date: 2024-04-25 11:50:54
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-04-25 12:30:43
 */
#include "boost/asio/detail/socket_holder.hpp"
#include <boost/asio.hpp>
#include <cstdint>
#include <iostream>

using namespace std;

// 判断当前系统的字节序是大端序还是小端序
bool is_big_endian() {
  int num = 1;
  if (*(char *)&num == 1) {
    // 当前系统为小端序
    return false;
  } else {
    // 当前系统为大端序
    return true;
  }
}

int main() {
  int num = 0x12345678;
  char *p = (char *)&num;
  std::cout << "real data: " << hex << num << endl;
  if (is_big_endian()) {
    std::cout << "current bytearray" << endl;
    std::cout << "first byte: " << hex << (int)p[0] << endl;
    std::cout << "second byte: " << hex << (int)p[1] << endl;
    std::cout << "third byte: " << hex << (int)p[2] << endl;
    std::cout << "fourth byte: " << hex << (int)p[3] << endl;
  } else {
    std::cout << "little bytearray" << endl;
    std::cout << "first byte: " << hex << (int)p[3] << endl;
    std::cout << "second byte: " << hex << (int)p[2] << endl;
    std::cout << "third byte: " << hex << (int)p[1] << endl;
    std::cout << "fourth byte: " << hex << (int)p[0] << endl;
  }
  std::cout << std::endl;

  uint32_t host_long_value = 0x12345678;
  uint16_t host_short_value = 0x1234;
  uint32_t network_long_value =
      boost::asio::detail::socket_ops::host_to_network_long(host_long_value);
  uint16_t network_short_value =
      boost::asio::detail::socket_ops::host_to_network_short(host_short_value);

  std::cout << "Host long value: " << std::hex << host_long_value << std::endl;
  std::cout << "Network long value: " << std::hex << network_long_value
            << std::endl;
  std::cout << "Host short value: " << std::hex << host_short_value
            << std::endl;
  std::cout << "Network short value: " << std::hex << network_short_value
            << std::endl;
  std::cout << std::endl;

  return 0;
}
