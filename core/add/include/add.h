#pragma once
#include "noncopyable.h"

class ADD : noncopyable {
public:
  ADD(int a, int b) : a_(a), b_(b){}
  int cal();

private:
  int a_;
  int b_;
};
