#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "udp_log.h"

static void udp_send_log_entry(char* data, int size) {
  static int debug_socket = -1;
  static struct sockaddr_in debug_addr;
  if (debug_socket == -1) {
    debug_addr.sin_family = AF_INET;
    debug_addr.sin_port = htons(UDP_DEBUG_PORT);
    debug_addr.sin_addr.s_addr = inet_addr(UDP_DEBUG_IP);
    debug_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  }
  sendto(debug_socket, data, size, 0, (struct sockaddr *)&debug_addr, sizeof(debug_addr));
}

void udp_log(const char* format, ...) {
  static int msg_count = 0;
  char buf[1024];
  int size = 0;
  memset(buf, 0, sizeof(buf));
  va_list args;
  size += sprintf(buf, "%08x: ", msg_count);
  msg_count++;
  va_start(args, format);
  size += vsnprintf(&buf[size], sizeof(buf) - size, format, args);
  va_end(args);
  udp_send_log_entry(buf, size);
}

void udp_log_raw(const char* format, ...) {
  char buf[1024];
  int size = 0;
  memset(buf, 0, sizeof(buf));
  va_list args;
  va_start(args, format);
  size += vsnprintf(&buf[size], sizeof(buf) - size, format, args);
  va_end(args);
  udp_send_log_entry(buf, size);
}


