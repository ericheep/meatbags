//
//  MeatbagsManager.hpp
//  meatbags

#ifndef MeatbagsManager_hpp
#define MeatbagsManager_hpp

#include "ofMain.h"
#include "Meatbags.hpp"
#include "Blob.hpp"
#include <vector>
#include <memory>
#include <string>

class MeatbagsManager {
public:
	MeatbagsManager();

	void update();
	void updateBlobs();
	void getBlobs(std::vector<Blob>& blobs);
	void setMaxCoordinateSize(int maxCoordinateSize);
	void addMeatbags();
	void removeMeatbag();
	std::vector<Meatbags*> getMeatbags();

	void initialize();
	void load(ofJson configuration);
	void loadMeatbags(int numberMeatbags, ofJson& config);
	void saveTo(ofJson& configuration);

private:
	struct MeatbagsEntry {
		std::unique_ptr<Meatbags> meatbags;
	};

	std::vector<MeatbagsEntry> meatbagsEntries;
};

#endif /* MeatbagsManager_hpp */
