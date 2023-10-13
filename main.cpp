#include "file_type.hpp"
#include <iostream>
void usage() {
  std::cout << "usage:\n";
  std::cout << " ft.exe file path\n";
  std::cout << "example:\n";
  std::cout << " ft.exe c:\\test.txt\n";
}

int main(int argc, char *argv[]) {
  try {
    if (argc != 2) {
      usage();
      return 0;
    }
    std::cout << file_type(argv[1]);
  } catch (std::exception &e) {
    std::cout << e.what();
    return -1;
  }
  return 0;
}