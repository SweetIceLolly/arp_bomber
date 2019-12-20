/*
Description:    This header provides functions for network interfaces,
.               including listing, retrieving IP, etc.
.               This program is a part of arp_bomber.
Author:         Hanson (SweetIceLolly)
Website:        https://github.com/SweetIceLolly/arp_bomber
File:           net_interface.hpp
*/

#include <sys/ioctl.h>
#include <linux/wireless.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <vector>

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

/*
Purpose:    To check if the interface is wireless or not
Args:       interface_name: Network interface name
Return:     1: The interface is wireless; 0: The interface is not wireless
*/
int check_interface_wireless(const char *interface_name) {
    int             test_socket = -1;                                           //The socket to test interface
    struct iwreq    pwrq = { 0 };                                               //Interface name structure

    strncpy(pwrq.ifr_ifrn.ifrn_name, interface_name, IFNAMSIZ);
    
    test_socket = socket(AF_INET, SOCK_STREAM, 0);                              //Create a test socket
    if (test_socket == -1) {
        return 0;
    }

    if (ioctl(test_socket, SIOCGIWNAME, &pwrq) != -1) {                         //Verify if the interface is wireless or not
        close(test_socket);
        return 1;
    }
    close(test_socket);
    return 0;
}

/*
Purpose:    Get gateway IP
Return:     ip_addr struct. If this function fails, the struct will be filled with 0
*/
struct ip_addr get_gateway_ip() {
    FILE            *command_pipe;              //Shell command pipe
    char            shell_output[1024];         //Shell command output
    struct ip_addr  rtn = { 0 };
    int             temp_data[4];               //Integer data for unsigned char conversion

    command_pipe = popen("ip -4 neigh | cut -d ' ' -f1", "r");
    if (!command_pipe) {
        pclose(command_pipe);
        return rtn;
    }
    fscanf(command_pipe, "%i.%i.%i.%i", &temp_data[0], &temp_data[1],
                                        &temp_data[2], &temp_data[3]);
    rtn.ip_value[0] = temp_data[0];
    rtn.ip_value[1] = temp_data[1];
    rtn.ip_value[2] = temp_data[2];
    rtn.ip_value[3] = temp_data[3];
    pclose(command_pipe);
    return rtn;
}

/*
Purpose:    Get gateway MAC
Return:     IP struct. If this function fails, the struct will be filled with 0
*/
struct mac_addr get_gateway_mac() {
    FILE            *command_pipe;              //Shell command pipe
    char            shell_output[1024];         //Shell command output
    struct mac_addr rtn = { 0 };
    int             temp_data[6];               //Integer data for unsigned char conversion

    command_pipe = popen("ip -4 neigh | cut -d ' ' -f5", "r");
    if (!command_pipe) {
        pclose(command_pipe);
        return rtn;
    }
    fscanf(command_pipe, "%x:%x:%x:%x:%x:%x", &temp_data[0], &temp_data[1], &temp_data[2],
                                              &temp_data[3], &temp_data[4], &temp_data[5]);
    rtn.mac_value[0] = temp_data[0];
    rtn.mac_value[1] = temp_data[1];
    rtn.mac_value[2] = temp_data[2];
    rtn.mac_value[3] = temp_data[3];
    rtn.mac_value[4] = temp_data[4];
    rtn.mac_value[5] = temp_data[5];
    pclose(command_pipe);
    return rtn;
}

/*
Purpose:    List all network interfaces
Args:       interface_count: Optional. An int to reveive interface count
Return:     A list of ifaddrs
*/
std::vector<struct ifaddrs> list_net_interfaces(int *interface_count) {
    struct ifaddrs              *ifaddr;
    struct ifaddrs              info;
    std::vector<struct ifaddrs> list;

    if (getifaddrs(&ifaddr) == -1) {
        if (interface_count) {
            *interface_count = 0;
        }
        return list;
    }

    while (ifaddr != NULL) {
        //Look for capable interfaces
        if (ifaddr->ifa_addr != NULL && ifaddr->ifa_addr->sa_family == AF_PACKET) {
            memcpy(&info, ifaddr, sizeof(info));
            list.push_back(info);
        }
        ifaddr = ifaddr->ifa_next;
    }
    if (interface_count) {
        *interface_count = list.size();
    }
    return list;
}

/*
Purpose:    List all network interfaces
Return:     A list of ifaddrs
*/
std::vector<struct ifaddrs> list_net_interfaces() {
    return list_net_interfaces(NULL);
}
