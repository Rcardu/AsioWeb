#include "msg.pb.h"

#include <iostream>
int main() {

  Book book;
  book.set_name("CPP programing");
  book.set_pages(100);
  book.set_price(200.0);

  std::string bookstr;
  book.SerializeToString(&bookstr);
  std::cout << "bookstr:" << bookstr << std::endl;

  Book book2;
  book2.ParseFromString(bookstr);
  std::cout << "book2.name:" << book2.name() << std::endl;
  std::cout << "book2.pages:" << book2.pages() << std::endl;
  std::cout << "book2.price:" << book2.price() << std::endl;

  return 0;
}
