//
//  MeatbagsManager.hpp
//  meatbags
//

#ifndef MeatbagsManager_hpp
#define MeatbagsManager_hpp

#include <stdio.h>
#include "ofxGui.h"
#include "Meatbags.hpp"
#include "Blob.hpp"
#include "memory.h"

class MeatbagsManager {
public:
    MeatbagsManager();

    void draw();
    void update();
    void updateBlobs();
    void getBlobs(vector<Blob>& blobs);
    void setMaxCoordinateSize(int maxCoordinateSize);
    void addMeatbags(Meatbags* meatbag);
    void removeMeatbag();
    void addMeatbags();
    vector<Meatbags*> getMeatbags();
    
    void initialize();
    void load(ofJson configuration);
    void loadMeatbags(int numberMeatbags, ofJson config);
    void saveTo(ofJson& configuration);
    
    void refreshGUIPositions();
private:
    struct MeatbagsEntry {
        std::unique_ptr<Meatbags> meatbags;
        std::unique_ptr<ofxPanel> gui;
    };
    
    vector<MeatbagsEntry> meatbagsEntries;
    std::unique_ptr<ofxPanel> createGUIForMeatbags(Meatbags * meatbags);
};

#endif /* MeatbagsManager_hpp */
