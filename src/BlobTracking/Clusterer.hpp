//
//  Clusterer.hpp
//  meatbags

#ifndef Clusterer_hpp
#define Clusterer_hpp

#include <vector>
#include "dbscan.hpp"

class Clusterer {
public:
	virtual ~Clusterer() = default;
	virtual std::vector<std::vector<int>> cluster(const std::vector<point2>& points) = 0;
	virtual void setParameters(float epsilon, int minPoints) {}
	virtual std::string getName() = 0;
};

#endif /* Clusterer_hpp */
