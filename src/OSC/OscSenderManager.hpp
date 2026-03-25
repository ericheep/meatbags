//
//  OscSenderManager.hpp
//  meatbags

#ifndef OscSenderManager_hpp
#define OscSenderManager_hpp

#include <stdio.h>
#include "ofMain.h"
#include "OscSender.hpp"
#include "Blob.hpp"
#include "Sensor.hpp"
#include "Filter.hpp"

class OscSenderManager {
public:
	OscSenderManager();

	void addOscSender();
	void removeOscSender();
	void send(vector<Blob>& blobs, const vector<Sensor*> sensors, const vector<Filter*>& filters);

	vector<OscSender*> getOscSenders();

	void initialize();
	void load(ofJson configuration);
	void loadOscSenders(int numberOscSenders, ofJson& config);
	void saveTo(ofJson& configuration);

private:
	struct OscSenderEntry {
		std::unique_ptr<OscSender> oscSender;
	};

	vector<OscSenderEntry> oscSenderEntries;
};

#endif /* OscSenderManager_hpp */
