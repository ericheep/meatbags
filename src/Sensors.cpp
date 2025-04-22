//
//  Sensors.cpp
//

#include "Sensors.hpp"

Sensors::Sensors() {
    
}

void Sensors::update() {
    for (auto& hokuyo : hokuyos) {
        hokuyo->update();
    }
}

void Sensors::addSensor(Hokuyo* hokuyo) {
    hokuyos.push_back(hokuyo);
}

void Sensors::closeSensors() {
    for (auto& hokuyo : hokuyos) {
        hokuyo->close();
    }
}

void Sensors::setBounds(Bounds& _bounds) {
    bounds = _bounds;
}

bool Sensors::areNewCoordinatesAvailable() {
    bool newCoordinatesAvalable = false;
    for (auto& hokuyo : hokuyos) {
        if (hokuyo->newCoordinatesAvailable) newCoordinatesAvalable = true;
    }
    return newCoordinatesAvalable;
}

void Sensors::getCoordinatesAndIntensities(vector<ofPoint>& coordinates, vector <int>& intensities, int &numberCoordinates) {
    int counter = 0;
    for (auto& hokuyo : hokuyos) {
        int intensityIndex = 0;
        for (auto& coordinate : hokuyo->coordinates) {
            float x = coordinate.x;
            float y = coordinate.y;
            
            if (coordinate.x != 0 && coordinate.x != 0) {
                

                if (x < bounds.x2 * 1000 &&
                    x > bounds.x1 * 1000 &&
                    y > bounds.y1 * 1000 &&
                    y < bounds.y2 * 1000)
                {
                    coordinates[counter].set(x, y);
                    intensities[counter] = hokuyo->intensities[intensityIndex];
                    counter++;
                }
            }
            
            intensityIndex++;
        }
    }

    numberCoordinates = counter;
}
