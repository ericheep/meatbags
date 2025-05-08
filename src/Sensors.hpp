//
//  Sensors.hpp
//

#ifndef Sensors_hpp
#define Sensors_hpp

#include <stdio.h>
#include "Hokuyo.hpp"
#include "Filters.hpp"
#include "Space.h"
#include "DraggablePoint.hpp"

// #include "superpose3d/superpose3d.hpp"
// using namespace superpose3d;
// #include <cpd/rigid.hpp>
// #include <cpd/gauss_transform_fgt.hpp>

class Sensors {
public:
    Sensors();
    ~Sensors();
    
    void update();
    
    void addSensor(Hokuyo* hokuyo);
    void removeSensor();
    void closeSensors();

    void getCoordinatesAndIntensities(vector<ofPoint> &coordinates, vector <int> &intensities, int &numberCoordinates);
    bool areNewCoordinatesAvailable();
    bool checkWithinFilters(float x, float y);
    
    void setLocalIpAddress(string & localIPAddress);
    void setFilters(Filters & filters);
    void setSpace(Space & space);
    void setTranslation(ofPoint translation);
    
    ofPoint convertCoordinateToScreenPoint(ofPoint coordinate);
    ofPoint convertScreenPointToCoordinate(ofPoint screenPoint);
    
    vector <Hokuyo *> hokuyos;
    Filters filters;
    
    ofPoint translation;
    Space space;
    int numberCoordinates;
    float scale;
    
    // vector<vector<double>> fixedCoordinates;
    // vector<vector<double>> movingCoordinates;
    // void alignSensor(Hokuyo* hokuyo);
    // void applySuperpose3d();
    // void applyCoherentPointDrift();
    // Superpose3D<double, double **> superposer;
    // cpd::Rigid rigid;
    // cpd::Matrix fixed;
    // cpd::Matrix moving;
protected:
    void onMouseMoved(ofMouseEventArgs & mouseArgs);
    void onMousePressed(ofMouseEventArgs & mouseArgs);
    void onMouseDragged(ofMouseEventArgs & mouseArgs);
    void onMouseReleased(ofMouseEventArgs & mouseArgs);
};

#endif /* Sensors_hpp */
