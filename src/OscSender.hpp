//
//  OscSender.hpp
//

#ifndef OscSender_hpp
#define OscSender_hpp

#include <stdio.h>
#include "ofxOsc.h"
#include "ofMain.h"
#include "Blob.hpp"
#include "Meatbags.hpp"
#include "Sensors.hpp"

class OscSender {
public:
    OscSender();
    ~OscSender();

    void setOscSenderAddress(string& oscSenderAddress);
    void setOscSenderPort(int& oscSenderPort);
    void sendBlobOsc(vector<Blob> & blobs, Meatbags & meatbags, Filters & filters);
    void sendLogs(Sensors & sensors);
    
    ofxOscSender oscSender;

    ofParameter<string> oscSenderAddress;
    ofParameter<int> oscSenderPort;
    ofParameter<bool> sendBlobsActive;
    ofParameter<bool> sendLogsActive;
    
    string lastConnectionStatus, lastLaserStatus, lastGeneralStatus;
};

#endif /* OscOut_hpp */
