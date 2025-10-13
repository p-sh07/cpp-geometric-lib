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
    void Push(const Point2D &p) { s.push_back(p); }
    void Pop() { s.pop_back(); }

    size_t Size() { return s.size(); }
    Point2D Top() { return s.back(); }
    Point2D NextToTop() { return *std::prev(s.end(), 2); }

    std::vector<Point2D> &&Extract() && { return std::move(s); }

private:
    std::vector<Point2D> s;
};

GeometryResult<std::vector<Point2D>> GrahamScan(ShapeContainer shapes) {

    auto pts_opt = shapes.GetAsPoint2DVec();
    if (!pts_opt.has_value()) {
        return std::unexpected(GeometryError::InvalidInput);
    }

    auto& pts = pts_opt.value();
    if (pts.size() < 3) {
        return std::unexpected(GeometryError::InsufficientPoints);
    }

    // Find P0: lowest y, then leftmost x
    auto p0_it = std::ranges::min_element(pts, {}, [](const Point2D& p) {
        return std::pair{p.y, p.x};
    });
    std::swap(pts[0], *p0_it);
    Point2D P0 = pts[0];

    // Sort by polar angle with P0
    std::sort(pts.begin() + 1, pts.end(), [&](const Point2D& a, const Point2D& b) {
        double cross = CrossProduct(P0, a, b);
        if (std::abs(cross) < 1e-12)
            return std::hypot(a.x - P0.x, a.y - P0.y) < std::hypot(b.x - P0.x, b.y - P0.y);
        return cross > 0;
    });

    // Remove duplicates (same polar angle) → keep farthest
    std::vector<Point2D> filtered{P0};
    for (size_t i = 1; i < pts.size(); ++i) {
        while (i + 1 < pts.size() && std::abs(CrossProduct(P0, pts[i], pts[i + 1])) < 1e-12) {
            ++i;
        }
        filtered.push_back(pts[i]);
    }

    if (filtered.size() < 3)
        return std::unexpected(GeometryError::DegenerateCase);

    // Graham Scan with custom stack
    StackForGrahamScan stack;
    stack.Push(filtered[0]);
    stack.Push(filtered[1]);
    stack.Push(filtered[2]);

    for (size_t i = 3; i < filtered.size(); ++i) {
        while (stack.Size() > 1 && CrossProduct(stack.NextToTop(), stack.Top(), filtered[i]) <= 0)
            stack.Pop();
        stack.Push(filtered[i]);
    }

    return std::move(stack).Extract();
}

}  // namespace geometry::convex_hull