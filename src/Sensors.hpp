//
//  Sensors.hpp
//

#ifndef Sensors_hpp
#define Sensors_hpp

#include <stdio.h>
#include "Hokuyo.hpp"
#include "Bounds.hpp"
#include "Space.h"

#include "superpose3d/superpose3d.hpp"
using namespace superpose3d;

#include <cpd/rigid.hpp>
#include <cpd/gauss_transform_fgt.hpp>

class Sensors {
public:
    Sensors();
    
    void update();
    
    void addSensor(Hokuyo* hokuyo);
    void alignSensor(Hokuyo* hokuyo);
    void closeSensors();

    void setBounds(Bounds& bounds);
    void getCoordinatesAndIntensities(vector<ofPoint> &coordinates, vector <int> &intensities, int &numberCoordinates);
    bool areNewCoordinatesAvailable();
    void applySuperpose3d();
    void applyCoherentPointDrift();
 
    void setSpace(Space & space);
    
    ofPoint convertCoordinateToScreenPoint(ofPoint coordinate);
    ofPoint convertScreenPointToCoordinate(ofPoint screenPoint);

    Superpose3D<double, double **> superposer;
    cpd::Rigid rigid;
    cpd::Matrix fixed;
    cpd::Matrix moving;
    vector<vector<double>> fixedCoordinates;
    vector<vector<double>> movingCoordinates;
    
    vector <Hokuyo *> hokuyos;
    Bounds bounds;
    
    Space space;
    int numberCoordinates;
    float scale;
    
protected:
    void onMouseMoved(ofMouseEventArgs & mouseArgs);
    void onMousePressed(ofMouseEventArgs & mouseArgs);
    void onMouseDragged(ofMouseEventArgs & mouseArgs);
    void onMouseReleased(ofMouseEventArgs & mouseArgs);
};

#endif /* Sensors_hpp */
