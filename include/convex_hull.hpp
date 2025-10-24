#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <ranges>
#include <stack>
#include <vector>

namespace geometry::convex_hull {

double CrossProduct(Point2D p1, Point2D middle, Point2D p2);

class StackForGrahamScan {
public:
    void Push(const Point2D &p);
    void Pop();
    size_t Size();
    Point2D Top();
    Point2D NextToTop();
    std::vector<Point2D> &&Extract() &&;

private:
    std::vector<Point2D> s;
};

GeometryResult<std::vector<Point2D>> GrahamScan(std::span<Point2D> pts);

}  // namespace geometry::convex_hull