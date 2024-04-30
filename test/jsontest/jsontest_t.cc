/*
 * @Author: Ricardo
 * @Date: 2024-04-29 14:47:05
 * @Last Modified by: ICEY
 * @Last Modified time: 2024-04-29 14:49:42
 */
#include "json/json.h"
#include "json/reader.h"
#include "json/value.h"
#include <iostream>

int main() {

  Json::Value root;
  root["id"] = 1001;
  root["data"] = "hello";
  std::string request = root.toStyledString();
  std::cout << "request:" << request << std::endl;

  Json::Value root2;
  Json::Reader reader;
  reader.parse(request, root2);
  std::cout << "msg id is " << root2["id"] << " msg is "
            << root2["data"].asString() << std::endl;

  return 0;
}
