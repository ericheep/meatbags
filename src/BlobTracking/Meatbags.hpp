//
//  Meatbags.hpp
//  meatbags

#ifndef Meatbags_hpp
#define Meatbags_hpp

#include "ofMain.h"
#include "Blob.hpp"
#include "Clusterer.hpp"
#include "EuclideanClusterer.hpp"
#include "DBSCANClusterer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

class Meatbags {
public:
	Meatbags();

	void update();
	void updateBlobs();
	void getBlobs(std::vector<Blob>& blobs);
	void setMaxCoordinateSize(int maxCoordinateSize);

	void setClusterer(std::unique_ptr<Clusterer> c);
	std::string getClustererName();
	Clusterer* getClusterer() { return clusterer.get(); }

	int index;

	std::vector<ofPoint> coordinates;
	std::vector<int>     intensities;
	int                  numberCoordinates;

	ofParameter<float> epsilon           = { "epsilon",        150.0f, 1.0f,  1000.0f };
	ofParameter<int>   minPoints         = { "min points",     3,      1,     50      };
	ofParameter<float> blobPersistence   = { "blob persistence", 0.5f, 0.0f,  5.0f   };
	ofParameter<float> velocitySmoothing = { "vel smoothing",  0.2f,   0.01f, 1.0f   };

	std::vector<Blob> oldBlobs;

private:
	void  clusterBlobs();
	void  matchBlobs();
	void  addBlobs();
	void  renewBlobs();
	int   findFreeBlobIndex();
	float compareBlobs(Blob newBlob, Blob oldBlob);

	void setBlobPersistence(float& _blobPersistence);

	std::vector<Blob>          newBlobs;
	std::unique_ptr<Clusterer> clusterer;
	double                     lastFrameTime;
};

#endif /* Meatbags_hpp */
