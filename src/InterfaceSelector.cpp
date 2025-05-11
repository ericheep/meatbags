//
//  InterfaceSelector.cpp
//

#include "InterfaceSelector.hpp"

InterfaceSelector::InterfaceSelector() {
}

void InterfaceSelector::listInterfaces() {
    interfaces.clear();
    interfacesStrings.clear();
    
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return;
    }

    // Get adapter info
    IP_ADAPTER_ADDRESSES* pAddresses = NULL;
    ULONG outBufLen = 15000;
    pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
    if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_GATEWAYS, NULL, pAddresses, &outBufLen) == NO_ERROR) {
        // Iterate over the list of adapters
        for (IP_ADAPTER_ADDRESSES* pCurrAddresses = pAddresses; pCurrAddresses != NULL; pCurrAddresses = pCurrAddresses->Next) {
            if (pCurrAddresses->OperStatus != IfOperStatusUp) {
                continue; // Skip non-active interfaces
            }

            // Process only the IPv4 address
            // pCurrAddresses already contains only IPv4 addresses, so we don't need a second loop
            if (pCurrAddresses->FirstUnicastAddress != NULL) {
                sockaddr_in* sa_in = (sockaddr_in*)pCurrAddresses->FirstUnicastAddress->Address.lpSockaddr;

                // Use WSAAddressToString to convert IP to string format
                char ip[16]; // 16 bytes for IPv4 address
                DWORD dwSize = sizeof(ip);
                if (WSAAddressToStringA((LPSOCKADDR)sa_in, sizeof(*sa_in), NULL, ip, &dwSize) != 0) {
                    std::cerr << "WSAAddressToString failed" << std::endl;
                    continue;
                }

                interfaceAndIP ifaip;
                ifaip.IP = ip;
                ifaip.interface = pCurrAddresses->AdapterName; // AdapterName is the interface name

                std::string interfaceAndIPStr = ifaip.interface + ": " + ifaip.IP;
                ifaip.interfaceAndIPString = interfaceAndIPStr;

                interfaces.push_back(ifaip);
                interfacesStrings.push_back(interfaceAndIPStr);
            }
        }
    }
    else {
        std::cerr << "Error getting adapter addresses" << std::endl;
    }

    if (pAddresses) {
        free(pAddresses);
    }

    WSACleanup();

#else
    struct ifaddrs* ifaddr;
    getifaddrs(&ifaddr);

    for (auto* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;

        char ip[INET_ADDRSTRLEN];
        auto* sin = (struct sockaddr_in*)ifa->ifa_addr;
        inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip));

        interfaceAndIP ifaip;
        ifaip.IP = ip;
        ifaip.interface = ifa->ifa_name;

        string interfaceAndIP = ifaip.interface + ": " + ifaip.IP;
        ifaip.interfaceAndIP = interfaceAndIP;

        interfaces.push_back(ifaip);
        interfacesStrings.push_back(interfaceAndIP);
    }

    freeifaddrs(ifaddr);
#endif
}

string InterfaceSelector::getInterface(string interfaceAndIP) {
    string interfaceString = "";
    
    for (auto interface : interfaces) {
        if (interfaceAndIP == interface.interfaceAndIPString) {
            interfaceString = interface.interface;
        }
    }
    
    return interfaceString;
}

string InterfaceSelector::getIP(string interfaceAndIP) {
    string IPString = "";
    
    for (auto interface : interfaces) {
        if (interfaceAndIP == interface.interfaceAndIPString) {
            IPString = interface.IP;
        }
    }
    
    return IPString;
}
