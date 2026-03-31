//
//  Meatbags.cpp
//  meatbags

#include "Meatbags.hpp"

Meatbags::Meatbags() {
	coordinates.resize(21600);
	intensities.resize(21600);

	numberCoordinates = 0;
	lastFrameTime     = 0;
	index             = 0;

	clusterer = std::make_unique<EuclideanClusterer>(epsilon, minPoints);

	blobPersistence.addListener(this, &Meatbags::setBlobPersistence);
}

void Meatbags::update() {
	lastFrameTime = ofGetLastFrameTime();

	for (auto& oldBlob : oldBlobs) {
		oldBlob.updateLifetime(lastFrameTime);
	}

	oldBlobs.erase(std::remove_if(oldBlobs.begin(), oldBlobs.end(), [](Blob& blob) {
		return !blob.isAlive();
	}), oldBlobs.end());
}

void Meatbags::setMaxCoordinateSize(int maxCoordinateSize) {
	coordinates.resize(maxCoordinateSize);
	intensities.resize(maxCoordinateSize);
}

void Meatbags::setClusterer(std::unique_ptr<Clusterer> c) {
	clusterer = std::move(c);
}

std::string Meatbags::getClustererName() {
	if (clusterer) return clusterer->getName();
	return "none";
}

void Meatbags::updateBlobs() {
	if (numberCoordinates == 0) return;

	clusterBlobs();
	matchBlobs();
	renewBlobs();
	addBlobs();
}

void Meatbags::clusterBlobs() {
	std::vector<point2> points;
	points.reserve(numberCoordinates);
	for (int i = 0; i < numberCoordinates; i++) {
		point2 p;
		p.x = coordinates[i].x;
		p.y = coordinates[i].y;
		points.push_back(p);
	}

	clusterer->setParameters(epsilon, minPoints);
	auto clusters = clusterer->cluster(points);

	newBlobs.clear();
	int idx = 0;
	for (auto& cluster : clusters) {
		std::vector<ofPoint> clusterCoordinates;
		std::vector<int>     clusterIntensities;
		int numberPoints = cluster.size();

		for (int i = 0; i < numberPoints; i++) {
			int coordinateIndex = cluster[i];
			clusterCoordinates.push_back(coordinates[coordinateIndex]);
			clusterIntensities.push_back(intensities[coordinateIndex]);
		}

		Blob newBlob = Blob(clusterCoordinates, clusterIntensities, blobPersistence, numberPoints);
		newBlob.index = idx;
		newBlobs.push_back(newBlob);
		idx++;
	}

	if (oldBlobs.empty()) {
		int i = 0;
		for (auto& newBlob : newBlobs) {
			Blob blob = Blob();
			blob.become(newBlob);
			blob.index = i++;
			oldBlobs.push_back(blob);
		}
	}

	for (auto& oldBlob : oldBlobs) {
		oldBlob.setMatched(false);
	}
}

float Meatbags::compareBlobs(Blob newBlob, Blob oldBlob) {
	float eps = 1.0f;
	float squareDistance = newBlob.centroid.squareDistance(oldBlob.centroid);
	return 1.0f / (squareDistance + eps);
}

void Meatbags::matchBlobs() {
	for (auto& newBlob : newBlobs) {
		int   potentialMatchIndex  = 0;
		float potentialHighestScore = 0.0f;

		for (auto& oldBlob : oldBlobs) {
			float score = compareBlobs(newBlob, oldBlob);
			if (score > potentialHighestScore) {
				potentialMatchIndex   = oldBlob.index;
				potentialHighestScore = score;
			}
		}

		newBlob.setPotentialMatch(potentialMatchIndex, potentialHighestScore);
	}

	std::sort(newBlobs.begin(), newBlobs.end(), [](const Blob& a, const Blob& b) {
		return a.potentialMatchScore > b.potentialMatchScore;
	});

	for (auto& newBlob : newBlobs) {
		for (auto& oldBlob : oldBlobs) {
			if (oldBlob.index == newBlob.potentialMatchIndex &&
				!newBlob.isMatched() && !oldBlob.isMatched()) {

				// compute EMA velocity before become() overwrites centroid
				ofPoint prevCentroid = oldBlob.centroid;
				ofPoint prevVelocity = oldBlob.velocity;

				oldBlob.become(newBlob);

				if (lastFrameTime > 0.0) {
					ofPoint rawVelocity = (oldBlob.centroid - prevCentroid) * (0.001f / (float)lastFrameTime);
					float alpha = ofClamp(velocitySmoothing.get(), 0.01f, 1.0f);
					oldBlob.velocity = prevVelocity * (1.0f - alpha) + rawVelocity * alpha;
				}

				newBlob.setMatched(true);
				oldBlob.setMatched(true);
			}
		}
	}
}

void Meatbags::addBlobs() {
	for (auto& newBlob : newBlobs) {
		if (!newBlob.isMatched()) {
			Blob blob;
			blob.index = findFreeBlobIndex();
			blob.setMatched(true);
			blob.become(newBlob);
			oldBlobs.push_back(blob);
		}
	}
}

void Meatbags::renewBlobs() {
	for (auto& oldBlob : oldBlobs) {
		if (oldBlob.isMatched()) {
			oldBlob.lifetime = 0;
		}
	}
}

int Meatbags::findFreeBlobIndex() {
	int  freeIndex           = 0;
	bool lookingForFreeIndex = true;

	while (lookingForFreeIndex) {
		lookingForFreeIndex = false;
		for (Blob& oldBlob : oldBlobs) {
			if (freeIndex == oldBlob.index) lookingForFreeIndex = true;
		}
		if (lookingForFreeIndex) freeIndex++;
	}

	return freeIndex;
}

void Meatbags::getBlobs(std::vector<Blob>& blobs) {
	blobs.clear();
	for (auto& oldBlob : oldBlobs) {
		blobs.push_back(oldBlob);
	}
}

void Meatbags::setBlobPersistence(float& _blobPersistence) {
	blobPersistence = _blobPersistence;
	for (auto& oldBlob : oldBlobs) {
		oldBlob.lifetimeLength = blobPersistence;
	}
}
