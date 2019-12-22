/*
Description:    ARP socket class. Provides functions for ARP sockets,
.               including creating, sending, receiving, etc.
.               This program is a part of arp_bomber.
Author:         Hanson (SweetIceLolly)
Website:        https://github.com/SweetIceLolly/arp_bomber
File:           arp_packet.hpp
*/

#include <linux/wireless.h>
#include <linux/if_arp.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#ifndef __IP_ADDR
#define __IP_ADDR
struct ip_addr {
    unsigned char ip_value[4];

    unsigned char operator [] (int i) const {
        return ip_value[i];
    }

    unsigned char & operator [] (int i) {
        return ip_value[i];
    }
};
#endif              //__IP_ADDR

#ifndef __MAC_ADDR
#define __MAC_ADDR
struct mac_addr {
    unsigned char mac_value[6];

    unsigned char operator [] (int i) const {
        return mac_value[i];
    }

    unsigned char & operator [] (int i) {
        return mac_value[i];
    }
};
#endif              //__MAC_ADDR

struct __attribute__((packed)) arp_packet {
    union {
        unsigned char   destination_mac[6];
        mac_addr        destination_mac_addr;
    };
    union {
        unsigned char   source_mac[6];
        mac_addr        source_mac_addr;
    };
    unsigned char   type[2] = {0x8, 0x6};           //(0x8, 0x6) for ARP
    unsigned char   hardware_type[2] = {0x0, 0x1};  //(0x0, 0x1) for ethernet
    unsigned char   protocol_type[2] = {0x8, 0x0};  //(0x8, 0x0) for IPv4
    unsigned char   hardware_size = 0x6;            //0x6 for MAC addresses
    unsigned char   protocol_size = 0x4;            //0x4 for ethernet
    unsigned char   opcode[2] = {0x0, 0x1};         //(0x0, 0x1) = ARP request, (0x0, 0x2) = ARP reply
    union {
        unsigned char   sender_mac[6];
        mac_addr        sender_mac_addr;
    };
    union {
        unsigned char   sender_ip[4];
        ip_addr         sender_ip_addr;
    };
    union {
        unsigned char   target_mac[6];
        mac_addr        target_mac_addr;
    };
    union {
        unsigned char   target_ip[4];
        ip_addr         target_ip_addr;
    };
};

class arp_socket {
private:
    int                 socket_descr = -1;      //ARP socket descriptor
    struct sockaddr_ll  socket_address;         //Socket address

public:
    /*
    Purpose:    To create the ARP socket
    Args:       interface_name: Network interface name. (e.g. eth0)
    Return:     -1: Failed to create the socket;
    .           Other value: Socket created successfully
    */
    int create(const char *interface_name) {
        //Handle NULL string pointer
        if (interface_name == NULL) {
            return -1;
        }

        //Create the socket
        socket_descr = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (socket_descr == -1) {               //Failed to create socket
            return -1;
        }
        
        //Get network interface index
        struct ifreq    ifr;
        strncpy(ifr.ifr_ifrn.ifrn_name, interface_name, IFNAMSIZ);
        if (ioctl(socket_descr, SIOCGIFINDEX, &ifr) == -1) {
            ::close(socket_descr);
            return -1;
        }

        //Set address info
        socket_address.sll_family = PF_PACKET;
        socket_address.sll_protocol = htons(ETH_P_ARP);
        socket_address.sll_ifindex = ifr.ifr_ifru.ifru_ivalue;
        socket_address.sll_hatype = ARPHRD_ETHER;
        socket_address.sll_pkttype = 0;
        socket_address.sll_halen = 0;
        socket_address.sll_addr[6] = 0x00;
        socket_address.sll_addr[7] = 0x00;

        return socket_descr;
    }

    /*
    Purpose:    To close the ARP socket
    */
    void close() {
        ::close(socket_descr);
    }

    /*
    Purpose:    Receive data
    Args:       packet_content: The arp_packet structure to store data
    Return:     -1: Failed to receive data;
    .           Other value: The length of received data
    */
    int recv(arp_packet *packet_content) {
        memset(packet_content, 0, sizeof(arp_packet));
        return recvfrom(socket_descr, packet_content, sizeof(arp_packet), 0, NULL, NULL);
    }

    /*
    Purpose:    Send data
    Args:       packet_content: The ARP packet to be sent
    Return:     -1: Failed to send data;
    .           Other value: The length of sent data
    */
    int send(arp_packet packet_content, mac_addr source_mac_addr) {
        memcpy(&socket_address.sll_addr, &source_mac_addr, 6);
        return sendto(socket_descr, &packet_content, sizeof(packet_content), 0,
            (struct sockaddr*)&socket_address, sizeof(socket_address));
    }

    /*
    Purpose:    Receive raw data
    Args:       buffer: The buffer to store data
    .           buffer_size: Size of the buffer to store data
    Return:     -1: Failed to receive data;
    .           Other value: The length of received data
    */
    int recv(unsigned char *buffer, int buffer_size) {
        memset(buffer, 0, buffer_size);
        return recvfrom(socket_descr, buffer, buffer_size, 0, NULL, NULL);
    }

    /*
    Purpose:    Send raw data
    Args:       buffer: The buffer to be sent
    .           buffer_size: Size of the buffer to be sent
    Return:     -1: Failed to send data;
    .           Other value: The length of sent data
    */
    int send(unsigned char *buffer, int buffer_size) {
        return sendto(socket_descr, buffer, buffer_size, 0,
            (struct sockaddr*)&socket_address, sizeof(socket_address));
    }
};
