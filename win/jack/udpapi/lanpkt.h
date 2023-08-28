#pragma once

#include <stddef.h>
#include <stdio.h>

#pragma pack(1)

// 6 byte MAC address
typedef struct _mac_addr_t {
   uint8_t    mac[6];
} mac_addr_t, *pmac_addr_t;

// Ethernet Header
typedef struct _eth_header_t {
   mac_addr_t dmac;      // Destination
   mac_addr_t smac;      // Source
   uint16_t   type;      // Protocol Type
} eth_header_t, *peth_header_t;

// 4 byte IP address
typedef struct _ip_addr_t {
	uint8_t ip[4];
} ip_addr_t, *pip_addr_t;

// IPv4 Header
typedef struct _ip_header_t {
   uint8_t    ver;       // Version (4 bits) + Internet header length (4 bits)
   uint8_t    tos;       // Type of service
   uint16_t   tlen;      // Total length
   uint16_t   id;        // Identification
   uint16_t   flags;     // Flags (3 bits) + Fragment offset (13 bits)
	uint8_t    ttl;       // Time to live
	uint8_t    proto;     // Protocol
	uint16_t   crc;       // Header checksum
	ip_addr_t  saddr;     // Source address
	ip_addr_t  daddr;     // Destination address
} ip_header_t, *pip_header_t;

// UDP Header
typedef struct _udp_header_t {
   uint16_t   sport;     // Source port
   uint16_t   dport;     // Destination port
   uint16_t   len;       // Datagram length
   uint16_t   crc;       // Checksum
} udp_header_t, *pudp_header_t;

// CM UDP Control Message Packet
typedef struct _cm_udp_ctl_t {
   eth_header_t   eth_hdr;
   ip_header_t    ip_hdr;
   udp_header_t   udp_hdr;
   uint8_t        pad[6];
   uint8_t        body[512];
} cm_udp_ctl_t, *pcm_udp_ctl_t;

// CM UDP Pipe Message Packet
typedef struct _cm_udp_pipe_t {
   eth_header_t   eth_hdr;
   ip_header_t    ip_hdr;
   udp_header_t   udp_hdr;
   uint8_t        pad[6];
   uint8_t        body[1024];
} cm_udp_pipe_t, *pcm_udp_pipe_t;

// ARP message body
typedef struct _arp_body_t {
   uint16_t   hw_type;
   uint16_t   proto_type;
   uint8_t    hw_size;
   uint8_t    proto_size;
   uint16_t   op_code;
   mac_addr_t sender_mac;
   ip_addr_t  sender_ip;
   mac_addr_t target_mac;
   ip_addr_t  target_ip;
} arp_body_t, *parp_body_t;

// ICMP message body
typedef struct _icmp_body_t {
   uint8_t    type;
   uint8_t    code;
   uint16_t   chksum;
   uint16_t   id;
   uint16_t   seq;
} icmp_body_t, *picmp_body_t;

// ARP message
typedef struct _arp_msg_t {
   eth_header_t eth_hdr;
   arp_body_t  arp;
   uint8_t     pad[18];
} arp_msg_t, *parp_msg_t;

// ICMP ping message
typedef struct _icmp_ping_t {
   eth_header_t   eth_hdr;
   ip_header_t    ip_hdr;
   icmp_body_t    icmp;
   uint8_t        data[256];
} icmp_ping_t, *picmp_ping_t;

// PCAP file header
typedef struct _pcap_hdr_t {
   uint32_t    magic_number;  /* magic number */
   uint16_t    version_major; /* major version number */
   uint16_t    version_minor; /* minor version number */
   int16_t     thiszone;      /* GMT to local correction */
   uint32_t    sigfigs;       /* accuracy of timestamps */
   uint32_t    snaplen;       /* max length of captured packets, in octets */
   uint32_t    network;       /* data link type */
} pcap_hdr_t, *ppcap_hdr_t;

// PCAP record header
typedef struct _pcaprec_hdr_t {
   uint32_t ts_sec;           /* timestamp seconds */
   uint32_t ts_usec;          /* timestamp microseconds */
   uint32_t incl_len;         /* number of octets of packet saved in file */
   uint32_t orig_len;         /* actual length of packet */
} pcaprec_hdr_t, *ppcaprec_hdr_t;

#pragma pack()
