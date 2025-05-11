//
//  InterfaceSelector.cpp
//

#include "InterfaceSelector.hpp"

InterfaceSelector::InterfaceSelector() {
    
}

void InterfaceSelector::selectInterface() {
    int sock = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (sock < 0) {
        perror("socket creation failed");
        return;
    }
    
    int ifIndex = if_nametoindex("en7"); // Replace "en0" with your interface name
        if (ifIndex > 0) {
            if (setsockopt(sock, IPPROTO_IP, IP_BOUND_IF, &ifIndex, sizeof(ifIndex)) < 0) {
                perror("setsockopt IP_BOUND_IF failed");
            } else {
                printf("Successfully bound to interface en7\n");
            }
        } else {
            perror("Invalid interface name");
        }
}

void InterfaceSelector::listInterfaces() {
    struct ifaddrs* ifaddr;
    getifaddrs(&ifaddr);
    
    interfaces.clear();
    interfacesStrings.clear();
    
    for (auto* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
        
        char ip[INET_ADDRSTRLEN];
        auto* sin = (struct sockaddr_in*)ifa->ifa_addr;
        inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip));        
        
        interfaceAndIP ifaip;
        ifaip.IP = ip;
        ifaip.interface = ifa->ifa_name;
        
        string interfaceAndIP = ifaip.interface + ": " +  ifaip.IP;
        ifaip.interfaceAndIP = interfaceAndIP;
        
        interfaces.push_back(ifaip);
        interfacesStrings.push_back(interfaceAndIP);
    }
    
    freeifaddrs(ifaddr);
}

string InterfaceSelector::getInterface(string interfaceAndIP) {
    string interfaceString = "";
    
    for (auto interface : interfaces) {
        if (interfaceAndIP == interface.interfaceAndIP) {
            interfaceString = interface.interface;
        }
    }
    
    return interfaceString;
}

string InterfaceSelector::getIP(string interfaceAndIP) {
    string IPString = "";
    
    for (auto interface : interfaces) {
        if (interfaceAndIP == interface.interfaceAndIP) {
            IPString = interface.IP;
        }
    }
    
    return IPString;
}
