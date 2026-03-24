//
//  DBSCANClusterer.hpp
//  meatbags

#ifndef DBSCANClusterer_hpp
#define DBSCANClusterer_hpp

#include "Clusterer.hpp"
#include "dbscan.hpp"  // your existing dbscan header

class DBSCANClusterer : public Clusterer {
public:
	float epsilon   = 150.0f;
	int   minPoints = 3;

	DBSCANClusterer() = default;
	DBSCANClusterer(float epsilon, int minPoints)
		: epsilon(epsilon), minPoints(minPoints) {}

	void setParameters(float _epsilon, int _minPoints) override {
		epsilon   = _epsilon;
		minPoints = _minPoints;
	}

	std::vector<std::vector<int>> cluster(const std::vector<point2>& points) override {
		auto rawClusters = dbscan(points, epsilon, minPoints);

		std::vector<std::vector<int>> result;
		result.reserve(rawClusters.size());
		for (auto& cluster : rawClusters) {
			std::vector<int> converted(cluster.begin(), cluster.end());
			result.push_back(std::move(converted));
		}
		return result;
	}

	std::string getName() override { return "DBSCAN"; }
};

#endif /* DBSCANClusterer_hpp */
