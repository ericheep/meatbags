//
//  Meatbags.cpp
//

#include "Meatbags.hpp"

Meatbags::Meatbags() {
    coordinates.resize(4320);
    intensities.resize(4320);
    
    numberCoordinates = 0;
    lastFrameTime = 0;
    
    boundsX1 = -2.5;
    boundsX2 = 2.5;
    boundsY1 = 0;
    boundsY2 = 5;
    
    blobPersistence.addListener(this, &Meatbags::setBlobPersistence);
}

void Meatbags::update() {
    lastFrameTime = ofGetLastFrameTime();
    
    for (auto& oldBlob : oldBlobs) {
        oldBlob.updateLifetime(lastFrameTime);
    }
    
    oldBlobs.erase(std::remove_if(oldBlobs.begin(), oldBlobs.end(), [](Blob blob) {
        return !blob.isAlive();
    }), oldBlobs.end());
}

void Meatbags::updateBlobs() {
    if (numberCoordinates == 0) return;
    
    clusterBlobs();
    matchBlobs();
    renewBlobs();
    addBlobs();
}

void Meatbags::clusterBlobs() {
    vector<struct point2> points;
    for (int i = 0; i < numberCoordinates; i++) {
        struct point2 point;
        point.x = coordinates[i].x;
        point.y = coordinates[i].y;
        points.push_back(point);
    }
    
    auto clusters = dbscan(points, epsilon, minPoints);
    
    // get new blobs
    newBlobs.clear();
    int index = 0;
    for(auto& cluster: clusters) {
        vector<ofPoint> clusterCoordinates;
        vector<int> clusterIntensities;
        int numberPoints = cluster.size();
        
        for (int i = 0; i < numberPoints; i++) {
            int coordinateIndex = cluster[i];
            clusterCoordinates.push_back(coordinates[coordinateIndex]);
            clusterIntensities.push_back(intensities[coordinateIndex]);
        }
        
        Blob newBlob = Blob(clusterCoordinates,
                            clusterIntensities,
                            blobPersistence,
                            numberPoints);
        
        newBlob.index = index;
        newBlobs.push_back(newBlob);
        
        index++;
    }
    
    // if no olds blobs present then we use all the new blobs
    if (oldBlobs.size() == 0) {
        int index = 0;
        for (auto& newBlob : newBlobs) {
            Blob blob = Blob();
            blob.become(newBlob);
            blob.index = index;
            oldBlobs.push_back(blob);
            
            index++;
        }
    }
    
    // initialization
    for (auto& oldBlob : oldBlobs) {
        oldBlob.setMatched(false);
    }
}

float Meatbags::compareBlobs(Blob newBlob, Blob oldBlob) {
    float epsilon = 1;
    float squareDistance = newBlob.centroid.squareDistance(oldBlob.centroid);

    return 1.0 / (squareDistance + epsilon);
}

void Meatbags::matchBlobs() {
    for (auto& newBlob : newBlobs) {
        int potentialMatchIndex = 0;
        float potentialHighestScore = 0.0;
        
        for (auto& oldBlob : oldBlobs) {
            float score = compareBlobs(newBlob, oldBlob);
            
            if (score > potentialHighestScore) {
                potentialMatchIndex = oldBlob.index;
                potentialHighestScore = score;
            }
        }
        
        newBlob.setPotentialMatch(potentialMatchIndex, potentialHighestScore);
    }
    
    sort(newBlobs.begin(), newBlobs.end(), [](const Blob& a, const Blob& b){
        return a.potentialMatchScore > b.potentialMatchScore;
    });
    
    for (auto& newBlob : newBlobs) {
        for (auto& oldBlob : oldBlobs) {
            if (oldBlob.index == newBlob.potentialMatchIndex && !newBlob.isMatched() && !oldBlob.isMatched()) {
                oldBlob.become(newBlob);
                
                newBlob.setMatched(true);
                oldBlob.setMatched(true);
            }
        }
    }
}

void Meatbags::addBlobs() {
    for (auto& newBlob : newBlobs) {
        if (!newBlob.isMatched()) {
            Blob blob;
            
            int freeIndex = findFreeBlobIndex();
            
            blob.index = freeIndex;
            blob.setMatched(true);
            blob.become(newBlob);
            
            oldBlobs.push_back(blob);
        }
    }
}

void Meatbags::renewBlobs() {
    for (auto& oldBlob : oldBlobs) {
        if (oldBlob.isMatched()) {
            oldBlob.lifetime = 0;
        }
    }
}

int Meatbags::findFreeBlobIndex() {
    int freeIndex = 0;
    Boolean lookingForFreeIndex = true;
    
    while (lookingForFreeIndex) {
        lookingForFreeIndex = false;
        
        for (Blob oldBlob : oldBlobs) {
            if (freeIndex == oldBlob.index) lookingForFreeIndex = true;
        }
        
        if (lookingForFreeIndex) freeIndex++;
    }
    
    return freeIndex;
}

void::Meatbags::getBlobs(vector<Blob> &blobs) {
    blobs.clear();
    for (auto& oldBlob : oldBlobs) {
        blobs.push_back(oldBlob);
    }
}

void Meatbags::setBlobPersistence(float & _blobPersistence) {
    blobPersistence = _blobPersistence;
    
    for (auto& oldBlob : oldBlobs) {
        oldBlob.lifetimeLength = blobPersistence;
    }
}
