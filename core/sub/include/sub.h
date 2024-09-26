#pragma once
#include "noncopyable.h"

class SUB : noncopyable {
public:
  SUB(int a, int b) : a_(a), b_(b) {}
  int cal();

private:
  int a_;
  int b_;
};
