//
//  MeatbagsManager.cpp
//  meatbags
//

#include "MeatbagsManager.hpp"

MeatbagsManager::MeatbagsManager() {
    
}

void MeatbagsManager::setMaxCoordinateSize(int maxCoordinateSize) {
    for (auto& entry : meatbagsEntries) {
        entry.meatbags->setMaxCoordinateSize(maxCoordinateSize);
    }
}

void MeatbagsManager::draw() {
    for (auto& entry : meatbagsEntries) {
        entry.gui->draw();
    }
}

void MeatbagsManager::update() {
    for (auto& entry : meatbagsEntries) {
        entry.meatbags->update();
    }
}

void MeatbagsManager::updateBlobs() {
    for (auto& entry : meatbagsEntries) {
        entry.meatbags->updateBlobs();
    }
}

vector<Meatbags*> MeatbagsManager::getMeatbags() {
    vector<Meatbags*> meatbags;
    for (auto& entry : meatbagsEntries) {
        meatbags.push_back(entry.meatbags.get());
    }
    return meatbags;
}

void MeatbagsManager::getBlobs(vector<Blob>& blobs) {
    blobs.clear();
    
    for (auto& entry : meatbagsEntries) {
        for (auto& oldBlob : entry.meatbags->oldBlobs) {
            oldBlob.whichMeatbag = entry.meatbags->index;
            blobs.push_back(oldBlob);
        }
    }
}

void MeatbagsManager::removeMeatbag() {
    meatbagsEntries.pop_back();
}

std::unique_ptr<ofxPanel> MeatbagsManager::createGUIForMeatbags(Meatbags* meatbags) {
    auto gui = std::make_unique<ofxPanel>();
    
    gui->setDefaultWidth(190);
    gui->setup("meatbags " + to_string(meatbags->index));
    gui->add(meatbags->epsilon.set( "cluster epsilon (mm)", 100, 1, 1000));
    gui->add(meatbags->minPoints.set( "cluster min points", 10, 1, 150));
    gui->add(meatbags->blobPersistence.set("blob persistence (s)", 0.25, 0.0, 3.0));
    
    return gui;
}

void MeatbagsManager::addMeatbags() {
    auto meatbags = std::make_unique<Meatbags>();
    if (meatbags) meatbags->index = meatbagsEntries.size() + 1;
    
    auto gui = createGUIForMeatbags(meatbags.get());
    meatbagsEntries.push_back({std::move(meatbags), std::move(gui)});
}


void MeatbagsManager::loadMeatbags(int numberMeatbags, ofJson config) {
    for (int i = 0; i < numberMeatbags; i++) {
        addMeatbags();
    }
    for (int i = 0; i < meatbagsEntries.size(); i++) {
        string meatbagsKey = "meatbags_" + to_string(i + 1);
        ofJson meatbagsConfig;
        meatbagsConfig[meatbagsKey] = config[meatbagsKey];
        meatbagsEntries[i].gui->loadFrom(meatbagsConfig);
    }
}

void MeatbagsManager::load(ofJson config) {
    if (config.contains("number_meatbags")) {
        int numberMeatbags = config["number_meatbags"];
        loadMeatbags(numberMeatbags, config);
    } else {
        addMeatbags();
        addMeatbags();
    }
}

void MeatbagsManager::saveTo(ofJson& config) {
    config["number_meatbags"] = meatbagsEntries.size();
    for (auto& entry : meatbagsEntries) {
        entry.gui->saveTo(config);
    }
}
