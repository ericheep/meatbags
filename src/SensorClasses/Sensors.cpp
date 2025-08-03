//
//  Sensors.cpp
//  meatbags

#include "Sensors.hpp"

Sensors::Sensors() {}

Sensors::~Sensors() {}

void Sensors::update() {
    lastFrameTime = ofGetLastFrameTime();
    for (auto &sensor : sensors) {
        sensor->lastFrameTime = lastFrameTime;
        sensor->update();
    }
}

void Sensors::setTranslation(ofPoint _translation) {
    translation = _translation;
}

void Sensors::setSpace(Space & _space) {
    space = _space;
    scale = space.width / (space.areaSize * 1000);
}

void Sensors::setLocalIpAddress(string & localIPAddress) {
    for (auto &sensor : sensors) {
        sensor->setLocalIPAddress(localIPAddress);
    }
}

void Sensors::setInterfaceAndIP(string interface, string localIP) {
    for (auto &sensor : sensors) {
        sensor->setInterfaceAndIP(interface, localIP);
    }
}

void Sensors::setInfoPositions(float x, float y) {
    for (auto &sensor : sensors) {
        sensor->setInfoPosition(x, y);
    }
}

void Sensors::addSensor(Sensor* sensor) {
    sensors.push_back(sensor);
}

void Sensors::removeSensor() {
    sensors.pop_back();
}

void Sensors::closeSensors() {
    for (auto& sensor : sensors) {
        sensor->close();
    }
}

void Sensors::setFilters(Filters & _filters) {
    filters = _filters;
}

bool Sensors::areNewCoordinatesAvailable() {
    bool newCoordinatesAvalable = false;
    for (auto& sensor : sensors) {
        if (sensor->newCoordinatesAvailable) {
            newCoordinatesAvalable = true;
            sensor->newCoordinatesAvailable = false;
            break;
        }
    }
    return newCoordinatesAvalable;
}

bool Sensors::checkWithinFilters(float x, float y) {
    bool isWithinFilter = false;
    for (const auto &filter : filters.filters) {
        if (filter->polyline.inside(x * 0.001, y * 0.001)) {
            if (filter->mask) {
                return false;
            } else {
                isWithinFilter = true;
            }
        }
    }
    
    return isWithinFilter;
}

void Sensors::getCoordinates(MeatbagsFactory& meatbags) {
    for (auto& meatbag : meatbags.meatbags) {
        int counter = 0;

        for (auto& sensor : sensors) {
            if (sensor->whichMeatbag == meatbag->index) {
                for (auto& coordinate : sensor->coordinates) {
                    float x = coordinate.x;
                    float y = coordinate.y;
                        
                    if (coordinate.x != 0 && coordinate.y != 0) {
                        if (checkWithinFilters(x, y)) {
                            meatbag->coordinates[counter].set(x, y);
                            counter++;
                        }
                    }
                }
                
                sensor->newCoordinatesAvailable = false;
            }
        }
        
        meatbag->numberCoordinates = counter;
    }
}

void Sensors::getCoordinatesAndIntensities(MeatbagsFactory& meatbags) {
    for (auto& meatbag : meatbags.meatbags) {
        int counter = 0;

        for (auto& sensor : sensors) {
            if (sensor->newCoordinatesAvailable && (sensor->whichMeatbag == meatbag->index)) {
                int intensityIndex = 0;
                    
                for (auto& coordinate : sensor->coordinates) {
                    float x = coordinate.x;
                    float y = coordinate.y;
                        
                    if (coordinate.x != 0 && coordinate.y != 0) {
                        if (checkWithinFilters(x, y)) {
                            meatbag->coordinates[counter].set(x, y);
                            meatbag->intensities[counter] = sensor->intensities[intensityIndex];
                            counter++;
                        }
                    }
                        
                    intensityIndex++;
                }
                
                sensor->newCoordinatesAvailable = false;
            }
        }
        
        meatbag->numberCoordinates = counter;
    }
}

void Sensors::getCoordinatesAndIntensities(vector<ofPoint>& coordinates, vector <int>& intensities, int &numberCoordinates) {
    int counter = 0;
    
    for (auto& sensor : sensors) {
        int intensityIndex = 0;
        for (auto& coordinate : sensor->coordinates) {
            float x = coordinate.x;
            float y = coordinate.y;
            
            if (coordinate.x != 0 && coordinate.y != 0) {
                if (checkWithinFilters(x, y)) {
                    coordinates[counter].set(x, y);
                    intensities[counter] = sensor->intensities[intensityIndex];
                    counter++;
                }
            }
            
            intensityIndex++;
        }
    }

    numberCoordinates = counter;
}

ofPoint Sensors::convertCoordinateToScreenPoint(ofPoint coordinate) {
    return coordinate * scale + space.origin + translation;
}

ofPoint Sensors::convertScreenPointToCoordinate(ofPoint screenPoint) {
    return (screenPoint - space.origin - translation) / scale * 0.001;
}

void Sensors::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < sensors.size(); i++) {
        ofPoint screenPoint = convertCoordinateToScreenPoint(sensors[i]->position);
        float distance = mousePoint.distance(screenPoint);

        if(distance <= sensors[i]->position.halfSize) {
            sensors[i]->position.isMouseOver = true;
        } else {
            sensors[i]->position.isMouseOver = false;
        }

        float noseX = cos(sensors[i]->sensorRotationRad - HALF_PI);
        float noseY = sin(sensors[i]->sensorRotationRad - HALF_PI);
        
        ofPoint offsetPoint = ofPoint(noseX, noseY) * sensors[i]->noseRadius;
        ofPoint nosePoint = screenPoint - offsetPoint;

        if (mousePoint.distance(nosePoint) < sensors[i]->nosePosition.halfSize) {
            sensors[i]->nosePosition.isMouseOver = true;
        } else {
            sensors[i]->nosePosition.isMouseOver = false;
        }
    }
}

void Sensors::onMousePressed(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);

    for(int i = 0; i < sensors.size(); i++) {
        if (sensors[i]->position.isMouseOver) {
            sensors[i]->position.isMouseClicked = true;
        } else {
            sensors[i]->position.isMouseClicked = false;
        }
        
        if (sensors[i]->nosePosition.isMouseOver) {
            sensors[i]->nosePosition.isMouseClicked = true;
        } else {
            sensors[i]->nosePosition.isMouseClicked = false;
        }
    }
}

void Sensors::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < sensors.size(); i++) {
        if (sensors[i]->position.isMouseClicked) {
            
            ofPoint coordinate = convertScreenPointToCoordinate(mousePoint);
            sensors[i]->positionX = coordinate.x;
            sensors[i]->positionY = coordinate.y;
        }
        
        if (sensors[i]->nosePosition.isMouseClicked) {
            ofPoint screenPoint = convertCoordinateToScreenPoint(sensors[i]->position);
            float angle = atan2(screenPoint.y - mousePoint.y, screenPoint.x - mousePoint.x);
            sensors[i]->sensorRotationDeg = (angle + HALF_PI) * 180.0 / PI;
        }
    }
}

void Sensors::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    for(int i = 0; i < sensors.size(); i++) {
        if (sensors[i]->position.isMouseClicked) {
            sensors[i]->position.isMouseClicked = false;
        }
    }
}
