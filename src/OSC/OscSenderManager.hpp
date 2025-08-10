//
//  OscSenders.hpp
//

#ifndef OscSenderManager_hpp
#define OscSenderManager_hpp

#include <stdio.h>
#include "ofxGui.h"
#include "OscSender.hpp"
#include "Blob.hpp"
#include "Meatbags.hpp"
#include "Sensor.hpp"

class OscSenderManager {
public:
    OscSenderManager();
    
    void draw();
    void addOscSender();
    void removeOscSender();
    void send(vector<Blob>& blobs, const vector<Sensor*> sensors, const vector<Filter*>& filters);
    
    void load(ofJson configuration);
    void loadOscSenders(int numberOscSenders, ofJson config);
    void saveTo(ofJson& configuration);
    
private:
    struct OscSenderEntry {
        unique_ptr<OscSender> oscSender;
        unique_ptr<ofxPanel> gui;
    };
    
    vector<OscSenderEntry> oscSenderEntries;
    unique_ptr<ofxPanel> createGUIForSender(OscSender* oscSender);
};

#endif /* OscSenders_hpp */
