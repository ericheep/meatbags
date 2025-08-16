//
//  Filter.hpp
//

#ifndef Filter_hpp
#define Filter_hpp

#include <stdio.h>
#include "ofxOpenCv.h"
#include "ofMain.h"
#include "ofxDropdown.h"
#include "DraggablePoint.hpp"
#include "Blob.hpp"
#include "Space.h"

class Filter {
public:
    Filter();
    virtual ~Filter();
        
    virtual void update() = 0;
    virtual void reset() = 0;
    virtual void updateNormalization() = 0;
    
    void updateCentroid();
    virtual void checkBlobs(vector<Blob>& blobs);
    virtual bool checkInside(float x, float y);
    virtual ofPoint normalizeCoordinate(float x, float y) = 0;
    
    virtual void drawOutline();
    virtual void drawShape();
    
    void setPosition(vector<ofPoint> position);
    vector<ofPoint> getPosition();

    void setSpace(Space & space);
    void setTranslation(ofPoint translation);
    void translatePointsByCentroid(ofPoint centroid);
    
    bool onMouseMoved(ofMouseEventArgs & mouseArgs);
    bool onMousePressed(ofMouseEventArgs & mouseArgs);
    bool onMouseDragged(ofMouseEventArgs & mouseArgs);
    bool onMouseReleased(ofMouseEventArgs & mouseArgs);
    bool onKeyPressed(ofKeyEventArgs & keyArgs);
    
    ofPoint convertCoordinateToScreenPoint(ofPoint coordinate);
    ofPoint convertScreenPointToCoordinate(ofPoint screenPoint);

    vector<ofParameter<ofVec2f>> anchorPoints;
    vector<DraggablePoint> draggablePoints;
    vector<Blob> filterBlobs;
    ofPolyline polyline;
    
    DraggablePoint centroid;
    ofxDropdown_<string> filterTypes { "shape" };
    ofParameter<bool> isMask;
    ofParameter<bool> isActive;
    ofParameter<bool> isNormalized;

    float scale;
    int index;
    bool isBlobInside;
    float distanceOfClosestBlob;
    int numberAnchorPoints = 4;
    
    ofPoint translation;
    Space space;
};

#endif /* Filter_hpp */
