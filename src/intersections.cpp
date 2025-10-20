//
// Created by Pavel on 20.10.2025.
//
#include "intersections.hpp"

namespace geometry::intersections {
Result IntersectionVisitor::operator()(const Line& a, const Line& b) const {
    const Point2D v1 = a.end - a.start;
    const Point2D v2 = b.end - b.start;

    // Calculate det. using 2D cross product
    double det = v1.Cross(v2);
    if (std::abs(det) < EPSILON) {
        return std::nullopt;
    }

    const Point2D v3 = b.start - a.start;

    double t = v3.Cross(v1) / det;
    double u = v3.Cross(v2) / det;

    // Check if point lies on both segments
    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
        return {{a.start + v1 * t}};
    }
    return std::nullopt;
}

Result IntersectionVisitor::operator()(const Circle& a, const Circle& b) const {
    Point2D p0 = a.center_p;
    Point2D p1 = b.center_p;
    double r0  = a.radius;
    double r1  = b.radius;

    double d = p0.DistanceTo(p1);

    // No solution
    if (d > r0 + r1 || d < std::abs(r0 - r1))
        return std::nullopt;

    // Single point (tangent)
    if (d == 0 && r0 == r1) {
        // Infinite intersections (coincident)
        return std::nullopt;
    }

    // Find x, h
    double x = (r0 * r0 - r1 * r1 + d * d) / (2 * d);
    double h = std::sqrt(r0 * r0 - x * x);

    // Find P2
    Point2D dir = (p1 - p0).Normalize();
    Point2D p2  = p0 + dir * x;

    // Offset vector
    Point2D offset{-dir.y * h, dir.x * h};

    return {{{p2 + offset}, {p2 - offset}}};
}

Result IntersectionVisitor::operator()(const Circle& circle, const Line& line) const {
    Point2D d = line.end - line.start;
    Point2D f = line.start - circle.center_p;

    double a = d.Dot(d);
    double b = 2 * f.Dot(d);
    double c = f.Dot(f) - circle.radius * circle.radius;

    double discr = b * b - 4 * a * c;

    if (discr < EPSILON) {
        return std::nullopt;  // No intersection
    }

    discr     = std::sqrt(discr);
    double t1 = (-b - discr) / (2 * a);
    double t2 = (-b + discr) / (2 * a);

    std::vector<Point2D> result;

    if (t1 >= 0 && t1 <= 1) {
        result.push_back(line.start + d * t1);
    }
    if (t2 >= 0 && t2 <= 1 && t2 != t1) {
        result.push_back(line.start + d * t2);
    }

    // TODO: look up in lessons about move/copy during this case
    return result.empty() ? Result{} : result;
}

Result IntersectionVisitor::operator()(const Line& l, const Circle& c) const { return operator()(c, l); }
}
