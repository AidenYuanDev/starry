#include "buffer.h"
#include "sockets_ops.h"

#include <cstddef>
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>

using namespace starry;

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPreend;
const size_t Buffer::kInitialSize;

// 读取文件描述符 fd 的字符，如果读取超过 buffer 就使用备用缓冲区
ssize_t Buffer::readFd(int fd, int* savedErrno) {
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writeable = writableBytes();
  vec[0].iov_base = begin() + writerIndex_;
  vec[0].iov_len = writeable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len  =sizeof(extrabuf);
  const int iovcnt = (writeable < sizeof(extrabuf)) ? 2 : 1;
  const ssize_t n = sockets::readv(fd, vec, iovcnt);
  if (n < 0) {
    *savedErrno = errno;
  } else if (static_cast<size_t>(n) <= writeable) {
    writerIndex_ += n;
  } else {
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writeable);
  }
  
  return n;
}
