//
//  Meatbags.cpp
//

#include "Meatbags.hpp"

Meatbags::Meatbags() {
    coordinates.resize(1440);
    intensities.resize(1440);
    filteredCoordinates.resize(1440);
    filteredIntensities.resize(1440);
    
    blobPersistence = 0.1;
    epsilon = 100.0;
    minPoints = 5;
   
    numberFilteredCoordinates = 0;
    lastUpdateTime = 0;
    timeBetweenUpdates = 0;
    
    boundsX1 = -2.5;
    boundsX2 = 2.5;
    boundsY1 = 0;
    boundsY2 = 5;
}

void Meatbags::update() {
    timeBetweenUpdates += ofGetLastFrameTime();
}

void Meatbags::updateBlobs() {
    lastUpdateTime = timeBetweenUpdates;
    timeBetweenUpdates = 0;
    
    filterCoordinates();
    calculateBlobs();
}

void Meatbags::filterCoordinates() {    
    int filteredIndex = 0;
    for (int i = 0; i < coordinates.size(); i++) {
        float x = coordinates[i].x;
        float y = coordinates[i].y;
        
        if (x != 0 && y != 0) {
            if (x < boundsX2 * 1000 &&
                x > boundsX1 * 1000 &&
                y > boundsY1 * 1000 &&
                y < boundsY2 * 1000)
            {
                filteredCoordinates[filteredIndex].set(x, y);
                filteredIntensities[filteredIndex] = intensities[i];
                filteredIndex++;
            }
        }
    }
    
    numberFilteredCoordinates = filteredIndex;
}

void Meatbags::clusterBlobs() {
    vector<struct point2> points;
    for (int i = 0; i < numberFilteredCoordinates; i++) {
        struct point2 point;
        point.x = filteredCoordinates[i].x;
        point.y = filteredCoordinates[i].y;
        points.push_back(point);
    }
    
    auto clusters = dbscan(points, epsilon, minPoints);
    
    // get new blobs
    newBlobs.clear();
    int index = 0;
    for(auto& cluster: clusters) {
        vector<ofPoint> coordinates;
        vector<int> intensities;
        int numberPoints = cluster.size();
        
        for (int i = 0; i < numberPoints; i++) {
            int coordinateIndex = cluster[i];
            coordinates.push_back(filteredCoordinates[coordinateIndex]);;
            intensities.push_back(filteredIntensities[coordinateIndex]);
        }
        
        Blob newBlob = Blob(coordinates, intensities, blobPersistence, numberPoints);
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

void Meatbags::removeBlobs() {
    for (auto& oldBlob : oldBlobs) {
        if (oldBlob.isMatched()) {
            oldBlob.lifetime = 0;
        } else {
            oldBlob.updateLifetime(lastUpdateTime);
        }
    }
    
    oldBlobs.erase(std::remove_if(oldBlobs.begin(), oldBlobs.end(), [](Blob blob) {
        return !blob.isAlive();
    }), oldBlobs.end());
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

void Meatbags::calculateBlobs() {
    if (filteredCoordinates.size() == 0) return;
    
    clusterBlobs();
    matchBlobs();
    removeBlobs();
    addBlobs();
}

void::Meatbags::getBlobs(vector<Blob> &blobs) {
    blobs.clear();
    for (auto& oldBlob : oldBlobs) {
        blobs.push_back(oldBlob);
    }
}

void Meatbags::setBounds(float _boundsX1, float _boundsX2, float _boundsY1, float _boundsY2) {
    boundsX1 = _boundsX1;
    boundsX2 = _boundsX2;
    boundsY1 = _boundsY1;
    boundsY2 = _boundsY2;
}

void Meatbags::setBlobPersistence(float _blobPersistence) {
    blobPersistence = _blobPersistence;
    
    for (auto& oldBlob : oldBlobs) {
        oldBlob.lifetimeLength = blobPersistence;
    }
}

void Meatbags::setEpsilon(float _epsilon) {
    epsilon = _epsilon;
}

void Meatbags::setMinPoints(int _minPoints) {
    minPoints = _minPoints;
}
