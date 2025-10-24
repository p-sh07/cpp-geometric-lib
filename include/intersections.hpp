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

class IntersectionVisitor {
public:
    Result operator()(const Line& a, const Line& b) const;
    Result operator()(const Circle& a, const Circle& b) const;
    Result operator()(const Circle& circle, const Line& line) const;
    Result operator()(const Line& l, const Circle& c) const;

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