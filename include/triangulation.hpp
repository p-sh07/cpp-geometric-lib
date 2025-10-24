#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <format>
#include <set>
#include <vector>

namespace geometry::triangulation {

struct DelaunayTriangle {
    Point2D a, b, c;

    DelaunayTriangle(Point2D a, Point2D b, Point2D c) : a(a), b(b), c(c) {
    }

    bool ContainsPoint(const Point2D& p) const {
        Point2D center = Circumcenter();
        double radius  = Circumradius();
        return center.DistanceTo(p) <= radius + EPSILON;
    }

    Point2D Circumcenter() const {
        double d = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
        if (std::abs(d) < EPSILON) {
            return {(a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3};
        }

        double ux = ((a.x * a.x + a.y * a.y) * (b.y - c.y) + (b.x * b.x + b.y * b.y) * (c.y - a.y) +
                     (c.x * c.x + c.y * c.y) * (a.y - b.y)) /
                    d;

        double uy = ((a.x * a.x + a.y * a.y) * (c.x - b.x) + (b.x * b.x + b.y * b.y) * (a.x - c.x) +
                     (c.x * c.x + c.y * c.y) * (b.x - a.x)) /
                    d;

        return {ux, uy};
    }

    double Circumradius() const {
        Point2D center = Circumcenter();
        return center.DistanceTo(a);
    }

    bool SharesEdge(const DelaunayTriangle& other) const {
        std::vector<Point2D> this_points  = {a, b, c};
        std::vector<Point2D> other_points = {other.a, other.b, other.c};

        int shared_count = 0;
        for (const Point2D& p1 : this_points) {
            for (const Point2D& p2 : other_points) {
                if (std::abs(p1.x - p2.x) < EPSILON && std::abs(p1.y - p2.y) < EPSILON) {
                    shared_count++;
                    break;
                }
            }
        }

        return shared_count == 2;
    }

    std::vector<Point2D> vertices() const { return {a, b, c}; }
};

struct Edge {
    Point2D p1, p2;

    Edge(Point2D p1, Point2D p2) : p1(p1), p2(p2) {
        if (p1.x > p2.x || (p1.x == p2.x && p1.y > p2.y)) {
            std::swap(this->p1, this->p2);
        }
    }

    bool operator<(const Edge& other) const {
        if (std::abs(p1.x - other.p1.x) > EPSILON)
            return p1.x < other.p1.x;
        if (std::abs(p1.y - other.p1.y) > EPSILON)
            return p1.y < other.p1.y;
        if (std::abs(p2.x - other.p2.x) > EPSILON)
            return p2.x < other.p2.x;
        return p2.y < other.p2.y;
    }

    bool operator==(const Edge& other) const {
        return std::abs(p1.x - other.p1.x) < EPSILON && std::abs(p1.y - other.p1.y) < EPSILON &&
               std::abs(p2.x - other.p2.x) < EPSILON && std::abs(p2.y - other.p2.y) < EPSILON;
    }
};

inline GeometryResult<std::vector<DelaunayTriangle>> DelaunayTriangulation(std::span<const Point2D> points) {
    // Need at least 3 non-collinear points
    if (points.size() < 3) return std::unexpected(GeometryError::InsufficientPoints);

    // Compute bounding super-triangle that surely contains all points
    double min_x = std::numeric_limits<double>::infinity();
    double min_y = std::numeric_limits<double>::infinity();
    double max_x = -std::numeric_limits<double>::infinity();
    double max_y = -std::numeric_limits<double>::infinity();

    for (const auto& p : points) {
        min_x = std::min(min_x, p.x);
        min_y = std::min(min_y, p.y);
        max_x = std::max(max_x, p.x);
        max_y = std::max(max_y, p.y);
    }

    const double dx        = max_x - min_x;
    const double dy        = max_y - min_y;
    const double delta_max = std::max(dx, dy);
    if (delta_max < EPSILON) return std::unexpected(GeometryError::DegenerateCase);

    // Create a super triangle large enough
    // place it far away: centered at mid and extended by large factor
    const Point2D mid{(min_x + max_x) / 2.0, (min_y + max_y) / 2.0};
    const double big = 16.0 * delta_max; // scale factor safe enough
    Point2D pA{mid.x - big, mid.y - big};
    Point2D pB{mid.x, mid.y + big};
    Point2D pC{mid.x + big, mid.y - big};

    std::vector<DelaunayTriangle> triangulation;
    triangulation.emplace_back(pA, pB, pC);

    // Insert points one by one
    for (const auto& pt : points) {
        // 1.Find all triangles whose circumcircle contains pt (bad triangles)
        std::vector<size_t> bad_indices;
        bad_indices.reserve(triangulation.size());
        for (size_t i = 0; i < triangulation.size(); ++i) {
            if (triangulation[i].ContainsPoint(pt)) {
                bad_indices.push_back(i);
            }
        }

        // 2.Build polygon (boundary) of the hole by collecting edges of bad triangles
        std::vector<Edge> edge_list;
        edge_list.reserve(bad_indices.size() * 3);
        for (size_t idx : bad_indices) {
            const auto& t = triangulation[idx];
            edge_list.emplace_back(t.a, t.b);
            edge_list.emplace_back(t.b, t.c);
            edge_list.emplace_back(t.c, t.a);
        }

        // Remove bad triangles from triangulation
        // erase in descending order to keep indices valid
        std::sort(bad_indices.rbegin(), bad_indices.rend());
        for (size_t idx : bad_indices) {
            triangulation.erase(triangulation.begin() + idx);
        }

        // 3.From edge_list, find boundary edges = edges that appear exactly once
        if (!edge_list.empty()) {
            std::sort(edge_list.begin(), edge_list.end());
        }

        std::vector<Edge> boundary;
        boundary.reserve(edge_list.size());
        for (size_t i = 0; i < edge_list.size();) {
            size_t j = i + 1;
            while (j < edge_list.size() && edge_list[j] == edge_list[i]) ++j;
            size_t count = j - i;
            if (count == 1) {
                boundary.push_back(edge_list[i]);
            }
            i = j;
        }

        // 4.Re-triangulate the hole with point pt and boundary edges
        for (const auto& e : boundary) {
            triangulation.emplace_back(e.p1, e.p2, pt);
        }
    }

    // 5.Remove triangles that share a vertex with super-triangle
    auto is_super_vertex = [&](const Point2D& p) {
        return (std::abs(p.x - pA.x) < EPSILON && std::abs(p.y - pA.y) < EPSILON) ||
               (std::abs(p.x - pB.x) < EPSILON && std::abs(p.y - pB.y) < EPSILON) ||
               (std::abs(p.x - pC.x) < EPSILON && std::abs(p.y - pC.y) < EPSILON);
    };

    triangulation.erase(std::remove_if(triangulation.begin(), triangulation.end(),
                                       [&](const DelaunayTriangle& t) {
                                           return is_super_vertex(t.a) || is_super_vertex(t.b) || is_super_vertex(t.c);
                                       }), triangulation.end());

    if (triangulation.empty()) return std::unexpected(GeometryError::DegenerateCase);

    return triangulation;
}
} // namespace geometry::triangulation

template<>
struct std::formatter<geometry::triangulation::DelaunayTriangle> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const geometry::triangulation::DelaunayTriangle& t, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "DelaunayTriangle({}, {}, {})", t.a, t.b, t.c);
    }
};
