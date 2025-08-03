//
//  Filter.hpp
//

#ifndef Filter_hpp
#define Filter_hpp

#include <stdio.h>
#include "ofxOpenCv.h"
#include "ofMain.h"
#include "DraggablePoint.hpp"
#include "Space.h"
#include "Blob.hpp"

class Filter {
public:
    Filter();
    ~Filter();
        
    void update();
    void updateHomography();
    void checkBlobs(vector<Blob> & blobs);
    void setNumberPoints(int numberPoints);
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

    vector<ofParameter<ofVec2f>> points;
    vector<DraggablePoint> positions;
    vector<Blob> filterBlobs;
    
    vector<cv::Point2f> quad;
    vector<cv::Point2f> normalizedQuad;
    
    DraggablePoint centroid;
    ofParameter<bool> mask;
    ofParameter<bool> isActive;
    ofParameter<bool> normalize;

    float scale;
    int numberPoints;
    int index;
    bool isBlobInside;
    float distanceOfClosestBlob;
    cv::Mat homography;
    
    ofPoint translation;
    Space space;
    ofPolyline polyline;

};

#endif /* Filter_hpp */
