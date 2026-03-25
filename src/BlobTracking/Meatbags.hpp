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

class Meatbags {
public:
	Meatbags();

	void update();
	void updateBlobs();
	void getBlobs(vector<Blob>& blobs);
	void setMaxCoordinateSize(int maxCoordinateSize);

	void setClusterer(std::unique_ptr<Clusterer> c);
	string getClustererName();
	Clusterer* getClusterer() { return clusterer.get(); }

	int index;

	vector<ofPoint> coordinates;
	vector<int>     intensities;
	int             numberCoordinates;

	ofParameter<float> epsilon        = { "epsilon", 150.0f, 1.0f, 1000.0f };
	ofParameter<int>   minPoints      = { "min points", 3, 1, 50 };
	ofParameter<float> blobPersistence = { "blob persistence", 0.5f, 0.0f, 5.0f };

	vector<Blob> oldBlobs;

private:
	void  clusterBlobs();
	void  matchBlobs();
	void  addBlobs();
	void  renewBlobs();
	int   findFreeBlobIndex();
	float compareBlobs(Blob newBlob, Blob oldBlob);

	void setBlobPersistence(float& _blobPersistence);

	vector<Blob>          newBlobs;
	std::unique_ptr<Clusterer> clusterer;
	double                lastFrameTime;
};

#endif /* Meatbags_hpp */
