#pragma once

#ifdef DEBUG
#include <assert.h>
#endif // DEBUG

namespace starry {

template<typename To, typename From>
inline To implicit_cast(From const& f) {
  return f;
}

template<typename To, typename From>
inline To down_case(From* f) {
  if (false) {
    implicit_cast<From*, To>(0);
  }

#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
  assert(f == nullptr || dynamic_cast<To>(f) != nullptr);  // RTTI: debug mode only!
#endif
  return static_cast<To>(f);
}

};
