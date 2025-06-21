//
//  MeatbagsFactory.cpp
//  meatbags
//

#include "MeatbagsFactory.hpp"

MeatbagsFactory::MeatbagsFactory() {
    
}

void MeatbagsFactory::setMaxCoordinateSize(int maxCoordinateSize) {
    for (auto& meatbag : meatbags) {
        meatbag->setMaxCoordinateSize(maxCoordinateSize);
    }
}

void MeatbagsFactory::update() {
    for (auto& meatbag : meatbags) {
        meatbag->update();
    }
}

void MeatbagsFactory::updateBlobs() {
    for (auto& meatbag : meatbags) {
        meatbag->updateBlobs();
    }
}

void MeatbagsFactory::getBlobs(vector<Blob>& blobs) {
    blobs.clear();
    
    for (auto& meatbag : meatbags) {
        for (auto& oldBlob : meatbag->oldBlobs) {
            oldBlob.whichMeatbag = meatbag->index;
            blobs.push_back(oldBlob);
        }
    }
}

void MeatbagsFactory::addMeatbag(Meatbags* meatbag) {
    meatbags.push_back(meatbag);
}

void MeatbagsFactory::removeMeatbag() {
    meatbags.pop_back();
}
