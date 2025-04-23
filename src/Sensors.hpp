//
//  Sensors.hpp
//

#ifndef Sensors_hpp
#define Sensors_hpp

#include <stdio.h>
#include "Hokuyo.hpp"
#include "Bounds.hpp"

#include "superpose3d/superpose3d.hpp"
using namespace superpose3d;
#include <cpd/rigid.hpp>

class Sensors {
public:
    Sensors();
    
    void update();
    
    void addSensor(Hokuyo* hokuyo);
    void closeSensors();

    void setBounds(Bounds& bounds);
    void getCoordinatesAndIntensities(vector<ofPoint> &coordinates, vector <int> &intensities, int &numberCoordinates);
    bool areNewCoordinatesAvailable();
    void applySuperpose3d();
    void applyCoherentPointDrift();

    Superpose3D<double, double **> superposer;
    
    vector <Hokuyo *> hokuyos;
    Bounds bounds;
    int numberCoordinates;
};

#endif /* Sensors_hpp */
