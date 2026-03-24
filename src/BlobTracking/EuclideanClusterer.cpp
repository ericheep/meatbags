//
//  EuclideanClusterer.cpp
//  meatbags

#include "EuclideanClusterer.hpp"
#include <cmath>
#include <queue>

EuclideanClusterer::CellKey EuclideanClusterer::cellFor(const point2& p, float cellSize) const {
	int cx = (int)std::floor(p.x / cellSize);
	int cy = (int)std::floor(p.y / cellSize);
	return { cx, cy };
}

std::vector<int> EuclideanClusterer::neighborsOf(
	int idx,
	const std::vector<point2>& points,
	const std::unordered_map<CellKey, std::vector<int>, CellKeyHash>& grid,
	float cellSize) const
{
	std::vector<int> result;
	const point2& p = points[idx];
	auto [cx, cy] = cellFor(p, cellSize);

	float tolSq = tolerance * tolerance;

	// check the 3x3 neighborhood of cells around this point
	for (int dx = -1; dx <= 1; dx++) {
		for (int dy = -1; dy <= 1; dy++) {
			auto it = grid.find({ cx + dx, cy + dy });
			if (it == grid.end()) continue;

			for (int neighborIdx : it->second) {
				if (neighborIdx == idx) continue;

				const point2& q = points[neighborIdx];
				float diffX = p.x - q.x;
				float diffY = p.y - q.y;
				float distSq = diffX * diffX + diffY * diffY;

				if (distSq <= tolSq) {
					result.push_back(neighborIdx);
				}
			}
		}
	}

	return result;
}

std::vector<std::vector<int>> EuclideanClusterer::cluster(const std::vector<point2>& points) {
	int n = (int)points.size();
	if (n == 0) return {};

	// cell size = tolerance so each point only needs to check its 3x3 neighbor cells
	float cellSize = tolerance;

	// build spatial hash grid
	std::unordered_map<CellKey, std::vector<int>, CellKeyHash> grid;
	grid.reserve(n);
	for (int i = 0; i < n; i++) {
		grid[cellFor(points[i], cellSize)].push_back(i);
	}

	// flood-fill connected components
	std::vector<int> labels(n, -1);  // -1 = unvisited
	int currentLabel = 0;

	for (int i = 0; i < n; i++) {
		if (labels[i] != -1) continue;

		// BFS from this seed point
		std::queue<int> q;
		q.push(i);
		labels[i] = currentLabel;

		while (!q.empty()) {
			int current = q.front();
			q.pop();

			for (int neighbor : neighborsOf(current, points, grid, cellSize)) {
				if (labels[neighbor] == -1) {
					labels[neighbor] = currentLabel;
					q.push(neighbor);
				}
			}
		}

		currentLabel++;
	}

	// group indices by label, filter by minPoints
	std::vector<std::vector<int>> rawClusters(currentLabel);
	for (int i = 0; i < n; i++) {
		if (labels[i] >= 0) {
			rawClusters[labels[i]].push_back(i);
		}
	}

	std::vector<std::vector<int>> result;
	result.reserve(rawClusters.size());
	for (auto& cluster : rawClusters) {
		if ((int)cluster.size() >= minPoints) {
			result.push_back(std::move(cluster));
		}
	}

	return result;
}
