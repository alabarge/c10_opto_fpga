#include <stdio.h>
#include <stdint.h>
#include <winsock2.h>
#include <sys/types.h>

#include "timer.h"

#pragma comment(lib,"ws2_32.lib")

#define SERVER "192.168.1.70" //ip address of udp server
#define BUFLEN 512   //Max length of buffer
#define PORT 2781 //The port on which to listen for incoming data

int main(void) {
   struct sockaddr_in si_other;
   int s, slen=sizeof(si_other);
   int rxbytes, txbytes;
   char buf[BUFLEN];
   char message[BUFLEN];
   WSADATA wsa;

   tmr_start();

   int err;
   char msgbuf [256] = {0};

   uint8_t query[]  = {0x00, 0x00, 0xAA, 0x55, 0x00, 0x00,
                       0x83, 0x83, 0x10, 0x10, 0x00, 0x00,
                       0x0C, 0x20, 0x83, 0x09, 0x00, 0x00};

   uint8_t txbuf[1024] = {0};
   uint8_t rxbuf[1024] = {0};

   memcpy(txbuf, query, sizeof(query));

   if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
      printf("Failed. Error Code : %d",WSAGetLastError());
      exit(EXIT_FAILURE);
   }

   //create socket
   if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
      printf("socket() failed with error code : %d" , WSAGetLastError());
      exit(EXIT_FAILURE);
   }

   struct timeval read_timeout;
   read_timeout.tv_sec = 0;
   read_timeout.tv_usec = 10;
   DWORD timeout = 100;
   setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof timeout);

   //setup address structure
   memset((char *) &si_other, 0, sizeof(si_other));
   si_other.sin_family = AF_INET;
   si_other.sin_port = htons(PORT);
   si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);

   //send the message
   txbytes = sendto(s, txbuf, 518, 0, (struct sockaddr *)&si_other, slen);
   if (txbytes == SOCKET_ERROR) {
      printf("sendto() failed with error code : %d" , WSAGetLastError());
      exit(EXIT_FAILURE);
   }

   printf("%d\n", (uint32_t)tmr_get_elapsed_microseconds());

   //try to receive some data, this is a blocking call with timeout
   rxbytes = recvfrom(s, rxbuf, sizeof(rxbuf), 0, (struct sockaddr *)&si_other, &slen);

   printf("%d\n", (uint32_t)tmr_get_elapsed_microseconds());

   if (rxbytes == SOCKET_ERROR) {
      err = WSAGetLastError ();
      FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
                     NULL,                // lpsource
                     err,                 // message id
                     MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
                     msgbuf,              // output buffer
                     sizeof (msgbuf),     // size of msgbuf, bytes
                     NULL);               // va_list of arguments
      if (! *msgbuf)
         sprintf (msgbuf, "%d", err);  // provide error # if no string available
      else
         printf("%s\n", msgbuf);
      printf("recvfrom() failed with error code : %d" , err);
      exit(EXIT_FAILURE);
   }
   else {
      printf("recvfrom() msglen = %d\n", rxbytes);
   }

   closesocket(s);
   WSACleanup();

   return 0;
}
