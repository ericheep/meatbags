//
//  MeatbagsManager.hpp
//  meatbags
//

#ifndef MeatbagsManager_hpp
#define MeatbagsManager_hpp

#include <stdio.h>
#include "ofMain.h"
#include "Meatbags.hpp"
#include "Blob.hpp"

class MeatbagsManager {
public:
	MeatbagsManager();

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
	void loadMeatbags(int numberMeatbags, ofJson& config);
	void saveTo(ofJson& configuration);

private:
	struct MeatbagsEntry {
		std::unique_ptr<Meatbags> meatbags;
	};

	vector<MeatbagsEntry> meatbagsEntries;
};

#endif /* MeatbagsManager_hpp */
