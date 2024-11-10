// inet_address_test.cc
#include <gtest/gtest.h>
#include "inet_address.h"

using namespace starry;

TEST(InetAddressTest, Basics) {
  InetAddress addr0(1234);
  EXPECT_EQ(addr0.toIp(), "0.0.0.0");
  EXPECT_EQ(addr0.toIpPort(), "0.0.0.0:1234");
  EXPECT_EQ(addr0.port(), static_cast<uint16_t>(1234));

  InetAddress addr1(4321, true);  // localhost
  EXPECT_EQ(addr1.toIp(), "127.0.0.1");
  EXPECT_EQ(addr1.toIpPort(), "127.0.0.1:4321");
  EXPECT_EQ(addr1.port(), static_cast<uint16_t>(4321));
}

TEST(InetAddressTest, HostToIp) {
  InetAddress addr("127.0.0.1", 1234);
  EXPECT_EQ(addr.toIp(), "127.0.0.1");
  EXPECT_EQ(addr.toIpPort(), "127.0.0.1:1234");
}

TEST(InetAddressTest, IpStringConstruction) {
  InetAddress addr0("1.2.3.4", 8888);
  EXPECT_EQ(addr0.toIp(), "1.2.3.4");
  EXPECT_EQ(addr0.toIpPort(), "1.2.3.4:8888");
  EXPECT_EQ(addr0.port(), static_cast<uint16_t>(8888));
}

TEST(InetAddressTest, NetworkEndian) {
  InetAddress addr(1234);
  const struct sockaddr* generic_addr = addr.getSockAddr();
  // 需要转换为sockaddr_in*来访问IP和端口
  const struct sockaddr_in* addr_in =
      reinterpret_cast<const struct sockaddr_in*>(generic_addr);
  EXPECT_EQ(ntohs(addr_in->sin_port), static_cast<uint16_t>(1234));
  EXPECT_EQ(ntohl(addr_in->sin_addr.s_addr), INADDR_ANY);
}

// 测试IPv4地址转换
TEST(InetAddressTest, IPv4AddressConversion) {
  const char* ip = "192.168.1.1";
  uint16_t port = 8080;
  InetAddress addr(ip, port);

  const struct sockaddr* generic_addr = addr.getSockAddr();
  const struct sockaddr_in* addr_in =
      reinterpret_cast<const struct sockaddr_in*>(generic_addr);

  char buf[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &addr_in->sin_addr, buf, INET_ADDRSTRLEN);
  EXPECT_STREQ(buf, ip);
}

// 测试端口范围
TEST(InetAddressTest, PortRange) {
  // 测试边界值
  InetAddress addr1(0);      // 最小端口
  InetAddress addr2(65535);  // 最大端口

  const struct sockaddr* generic_addr1 = addr1.getSockAddr();
  const struct sockaddr* generic_addr2 = addr2.getSockAddr();

  const struct sockaddr_in* addr_in1 =
      reinterpret_cast<const struct sockaddr_in*>(generic_addr1);
  const struct sockaddr_in* addr_in2 =
      reinterpret_cast<const struct sockaddr_in*>(generic_addr2);

  EXPECT_EQ(ntohs(addr_in1->sin_port), static_cast<uint16_t>(0));
  EXPECT_EQ(ntohs(addr_in2->sin_port), static_cast<uint16_t>(65535));
}

// 测试地址族
TEST(InetAddressTest, AddressFamily) {
  InetAddress addr("127.0.0.1", 8080);
  const struct sockaddr* generic_addr = addr.getSockAddr();
  EXPECT_EQ(generic_addr->sa_family, AF_INET);
}
