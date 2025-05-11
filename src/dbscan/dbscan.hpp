#pragma once

// dbscan library courtesy of Eleobert
// https://github.com/Eleobert/dbscan

#include <cassert>
#include <cstddef>
#include <span>
#include <vector>
#include <cstdlib>
#include "tcb/span.hpp"
#include <iterator>

struct point2
{
    float x, y;
};

struct point3
{
    float x, y, z;
};

auto dbscan(const tcb::span<const point2>& data, float eps, int min_pts) -> std::vector<std::vector<size_t>>;
auto dbscan(const tcb::span<const point3>& data, float eps, int min_pts) -> std::vector<std::vector<size_t>>;
