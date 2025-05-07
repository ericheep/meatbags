//
//  OscSenders.cpp
//

#include "OscSenders.hpp"

OscSenders::OscSenders() {
    
}

void OscSenders::send(vector<Blob> & blobs, Meatbags & meatbags, Sensors & sensors, Filters & filters) {
    for (auto & oscSender : oscSenders) {
        oscSender->sendBlobOsc(blobs, meatbags, filters);
        oscSender->sendLogs(sensors);
    }
}

void OscSenders::addOscSender(OscSender* oscSender) {
    oscSenders.push_back(oscSender);
}

void OscSenders::removeOscSender() {
    oscSenders.pop_back();
}
