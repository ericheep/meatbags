//
//  OscSender.hpp
//

#ifndef OscSender_hpp
#define OscSender_hpp

#include <stdio.h>
#include "ofxOsc.h"
#include "ofxOpenCv.h"
#include "ofMain.h"
#include "Blob.hpp"
#include "Meatbags.hpp"
#include "Filter.hpp"
#include "Sensor.hpp"

class OscSender {
public:
    OscSender();
    ~OscSender();

    void setOscSenderAddress(string& oscSenderAddress);
    void setOscSenderPort(int& oscSenderPort);
    void sendBlobOsc(vector<Blob>& blobs, const vector<Filter*>& filters);
    void sendFilterOsc(const vector<Filter*>& filters);
    void sendFilterStatus(const vector<Filter*>& filters);
    void sendFilterBlob(const vector<Filter *>& filters);
    void sendFilterBlobs(const vector<Filter*>& filters);
    void sendLogs(const vector<Sensor*> sensors);
    
    ofxOscSender oscSender;

    ofParameter<string> oscSenderAddress;
    ofParameter<int> oscSenderPort;
    ofParameter<bool> sendBlobsActive;
    ofParameter<bool> sendFiltersActive;
    ofParameter<bool> sendLogsActive;

    string lastConnectionStatus, lastMode, lastStatus;
    int index;
};

#endif /* OscOut_hpp */
