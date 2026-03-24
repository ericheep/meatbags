//
//  EuclideanClusterer.hpp
//  meatbags

#ifndef EuclideanClusterer_hpp
#define EuclideanClusterer_hpp

#include "Clusterer.hpp"
#include <unordered_map>

class EuclideanClusterer : public Clusterer {
public:
	float tolerance = 150.0f;  // max distance between points in the same cluster (mm)
	int   minPoints = 3;       // minimum points to form a cluster

	EuclideanClusterer() = default;
	EuclideanClusterer(float tolerance, int minPoints)
		: tolerance(tolerance), minPoints(minPoints) {}

	void setParameters(float _epsilon, int _minPoints) override {
		tolerance  = _epsilon;  // epsilon maps to tolerance for Euclidean
		minPoints  = _minPoints;
	}

	std::vector<std::vector<int>> cluster(const std::vector<point2>& points) override;

	std::string getName() override { return "Euclidean"; }

private:
	// spatial hash helpers
	using CellKey = std::pair<int, int>;

	struct CellKeyHash {
		size_t operator()(const CellKey& k) const {
			// combine two ints into a single hash
			size_t h1 = std::hash<int>{}(k.first);
			size_t h2 = std::hash<int>{}(k.second);
			return h1 ^ (h2 * 2654435761u);
		}
	};

	CellKey cellFor(const point2& p, float cellSize) const;
	std::vector<int> neighborsOf(int idx,
								 const std::vector<point2>& points,
								 const std::unordered_map<CellKey, std::vector<int>, CellKeyHash>& grid,
								 float cellSize) const;
};

#endif /* EuclideanClusterer_hpp */
