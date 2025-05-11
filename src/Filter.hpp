//
//  Filter.hpp
//

#ifndef Filter_hpp
#define Filter_hpp

#include <stdio.h>
#include "ofMain.h"
#include "DraggablePoint.hpp"
#include "Space.h"
#include "Blob.hpp"

class Filter {
public:
    Filter();
    ~Filter();
        
    void update();
    void checkBlobs(vector<Blob> & blobs);
    void setNumberPoints(int numberPoints);
    void setSpace(Space & space);
    void setTranslation(ofPoint translation);
    void translatePointsByCentroid(ofPoint centroid);
    
    ofPoint convertCoordinateToScreenPoint(ofPoint coordinate);
    ofPoint convertScreenPointToCoordinate(ofPoint screenPoint);

    vector<ofParameter<ofVec2f>> points;
    vector<DraggablePoint> positions;
    DraggablePoint centroid;
    ofParameter<bool> mask;

    float scale;
    int numberPoints;
    int index;
    bool isBlobInside;
    float distanceOfClosestBlob;
    
    ofPoint translation;
    Space space;
    ofPolyline polyline;
protected:
    void onMouseMoved(ofMouseEventArgs & mouseArgs);
    void onMousePressed(ofMouseEventArgs & mouseArgs);
    void onMouseDragged(ofMouseEventArgs & mouseArgs);
    void onMouseReleased(ofMouseEventArgs & mouseArgs);
    void onKeyPressed(ofKeyEventArgs & keyArgs);
};

#endif /* Filter_hpp */
