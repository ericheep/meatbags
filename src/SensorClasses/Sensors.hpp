//
//  Sensors.hpp
//  meatbags

#ifndef Sensors_hpp
#define Sensors_hpp

#include <stdio.h>
#include "Hokuyo.hpp"
#include "Filters.hpp"
#include "Space.h"
#include "DraggablePoint.hpp"
#include "MeatbagsFactory.hpp"

class Sensors {
public:
    Sensors();
    ~Sensors();
    
    void update();
    
    void addSensor(Sensor* sensor);
    void removeSensor();
    void closeSensors();

    void getCoordinates(MeatbagsFactory& meatbags);
    void getCoordinatesAndIntensities(MeatbagsFactory& meatbags);
    void getCoordinatesAndIntensities(vector<ofPoint> &coordinates, vector <int> &intensities, int &numberCoordinates);
    bool areNewCoordinatesAvailable();
    bool checkWithinFilters(float x, float y);
    
    void setInterfaceAndIP(string interface, string IP);
    void setLocalIpAddress(string & localIPAddress);
    void setFilters(Filters & filters);
    void setSpace(Space & space);
    void setTranslation(ofPoint translation);
    void setInfoPositions(float x, float y);
    
    void onMouseMoved(ofMouseEventArgs & mouseArgs);
    void onMousePressed(ofMouseEventArgs & mouseArgs);
    void onMouseDragged(ofMouseEventArgs & mouseArgs);
    void onMouseReleased(ofMouseEventArgs & mouseArgs);
    
    ofPoint convertCoordinateToScreenPoint(ofPoint coordinate);
    ofPoint convertScreenPointToCoordinate(ofPoint screenPoint);
    
    vector <Sensor *> sensors;
    Filters filters;
    
    ofPoint translation;
    Space space;
    
    int numberCoordinates;
    float scale;
    float lastTimeRecorded, lastFrameTime;
};

#endif /* Sensors_hpp */
