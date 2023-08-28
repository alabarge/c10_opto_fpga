
/* Debug messages go to this IP address... */
#ifndef UDP_DEBUG_IP
#define UDP_DEBUG_IP "192.168.1.62"
#endif
/* ... and this port. */
/* To capture debug messages: nc -luv <port> */
#ifndef UDP_DEBUG_PORT
#define UDP_DEBUG_PORT 2782
#endif
void udp_log(const char* format, ...);
void udp_log_raw(const char* format, ...);

