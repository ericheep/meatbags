//
//  MeatbagsFactory.hpp
//  meatbags
//

#ifndef MeatbagsFactory_hpp
#define MeatbagsFactory_hpp

#include <stdio.h>
#include "Meatbags.hpp"
#include "Blob.hpp"

class MeatbagsFactory {
public:
    MeatbagsFactory();
    
    void update();
    void updateBlobs();
    void getBlobs(vector<Blob>& blobs);
    void setMaxCoordinateSize(int maxCoordinateSize);
    void addMeatbag(Meatbags* meatbag);
    void removeMeatbag();
    
    vector <Meatbags *> meatbags;
};

#endif /* MeatbagsFactory_hpp */
