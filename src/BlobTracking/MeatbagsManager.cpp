//
//  MeatbagsManager.cpp
//  meatbags
//

#include "MeatbagsManager.hpp"
#include "DBSCANClusterer.hpp"
#include "EuclideanClusterer.hpp"

MeatbagsManager::MeatbagsManager() {
}

void MeatbagsManager::setMaxCoordinateSize(int maxCoordinateSize) {
	for (auto& entry : meatbagsEntries) {
		entry.meatbags->setMaxCoordinateSize(maxCoordinateSize);
	}
}

void MeatbagsManager::draw() {
	for (auto& entry : meatbagsEntries) {
		entry.gui->draw();
	}
}

void MeatbagsManager::update() {
	for (auto& entry : meatbagsEntries) {
		entry.meatbags->update();
	}

	// deferred clusterer type swap - same pattern as FilterManager
	for (int i = 0; i < meatbagsEntries.size(); ++i) {
		auto& entry = meatbagsEntries[i];
		if (!entry.nextClustererType.empty()) {
			std::string type = entry.nextClustererType;
			entry.nextClustererType.clear();
			changeClustererType(i, type);
		}
	}
}

void MeatbagsManager::updateBlobs() {
	for (auto& entry : meatbagsEntries) {
		entry.meatbags->updateBlobs();
	}
}

vector<Meatbags*> MeatbagsManager::getMeatbags() {
	vector<Meatbags*> meatbags;
	for (auto& entry : meatbagsEntries) {
		meatbags.push_back(entry.meatbags.get());
	}
	return meatbags;
}

void MeatbagsManager::getBlobs(vector<Blob>& blobs) {
	blobs.clear();

	for (auto& entry : meatbagsEntries) {
		for (auto& oldBlob : entry.meatbags->oldBlobs) {
			oldBlob.whichMeatbag = entry.meatbags->index;
			blobs.push_back(oldBlob);
		}
	}
}

void MeatbagsManager::removeMeatbag() {
	if (!meatbagsEntries.empty()) {
		meatbagsEntries.back().meatbags->clustererTypes.removeListener(this, &MeatbagsManager::onClustererTypeChanged);
		meatbagsEntries.pop_back();
	}
}

void MeatbagsManager::changeClustererType(int index, const std::string& type) {
	if (index < 0 || index >= meatbagsEntries.size()) return;

	auto& entry = meatbagsEntries[index];
	string currentType = entry.meatbags->getClustererName();
	if (currentType == type) return;

	if (type == "DBSCAN") {
		entry.meatbags->setClusterer(std::make_unique<DBSCANClusterer>());
	} else if (type == "Euclidean") {
		entry.meatbags->setClusterer(std::make_unique<EuclideanClusterer>());
	}

	// sync the radio to reflect the new state
	entry.meatbags->clustererTypes.selectedValue = type;
}

void MeatbagsManager::onClustererTypeChanged(string& selectedType) {
	for (int i = 0; i < meatbagsEntries.size(); ++i) {
		auto& entry = meatbagsEntries[i];
		if (!entry.meatbags) continue;

		string dropdownValue = entry.meatbags->clustererTypes.selectedValue;
		string currentType   = entry.meatbags->getClustererName();

		if (dropdownValue == selectedType && currentType != selectedType) {
			entry.nextClustererType = selectedType;
			break;
		}
	}
}

std::unique_ptr<ofxPanel> MeatbagsManager::createGUIForMeatbags(Meatbags* meatbags) {
	ofColor guiBarColor;
	guiBarColor = ofColor::grey;
	guiBarColor.a = 100;
	ofxGuiSetFillColor(guiBarColor);

	auto gui = std::make_unique<ofxPanel>();

	ofColor filterColor = ofColor::thistle;
	ofColor backgroundColor = ofColor::snow;
	backgroundColor.a = 210;

	gui->setDefaultBackgroundColor(backgroundColor);
	gui->setBackgroundColor(backgroundColor);
	gui->setHeaderBackgroundColor(filterColor);
	gui->setFillColor(guiBarColor);
	gui->setDefaultFillColor(guiBarColor);

	gui->setup("meatbags " + to_string(meatbags->index));

	gui->add(meatbags->epsilon.set("cluster epsilon (mm)", 100, 1, 1000));
	gui->add(meatbags->minPoints.set("cluster min points", 10, 1, 150));
	gui->add(meatbags->blobPersistence.set("blob persistence (s)", 0.25, 0.0, 3.0));

	// initialize dropdown here so it picks up the correct ofxGui default width/colors
	meatbags->clustererTypes.setName("algorithm");
	meatbags->clustererTypes.add("DBSCAN");
	meatbags->clustererTypes.add("Euclidean");
	meatbags->clustererTypes.disableMultipleSelection();
	meatbags->clustererTypes.setFillColor(guiBarColor);
	meatbags->clustererTypes.setBackgroundColor(backgroundColor);
	meatbags->clustererTypes.setTextColor(ofColor::black);
	meatbags->clustererTypes.addListener(this, &MeatbagsManager::onClustererTypeChanged);
	gui->add(&meatbags->clustererTypes);

	return gui;
}

void MeatbagsManager::refreshGUIPositions() {
	int yOffset = 202;
	int nextYPos = 0;
	int margin = 10;

	for (int i = 0; i < meatbagsEntries.size(); ++i) {
		auto& entry = meatbagsEntries[i];

		int xPos = margin;
		int yPos = nextYPos + yOffset;

		entry.gui->setPosition(xPos, yPos);
		nextYPos += entry.gui->getHeight() + margin;
	}
}

void MeatbagsManager::addMeatbags() {
	auto meatbags = std::make_unique<Meatbags>();
	if (meatbags) meatbags->index = meatbagsEntries.size() + 1;

	int guiWidth = 200;
	ofxGuiSetDefaultWidth(guiWidth);

	auto gui = createGUIForMeatbags(meatbags.get());
	meatbagsEntries.push_back({std::move(meatbags), std::move(gui)});

	refreshGUIPositions();
}

void MeatbagsManager::initialize() {
	addMeatbags();
	addMeatbags();
}

void MeatbagsManager::loadMeatbags(int numberMeatbags, ofJson config) {
	for (int i = 0; i < numberMeatbags; i++) {
		addMeatbags();
	}
	for (int i = 0; i < meatbagsEntries.size(); i++) {
		string meatbagsKey = "meatbags_" + to_string(i + 1);
		ofJson meatbagsConfig;
		meatbagsConfig[meatbagsKey] = config[meatbagsKey];
		meatbagsEntries[i].gui->loadFrom(meatbagsConfig);
	}
}

void MeatbagsManager::load(ofJson config) {
	if (config.contains("number_meatbags")) {
		int numberMeatbags = config["number_meatbags"];
		loadMeatbags(numberMeatbags, config);
	} else {
		addMeatbags();
		addMeatbags();
	}
}

void MeatbagsManager::saveTo(ofJson& config) {
	config["number_meatbags"] = meatbagsEntries.size();
	for (auto& entry : meatbagsEntries) {
		entry.gui->saveTo(config);
	}
}
