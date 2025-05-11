//
//  InterfaceSelector.hpp
//

#ifndef InterfaceSelector_hpp
#define InterfaceSelector_hpp

#include "ofMain.h"
#include <stdio.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <iostream>
#include <net/if.h>       // for if_nametoindex()
#include <sys/socket.h>   // for setsockopt()
#include <netinet/in.h>   // for IPPROTO_IP
#include <netinet/ip.h>   // for IP_BOUND_IF (macOS-specific)

class InterfaceSelector {
public:
    InterfaceSelector();
    
    void listInterfaces();
    void selectInterface();
    string getInterface(string interfaceAndIP);
    string getIP(string interfaceAndIP);
    
    struct interfaceAndIP {
        int index;
        int interfaceIndex;
        string interface;
        string IP;
        string interfaceAndIP;
    };
    
    vector<interfaceAndIP> interfaces;
    vector<string> interfacesStrings;
};

#endif /* InterfaceSelector_hpp */
