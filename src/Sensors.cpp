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

void Sensors::applyCoherentPointDrift() {
    //cpd::Matrix fixed = cpd::Matrix::Random(1440, 2);
    //cpd::Matrix moving = cpd::Matrix::Random(1440, 2);
    //cpd::RigidResult result = cpd::rigid(fixed, moving);

    // superposer.SetNumPoints(1440);
    // pointSetRegistration();
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
    
    cout << superposer.T[0] << " " << superposer.T[1] << " " << superposer.T[2] << endl;
    cout << superposer.R[0][0] << " " << superposer.R[0][1] << " " << superposer.R[0][2] << endl;
    cout << superposer.R[1][0] << " " << superposer.R[1][1] << " " << superposer.R[1][2] << endl;
    cout << superposer.R[2][0] << " " << superposer.R[2][1] << " " << superposer.R[2][2] << endl;

    Dealloc2D(&target);
    Dealloc2D(&source);
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
