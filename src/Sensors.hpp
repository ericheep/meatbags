//
//  Sensors.hpp
//

#ifndef Sensors_hpp
#define Sensors_hpp

#include <stdio.h>
#include "Hokuyo.hpp"
#include "Bounds.hpp"

class Sensors {
public:
    Sensors();
    
    void update();
    void addSensor(Hokuyo* hokuyo);
    void setBounds(Bounds& bounds);
    void getCoordinatesAndIntensities(vector<ofPoint> &coordinates, vector <int> &intensities, int &numberCoordinates);
    bool areNewCoordinatesAvailable();

    vector <Hokuyo *> hokuyos;
    Bounds bounds;
    int numberCoordinates;
};

#endif /* Sensors_hpp */
