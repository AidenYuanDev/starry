#include <iostream>
#include "add.h"
#include "sub.h"

int main() {
  ADD a(5, 6);
  SUB b(5, 6);
  std::cout << a.cal() << std::endl;
  std::cout << b.cal() << std::endl;
  return 0;
}
