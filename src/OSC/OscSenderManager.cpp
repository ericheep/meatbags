//
//  OscSenderManager.cpp
//

#include "OscSenderManager.hpp"

OscSenderManager::OscSenderManager() {
}

void OscSenderManager::draw() {
    for (auto& entry : oscSenderEntries) {
        if (entry.gui) {
            entry.gui->draw();
        }
    }
}

void OscSenderManager::send(vector<Blob>& blobs, const vector<Sensor*> sensors, const vector<Filter*>& filters) {
    for (auto & entry : oscSenderEntries) {
        if (entry.oscSender->sendBlobsActive) entry.oscSender->sendBlobOsc(blobs, filters);
        if (entry.oscSender->sendFiltersActive) entry.oscSender->sendFilterOsc(filters);
        if (entry.oscSender->sendLogsActive ) entry.oscSender->sendLogs(sensors);
    }
}

void OscSenderManager::addOscSender() {
    if (oscSenderEntries.size() < 5) {
        auto oscSender = std::make_unique<OscSender>();
        if (oscSender) oscSender->index = oscSenderEntries.size() + 1;
        
        auto gui = createGUIForSender(oscSender.get());
        oscSenderEntries.push_back({std::move(oscSender), std::move(gui)});
        
        refreshGUIPositions();
    }
}

void OscSenderManager::removeOscSender() {
    if (oscSenderEntries.size() > 1) {
        oscSenderEntries.pop_back();
    }
}

std::unique_ptr<ofxPanel> OscSenderManager::createGUIForSender(OscSender* oscSender) {
    
    ofColor guiBarColor;
    guiBarColor = ofColor::grey;
    guiBarColor.a = 100;
    ofxGuiSetFillColor(guiBarColor);
    
    auto gui = std::make_unique<ofxPanel>();
    
    ofColor filterColor = ofColor::thistle;
    ofColor backgroundColor = ofColor::snow;
    backgroundColor.a = 210;
    
    gui->setDefaultBackgroundColor(backgroundColor);
    gui->setBackgroundColor(backgroundColor);
    gui->setHeaderBackgroundColor(filterColor);
    gui->setFillColor(guiBarColor);
    gui->setDefaultFillColor(guiBarColor);
    
    gui->setDefaultWidth(190);
    gui->setup("osc sender " + to_string(oscSender->index));
    gui->add(oscSender->oscSenderAddress.set("ip address", "127.0.0.1"));
    gui->add(oscSender->oscSenderPort.set("port", 5432));
    gui->add(oscSender->sendBlobsActive.set("send blobs", false));
    gui->add(oscSender->sendFiltersActive.set("send filters", false));
    gui->add(oscSender->sendLogsActive.set("send logs", false));
    
    return gui;
}

void OscSenderManager::refreshGUIPositions() {
    for (int i = 0; i < oscSenderEntries.size(); ++i) {
        auto& entry = oscSenderEntries[i];
        if (!(entry.gui && entry.oscSender)) continue;
        
        int guiWidth = 190;
        int guiHeight = 73;
        int margin = 10;

        int xPos = ofGetWidth() - guiWidth - margin - 197;
        int yPos = margin + i * (guiHeight + margin);
        
        entry.gui->setPosition(xPos, yPos);
    }
}

void OscSenderManager::initialize() {
    addOscSender();
}

void OscSenderManager::loadOscSenders(int numberOscSenders, ofJson config) {
    for (int i = 0; i < numberOscSenders; i++) {
        addOscSender();
    }
    for (int i = 0; i < oscSenderEntries.size(); i++) {
        string oscSenderKey = "osc_sender_" + to_string(i + 1);
        ofJson oscSenderConfig;
        oscSenderConfig[oscSenderKey] = config[oscSenderKey];
        oscSenderEntries[i].gui->loadFrom(oscSenderConfig);
    }
}

void OscSenderManager::load(ofJson config) {
    if (config.contains("number_osc_senders")) {
        int numberOscSenders = config["number_osc_senders"];
        loadOscSenders(numberOscSenders, config);
    } else {
        addOscSender();
    }
}

void OscSenderManager::saveTo(ofJson& config) {
    config["number_osc_senders"] = oscSenderEntries.size();
    for (auto& entry : oscSenderEntries) {
        entry.gui->saveTo(config);
    }
}
