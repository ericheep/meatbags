//
//  Sensors.cpp
//

#include "Sensors.hpp"

Sensors::Sensors() {
    ofAddListener(ofEvents().mouseMoved, this, &Sensors::onMouseMoved);
    ofAddListener(ofEvents().mousePressed, this, &Sensors::onMousePressed);
    ofAddListener(ofEvents().mouseDragged, this, &Sensors::onMouseDragged);
    ofAddListener(ofEvents().mouseReleased, this, &Sensors::onMouseReleased);
}

Sensors::~Sensors() {
    ofRemoveListener(ofEvents().mouseMoved, this, &Sensors::onMouseMoved);
    ofRemoveListener(ofEvents().mousePressed, this, &Sensors::onMousePressed);
    ofRemoveListener(ofEvents().mouseDragged, this, &Sensors::onMouseDragged);
    ofRemoveListener(ofEvents().mouseReleased, this, &Sensors::onMouseReleased);
}

void Sensors::update() {
    for (auto &hokuyo : hokuyos) {
        hokuyo->update();
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
    for (auto &hokuyo : hokuyos) {
        hokuyo->setLocalIPAddress(localIPAddress);
    }
}

void Sensors::setInterfaceAndIP(string interface, string localIP) {
    for (auto &hokuyo : hokuyos) {
        hokuyo->setInterfaceAndIP(interface, localIP);
    }
}

void Sensors::setInfoPositions(float x, float y) {
    for (auto &hokuyo : hokuyos) {
        hokuyo->setInfoPosition(x, y);
    }
}

void Sensors::addSensor(Hokuyo* hokuyo) {
    hokuyos.push_back(hokuyo);
}

void Sensors::removeSensor() {
    hokuyos.pop_back();
}

void Sensors::closeSensors() {
    for (auto& hokuyo : hokuyos) {
        hokuyo->close();
    }
}

void Sensors::setFilters(Filters & _filters) {
    filters = _filters;
}

bool Sensors::areNewCoordinatesAvailable() {
    bool newCoordinatesAvalable = false;
    for (auto& hokuyo : hokuyos) {
        if (hokuyo->newCoordinatesAvailable) {
            newCoordinatesAvalable = true;
            hokuyo->newCoordinatesAvailable = false;
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

void Sensors::getCoordinatesAndIntensities(MeatbagsFactory& meatbags) {
    for (auto& meatbag : meatbags.meatbags) {
        int counter = 0;

        for (auto& sensor : hokuyos) {
            if (sensor->whichMeatbag == meatbag->index) {
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
            }
        }
        
        meatbag->numberCoordinates = counter;
    }
}

void Sensors::getCoordinatesAndIntensities(vector<ofPoint>& coordinates, vector <int>& intensities, int &numberCoordinates) {
    int counter = 0;
    
    for (auto& hokuyo : hokuyos) {
        int intensityIndex = 0;
        for (auto& coordinate : hokuyo->coordinates) {
            float x = coordinate.x;
            float y = coordinate.y;
            
            if (coordinate.x != 0 && coordinate.y != 0) {
                if (checkWithinFilters(x, y)) {
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

ofPoint Sensors::convertCoordinateToScreenPoint(ofPoint coordinate) {
    return coordinate * scale + space.origin + translation;
}

ofPoint Sensors::convertScreenPointToCoordinate(ofPoint screenPoint) {
    return (screenPoint - space.origin - translation) / scale * 0.001;
}

void Sensors::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < hokuyos.size(); i++) {
        ofPoint screenPoint = convertCoordinateToScreenPoint(hokuyos[i]->position);
        float distance = mousePoint.distance(screenPoint);

        if(distance <= hokuyos[i]->position.halfSize) {
            hokuyos[i]->position.isMouseOver = true;
        } else {
            hokuyos[i]->position.isMouseOver = false;
        }

        float noseX = cos(hokuyos[i]->sensorRotationRad - HALF_PI);
        float noseY = sin(hokuyos[i]->sensorRotationRad - HALF_PI);
        
        ofPoint offsetPoint = ofPoint(noseX, noseY) * hokuyos[i]->noseRadius;
        ofPoint nosePoint = screenPoint - offsetPoint;

        if (mousePoint.distance(nosePoint) < hokuyos[i]->nosePosition.halfSize) {
            hokuyos[i]->nosePosition.isMouseOver = true;
        } else {
            hokuyos[i]->nosePosition.isMouseOver = false;
        }
    }
}

void Sensors::onMousePressed(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);

    for(int i = 0; i < hokuyos.size(); i++) {
        if (hokuyos[i]->position.isMouseOver) {
            hokuyos[i]->position.isMouseClicked = true;
        } else {
            hokuyos[i]->position.isMouseClicked = false;
        }
        
        if (hokuyos[i]->nosePosition.isMouseOver) {
            hokuyos[i]->nosePosition.isMouseClicked = true;
        } else {
            hokuyos[i]->nosePosition.isMouseClicked = false;
        }
    }
}

void Sensors::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < hokuyos.size(); i++) {
        if (hokuyos[i]->position.isMouseClicked) {
            ofPoint coordinate = convertScreenPointToCoordinate(mousePoint);
            hokuyos[i]->positionX = coordinate.x;
            hokuyos[i]->positionY = coordinate.y;
        }
        
        if (hokuyos[i]->nosePosition.isMouseClicked) {
            ofPoint screenPoint = convertCoordinateToScreenPoint(hokuyos[i]->position);
            float angle = atan2(screenPoint.y - mousePoint.y, screenPoint.x - mousePoint.x);
            hokuyos[i]->sensorRotationDeg = (angle + HALF_PI) * 180.0 / PI;
        }
    }
}

void Sensors::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < hokuyos.size(); i++) {
        if (hokuyos[i]->position.isMouseClicked) {
            hokuyos[i]->position.isMouseClicked = false;
        }
    }
}
