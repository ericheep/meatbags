//
//  MeatbagsManager.cpp
//  meatbags

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

void MeatbagsManager::update() {
	for (auto& entry : meatbagsEntries) {
		entry.meatbags->update();
	}
}

void MeatbagsManager::updateBlobs() {
	for (auto& entry : meatbagsEntries) {
		entry.meatbags->updateBlobs();
	}
}

std::vector<Meatbags*> MeatbagsManager::getMeatbags() {
	std::vector<Meatbags*> meatbags;
	for (auto& entry : meatbagsEntries) {
		meatbags.push_back(entry.meatbags.get());
	}
	return meatbags;
}

void MeatbagsManager::getBlobs(std::vector<Blob>& blobs) {
	blobs.clear();
	for (auto& entry : meatbagsEntries) {
		for (auto& oldBlob : entry.meatbags->oldBlobs) {
			oldBlob.whichMeatbag = entry.meatbags->index;
			blobs.push_back(oldBlob);
		}
	}
}

void MeatbagsManager::removeMeatbag() {
	if (meatbagsEntries.size() > 1) {
		meatbagsEntries.pop_back();
	}
}

void MeatbagsManager::addMeatbags() {
	auto meatbags = std::make_unique<Meatbags>();
	meatbags->index = meatbagsEntries.size() + 1;

	meatbags->epsilon.set("epsilon",            100,  1,    1000);
	meatbags->minPoints.set("min points",       10,   1,    150);
	meatbags->blobPersistence.set("blob persistence", 0.25f, 0.0f, 3.0f);
	meatbags->velocitySmoothing.set("vel smoothing",  0.2f,  0.01f, 1.0f);

	meatbagsEntries.push_back({ std::move(meatbags) });
}

void MeatbagsManager::initialize() {
	addMeatbags();
}

void MeatbagsManager::loadMeatbags(int numberMeatbags, ofJson& config) {
	for (int i = 0; i < numberMeatbags; i++) {
		addMeatbags();
	}
	for (int i = 0; i < (int)meatbagsEntries.size(); i++) {
		std::string key = "meatbags_" + std::to_string(i + 1);
		if (!config.contains(key)) continue;
		ofJson& m = config[key];
		auto& mb = meatbagsEntries[i].meatbags;

		if (m.contains("epsilon"))            mb->epsilon           = m["epsilon"].get<float>();
		if (m.contains("min_points"))         mb->minPoints         = m["min_points"].get<int>();
		if (m.contains("blob_persistence"))   mb->blobPersistence   = m["blob_persistence"].get<float>();
		if (m.contains("velocity_smoothing")) mb->velocitySmoothing = m["velocity_smoothing"].get<float>();
		if (m.contains("clusterer")) {
			std::string type = m["clusterer"].get<std::string>();
			if (type == "Euclidean") mb->setClusterer(std::make_unique<EuclideanClusterer>());
			else                     mb->setClusterer(std::make_unique<DBSCANClusterer>());
		}
	}
}

void MeatbagsManager::load(ofJson config) {
	if (config.contains("number_meatbags")) {
		int n = config["number_meatbags"];
		loadMeatbags(n, config);
	} else {
		addMeatbags();
	}
}

void MeatbagsManager::saveTo(ofJson& config) {
	config["number_meatbags"] = meatbagsEntries.size();
	for (int i = 0; i < (int)meatbagsEntries.size(); i++) {
		std::string key = "meatbags_" + std::to_string(i + 1);
		auto& mb = meatbagsEntries[i].meatbags;
		config[key]["epsilon"]            = mb->epsilon.get();
		config[key]["min_points"]         = mb->minPoints.get();
		config[key]["blob_persistence"]   = mb->blobPersistence.get();
		config[key]["velocity_smoothing"] = mb->velocitySmoothing.get();
		config[key]["clusterer"]          = mb->getClustererName();
	}
}
