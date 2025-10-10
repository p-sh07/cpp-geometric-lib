#pragma once
#include "geometry.hpp"
#include <cmath>
#include <optional>

namespace geometry::intersections {

/*
 * Класс для поиска пересечений между двумя фигурами
 *
 * Требуется организовать возможность нахождения пересечений только для следующих комбинаций фигур:
 *    - Line   & Line
 *    - Circle & Circle
 *
 * Для всех остальных требуется вернуть std::nullopt
 */

// Use just Result beacause geometry::intersections::IntersectionResult is pointless
using Result = std::optional<std::vector<Point2D>>;

template <typename T>
concept IsLine = std::is_same_v<std::decay_t<T>, Line>;

template <typename T>
concept IsCircle = std::is_same_v<std::decay_t<T>, Circle>;

class IntersectionVisitor {
public:
    Result operator()(const Line& a, const Line& b) const {
        const Point2D v1 = a.end - a.start;
        const Point2D v2 = b.end - b.start;
        const Point2D v3 = b.start - a.start;

        // Calculate det. using 2D cross product
        double det = v1.Cross(v2);
        if (std::abs(det) < 1e-9) {
            return std::nullopt;
        }

        double t = v3.Cross(v1) / det;
        double u = v3.Cross(v2) / det;

        // Check if point lies on both segments
        if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
            return {{a.start + v1 * t}};
        }
        return std::nullopt;
    }

    Result operator()(const Circle& a, const Circle& b) const {
        Point2D p0 = a.center_p;
        Point2D p1 = b.center_p;
        double r0 = a.radius;
        double r1 = b.radius;

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
        Point2D p2 = p0 + dir * x;

        // Offset vector
        Point2D offset{-dir.y * h, dir.x * h};

        return {{{p2 + offset}, {p2 - offset}}};
    }

    Result operator()(const Circle& circle, const Line& line) const {
        Point2D d = line.end - line.start;
        Point2D f = line.start - circle.center_p;

        double a = d.Dot(d);
        double b = 2 * f.Dot(d);
        double c = f.Dot(f) - circle.radius * circle.radius;

        double discr = b * b - 4 * a * c;

        if (discr < EPSILON) {
            return std::nullopt;  // No intersection
        }

        discr = std::sqrt(discr);
        double t1 = (-b - discr) / (2 * a);
        double t2 = (-b + discr) / (2 * a);

        std::vector<Point2D> result;  // to support case of two intersections, but return first one for now

        if (t1 >= 0 && t1 <= 1) {
            result.push_back(line.start + d * t1);
        }
        if (t2 >= 0 && t2 <= 1 && t2 != t1) {
            result.push_back(line.start + d * t2);
        }

        // TODO: look up in lessons about move/copy during this case
        return result.empty() ? Result{} : result;
    }

    Result operator()(const Line& l, const Circle& c) const { return operator()(c, l); }

    //General case for unsupported types
    template <typename T, typename S>
    Result operator()(T&& value1, S&& value2) const {
        // return std::nullopt;
        throw std::logic_error("Unsupported types for Intersection search");
    }
};

inline Result GetIntersectPoint(const Shape& shape1, const Shape& shape2) {
    return std::visit(IntersectionVisitor{}, shape1, shape2);
}

}  // namespace geometry::intersections

/** multilambda example
* using Value = std::variant<int, double, std::string>;

Value a = 24;
Value b = 42;

auto result =
std::visit(Multilambda{[](int l, int r) -> double { return l + r; }, [](double l, double r) { return l + r; },
[](const auto &, const auto &) -> double {
throw std::logic_error{"Unsupported type combination"};
return {};
}},
a, b);

std::println("Результат: {}", result);  // 66
*/