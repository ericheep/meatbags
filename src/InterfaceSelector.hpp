//
//  InterfaceSelector.hpp
//

#ifndef InterfaceSelector_hpp
#define InterfaceSelector_hpp

#include "ofMain.h"
#include <stdio.h>
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#else
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#endif

class InterfaceSelector {
public:
    InterfaceSelector();
    
    void listInterfaces();
    string getInterface(string _interfaceAndIP);
    string getIP(string _interfaceAndIP);
    
    struct interfaceAndIP {
        int index;
        int interfaceIndex;
        string interface;
        string IP;
        string interfaceAndIPString;    
    };
    
    vector<interfaceAndIP> interfaces;
    vector<string> interfacesStrings;
};

#endif /* InterfaceSelector_hpp */
