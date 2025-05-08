//
//  OscOscSender.cpp
//

#include "OscSender.hpp"

OscSender::OscSender() {
    oscSenderAddress.addListener(this, &OscSender::setOscSenderAddress);
    oscSenderPort.addListener(this, &OscSender::setOscSenderPort);
}

OscSender::~OscSender() {
    oscSenderAddress.removeListener(this, &OscSender::setOscSenderAddress);
    oscSenderPort.removeListener(this, &OscSender::setOscSenderPort);
}

void OscSender::setOscSenderAddress(string& oscSenderAddress) {
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

void OscSender::setOscSenderPort(int& oscSenderPort) {
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

void OscSender::sendBlobOsc(vector<Blob> & blobs, Meatbags & meatbags, Filters & filters) {
    if (!sendBlobsActive) return;
    
    for (auto & blob : blobs) {
        ofxOscMessage msg;
        msg.setAddress("/blob");
        msg.addIntArg(blob.index);
        
        // millimeters to meters
        float x = blob.centroid.x * 0.001;
        float y = blob.centroid.y * 0.001;
        
        msg.addFloatArg(x);
        msg.addFloatArg(y);
        msg.addFloatArg(blob.intensity);
        msg.addFloatArg(blob.distanceFromSensor * 0.001);
        
        // sends out which filter(s) the blob is in
        for (auto & filter : filters.filters) {
            if (filter->polyline.inside(x, y) && !filter->mask) {
                msg.addIntArg(filter->index);
            }
        }
    
        oscSender.sendMessage(msg);
    }
}

void OscSender::sendLogs(Sensors & sensors) {
    if (!sendLogsActive) return;

    for (auto & sensor : sensors.hokuyos) {
        string connectionStatus = sensor->connectionStatus;
        string generalStatus = sensor->status;
        string laserStatus = sensor->laserState;
        
        if (lastConnectionStatus != connectionStatus) {
            lastConnectionStatus = connectionStatus;
            
            ofxOscMessage msg;
            msg.setAddress("/connectionStatus");
            msg.addIntArg(sensor->index);
            msg.addStringArg(connectionStatus);
            oscSender.sendMessage(msg);
        }
        
        if (lastGeneralStatus != generalStatus) {
            lastGeneralStatus = generalStatus;
            
            ofxOscMessage msg;
            msg.setAddress("/generalStatus");
            msg.addIntArg(sensor->index);
            msg.addStringArg(generalStatus);
            oscSender.sendMessage(msg);
        }
        
        if (lastLaserStatus != laserStatus) {
            lastLaserStatus = laserStatus;
            
            ofxOscMessage msg;
            msg.setAddress("/laserStatus");
            msg.addIntArg(sensor->index);
            msg.addStringArg(laserStatus);
            oscSender.sendMessage(msg);
        }
    }
}
