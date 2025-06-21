//
//  OscSenders.hpp
//

#ifndef OscSenders_hpp
#define OscSenders_hpp

#include <stdio.h>
#include "OscSender.hpp"
#include "Blob.hpp"
#include "Meatbags.hpp"
#include "Sensors.hpp"

class OscSenders {
public:
    OscSenders();
    
    void addOscSender(OscSender * oscSender);
    void removeOscSender();
    void send(vector<Blob>& blobs, Sensors& sensors, Filters& filters);
    
    vector <OscSender *> oscSenders;
};


#endif /* OscSenders_hpp */
