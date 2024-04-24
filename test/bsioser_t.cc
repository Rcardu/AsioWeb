/*
 * @Author: Ricardo
 * @Date: 2024-04-23 17:58:48
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-04-23 19:28:09
 */

#include "Server.h"
#include <iostream>

int main() {
  try {
    boost::asio::io_context io_context;
    ICEY::Server s(io_context, 10086);
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}
