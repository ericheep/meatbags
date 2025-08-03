//
//  OscOscSender.cpp
//

#include "OscSender.hpp"

OscSender::OscSender() {
    oscSenderAddress.addListener(this, &OscSender::setOscSenderAddress);
    oscSenderPort.addListener(this, &OscSender::setOscSenderPort);
    
    oscSenderAddress = "127.0.0.1";
    oscSenderPort = 5322;
}

OscSender::~OscSender() {
    oscSenderAddress.removeListener(this, &OscSender::setOscSenderAddress);
    oscSenderPort.removeListener(this, &OscSender::setOscSenderPort);
}

void OscSender::setOscSenderAddress(string& oscSenderAddress) {
    if (oscSenderPort == 0) return;
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

void OscSender::setOscSenderPort(int& oscSenderPort) {
    if (oscSenderPort == 0) return;
    oscSender.setup(oscSenderAddress, oscSenderPort);
}

void OscSender::sendBlobOsc(vector<Blob>& blobs, Filters& filters) {
    ofxOscMessage blobsActiveMsg;
    blobsActiveMsg.setAddress("/blobsActive");
    
    ofxOscMessage blobsMsg;
    blobsMsg.setAddress("/blobs");
    
    for (auto & blob : blobs) {
        ofxOscMessage msg;
        msg.setAddress("/blob");
        msg.addIntArg(blob.index);
        
        // millimeters to meters
        float x = blob.centroid.x * 0.001;
        float y = blob.centroid.y * 0.001;
        
        msg.addFloatArg(x);
        msg.addFloatArg(y);
        msg.addFloatArg(blob.bounds.width);
        msg.addFloatArg(blob.bounds.height);
        msg.addFloatArg(blob.intensity);
        
        // sends out which filter(s) the blob is in
        for (auto & filter : filters.filters) {
            if (filter->polyline.inside(x, y) && !filter->mask) {
                msg.addIntArg(filter->index);
            }
        }
    
        oscSender.sendMessage(msg);
        
        blobsMsg.addIntArg(blob.index);
        blobsMsg.addFloatArg(x);
        blobsMsg.addFloatArg(y);
        
        blobsActiveMsg.addIntArg(blob.index);
    }
    
    if (blobs.size() > 0) {
        oscSender.sendMessage(blobsMsg);
        oscSender.sendMessage(blobsActiveMsg);
    }
}

void OscSender::sendFilterOsc(Filters& filters) {
    sendFilterStatus(filters);
    sendFilterBlobs(filters);
}

void OscSender::sendFilterStatus(Filters& filters) {
    for (auto & filter : filters.filters) {
        ofxOscMessage msg;
        msg.setAddress("/filter");
        
        msg.addIntArg(filter->index);
        msg.addIntArg(filter->isBlobInside);

        if (filter->isBlobInside) msg.addFloatArg(filter->distanceOfClosestBlob);
        
        oscSender.sendMessage(msg);
    }
}

void OscSender::sendFilterBlobs(Filters& filters) {
    for (auto & filter : filters.filters) {
        if (!filter->isBlobInside) continue;
        
        ofxOscMessage msg;
        msg.setAddress("/filterBlobs");
        
        msg.addIntArg(filter->index);
        
        for (auto & blob : filter->filterBlobs) {
            msg.addIntArg(blob.index);
            
            float x = blob.centroid.x * 0.001;
            float y = blob.centroid.y * 0.001;
            
            if (filter->normalize) {
                cv::Point2f inputPoint(x, y);
                vector<cv::Point2f> inputVec = { inputPoint }, outputVec;
                cv::perspectiveTransform(inputVec, outputVec, filter->homography);
                
                x = outputVec[0].x;
                y = outputVec[0].y;
            }
            
            msg.addFloatArg(x);
            msg.addFloatArg(y);
        }
    
        oscSender.sendMessage(msg);
    }
}

void OscSender::sendLogs(Sensors& sensors) {
    for (auto & sensor : sensors.sensors) {
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
