//
//  Sensors.cpp
//

#include "Sensors.hpp"

Sensors::Sensors() {
    movingCoordinates.resize(1440);
    fixedCoordinates.resize(1440);
    rigid.normalize(false);
    
    ofAddListener(ofEvents().mouseMoved, this, &Sensors::onMouseMoved);
    ofAddListener(ofEvents().mousePressed, this, &Sensors::onMousePressed);
    ofAddListener(ofEvents().mouseDragged, this, &Sensors::onMouseDragged);
    ofAddListener(ofEvents().mouseReleased, this, &Sensors::onMouseReleased);
}

void Sensors::update() {
    for (auto& hokuyo : hokuyos) {
        hokuyo->update();
        
        if (hokuyo->alignRequested) {
            applyCoherentPointDrift();
            hokuyo->alignRequested = false;
        }
    }
    
}

void Sensors::setSpace(Space & _space) {
    space = _space;
    scale = space.width / (space.areaSize * 1000);
}

void Sensors::applyCoherentPointDrift() {
    cpd::Matrix fixed(1440, 2);
    cpd::Matrix moving(1440, 2);
    
    for (int i = 0; i < 1440; i++) {
        fixed(i, 0) = (double) hokuyos[0]->coordinates[i].x;
        fixed(i, 1) = (double) hokuyos[0]->coordinates[i].y;
        moving(i, 0) = (double) hokuyos[1]->coordinates[i].x;
        moving(i, 1) = (double) hokuyos[1]->coordinates[i].y;
    }

    cpd::RigidResult result = rigid.run(fixed, moving);
    
    if (result.translation.size() == 2) {
        float x = result.translation(0);
        float y = result.translation(1);
        cout << "! " << x << " " << y << endl;

        float addx = hokuyos[1]->position.x + x;
        float addy = hokuyos[1]->position.y + y;
        cout << "add " << addx << " " << addy << endl;

        hokuyos[1]->positionX = addx * 0.001;
        hokuyos[1]->positionY = addy * 0.001;
    }
}

void Sensors::applySuperpose3d() {
    double **target;
    double **source;
    
    Alloc2D(1440, 2, &target);
    Alloc2D(1440, 2, &source);
    
    for (int i = 0; i < 1440; i++) {
        target[i][0] = hokuyos[0]->coordinates[i].x;
        target[i][1] = hokuyos[0]->coordinates[i].y;
        
        source[i][0] = hokuyos[1]->coordinates[i].x;
        source[i][1] = hokuyos[1]->coordinates[i].y;
    }
    
    double rmsd;
    bool allowRescale = false;
    rmsd = superposer.Superpose(target, source, allowRescale);

    Dealloc2D(&target);
    Dealloc2D(&source);
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

void Sensors::setBounds(Bounds& _bounds) {
    bounds = _bounds;
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

ofPoint Sensors::convertCoordinateToScreenPoint(ofPoint coordinate) {
    return coordinate * scale + space.origin;
}

ofPoint Sensors::convertScreenPointToCoordinate(ofPoint screenPoint) {
    return (screenPoint - space.origin) / scale * 0.001;
}

void Sensors::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < hokuyos.size(); i++) {
        ofPoint screenPoint = convertCoordinateToScreenPoint(hokuyos[i]->position);
        float distance = mousePoint.distance(screenPoint);

        if(distance <= hokuyos[i]->mouseBoxHalfSize) {
            hokuyos[i]->isMouseOver = true;
        } else {
            hokuyos[i]->isMouseOver = false;
        }
        hokuyos[i]->isMouseOverNose = true;

        float noseX = cos(hokuyos[i]->sensorRotationRad - HALF_PI);
        float noseY = sin(hokuyos[i]->sensorRotationRad - HALF_PI);
        
        ofPoint offsetPoint = ofPoint(noseX, noseY) * hokuyos[i]->mouseNoseBoxRadius;
        ofPoint nosePoint = screenPoint - offsetPoint;

        if (mousePoint.distance(nosePoint) < hokuyos[i]->mouseNoseBoxHalfSize) {
            hokuyos[i]->isMouseOverNose = true;
        } else {
            hokuyos[i]->isMouseOverNose = false;
        }
    }
}

void Sensors::onMousePressed(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);

    for(int i = 0; i < hokuyos.size(); i++) {
        if (hokuyos[i]->isMouseOver) {
            hokuyos[i]->isMouseClicked = true;
        } else {
            hokuyos[i]->isMouseClicked = false;
        }
        
        if (hokuyos[i]->isMouseOverNose) {
            hokuyos[i]->isMouseOverNoseClicked = true;
        } else {
            hokuyos[i]->isMouseOverNoseClicked = false;
        }
    }
}

void Sensors::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < hokuyos.size(); i++) {
        if (hokuyos[i]->isMouseClicked) {
            ofPoint coordinate = convertScreenPointToCoordinate(mousePoint);
            hokuyos[i]->positionX = coordinate.x;
            hokuyos[i]->positionY = coordinate.y;
        }
        
        if (hokuyos[i]->isMouseOverNoseClicked) {
            ofPoint screenPoint = convertCoordinateToScreenPoint(hokuyos[i]->position);
            float angle = atan2(screenPoint.y - mousePoint.y, screenPoint.x - mousePoint.x);
            hokuyos[i]->sensorRotationDeg = (angle + HALF_PI) * 180.0 / PI;
        }
    }
}

void Sensors::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    
    for(int i = 0; i < hokuyos.size(); i++) {
        if (hokuyos[i]->isMouseClicked) {
            hokuyos[i]->isMouseClicked = false;
        }
    }
}
