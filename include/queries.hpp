#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <optional>
#include <variant>

#include "intersections.hpp"

namespace geometry::queries {
template<class... Ts>
struct Multilambda : Ts... {
    using Ts::operator()...;
};

//To find distance from pt to line & line to line (hence outside class)
double GetDistanceToLine(const Point2D& point, const Line& line) {
    Point2D ap = point - line.start;
    Point2D ab = line.end - line.start;

    //clamp for cases where point would be on the same line, but is outside of segment
    double proj     = std::clamp(ap.Dot(ab) / ab.Dot(ab), 0.0, 1.0);
    Point2D closest = line.start + ab * proj;

    return point.DistanceTo(closest);
}

/*
 * Класс для поиска расстояния от точки до фигуры
 *
 * Требуется организовать возможность нахождения расстояния для всех возможных фигур типа-суммы Shape
 */
struct PointToShapeDistanceVisitor {
    Point2D point;

    explicit PointToShapeDistanceVisitor(const Point2D& p) : point(p) {
    }

    template<typename T>
    double operator()(T&& unsupported_shape) const {
        throw std::logic_error("Unsupported Shape type for distance to/from point");
    }

    double operator()(const BoundingBox& b) const {
        double clamped_x = std::clamp(point.x, b.min_x, b.max_x);
        double clamped_y = std::clamp(point.y, b.min_y, b.max_y);
        return point.DistanceTo(Point2D{clamped_x, clamped_y});
    }

    double operator()(const Line& line) const {
        return GetDistanceToLine(point, line);
    }

    double operator()(const Circle& circle) const {
        double center_dist = point.DistanceTo(circle.center_p);
        //return center_dist - circle.radius;
        return std::max(0.0, center_dist - circle.radius); //assume within circle counts as 0 dist
    }

    double operator()(const Triangle& t) const {
        return DistanceToPolyline(t.VerticesVec());
    }

    double operator()(const Rectangle& r) const {
        return DistanceToPolyline(r.VerticesVec());
    }

    double operator()(const RegularPolygon& p) const {
        return DistanceToPolyline(p.Vertices());
    }

    double operator()(const Polygon& p) const {
        return DistanceToPolyline(p.Vertices());
    }

private:
    // Compute minimal distance from point to closed polygon (loop = true)
    double DistanceToPolyline(std::vector<Point2D>&& vertices) const {
        double min_dist = std::numeric_limits<double>::infinity();
        size_t count    = vertices.size();

        for (size_t i = 0; i < count; ++i) {
            const Point2D& a = vertices[i];
            const Point2D& b = vertices[(i + 1) % count]; // next vertex or [0], assume closed shape
            min_dist = std::min(min_dist, GetDistanceToLine(point, Line{a,b}));
        }

        return min_dist;
    }
};


/*
 * Класс для поиска расстояния между двумя фигурами
 *
 * Требуется организовать возможность нахождения расстояния только для следующих комбинаций фигур:
 *    - Any    & Point
 *    - Line   & Line
 *    - Circle & Circle
 *
 * Важно: вы можете выбрать любой метод нахождения расстояния, даже если он даёт не точный результат
 *
 * Для всех остальных требуется вернуть пустое значение
 */
struct ShapeToShapeDistanceVisitor {
    //unsupported shape combination
    template<typename T1, typename T2>
    std::optional<double> operator()(T1&&, T2&&) const {
        //throw std::logic_error("Unsupported Shape type(s) for distance calculation");
        return std::nullopt;
    }

    template<typename ShapeT>
    std::optional<double> operator()(const ShapeT& shape, const Point2D& pt) const {
        return std::visit(PointToShapeDistanceVisitor{pt}, shape);
    }

    std::optional<double> operator()(const Circle& c1, const Circle& c2) const {
        //TODO: nullopt or allow?
        if (c1.radius == 0 || c2.radius == 0) {
            return std::nullopt;
        }

        const double center_dist = c1.Center().DistanceTo(c2.Center());
        const double r_sum = c1.Radius() + c2.Radius();

        return std::max(0.0, center_dist - r_sum);
    }

    std::optional<double> operator()(const Line& l1, const Line& l2) const {
        //TODO: case when line is 0 length? Leads to NaN, so nullopt for now
        if (l1.Length() == 0 || l2.Length() == 0) {
            return std::nullopt;
        }

        if (intersections::GetIntersectPoint(l1, l2).has_value()) {
            return 0.0;
        }

        return std::min({
            GetDistanceToLine(l1.start, l2),
            GetDistanceToLine(l1.end, l2),
            GetDistanceToLine(l2.start, l1),
            GetDistanceToLine(l2.end, l1)
        });
    }

};

/*
 * Функции-помощники
 */
inline double DistanceToPoint(const Shape& shape, const Point2D& point) {
    return std::visit(PointToShapeDistanceVisitor{point}, shape);
}

inline BoundingBox GetBoundBox(const Shape& shape) {
    return std::visit(Multilambda{
                          [](const Line& l) { return l.BoundBox(); },
                          [](const Triangle& t) { return t.BoundBox(); },
                          [](const Circle& c) { return c.BoundBox(); },
                          //[](const Rectangle& r){ return r.BoundBox(); }, //TODO?
                          //[](const Polygon& p){ return p.BoundBox(); },
                          [](auto&&) -> BoundingBox { throw std::logic_error{"Unsupported value"}; }
                      }, shape);
}

inline double GetHeight(const Shape& shape) {
    //cannot use c++26 shape.visit in llvm...
    return std::visit(Multilambda{
                          [](const BoundingBox& b) { return b.Height(); },
                          [](const Line& l) { return l.Height(); },
                          [](const Triangle& t) { return t.Height(); },
                          [](const Rectangle& r) { return r.Height(); },
                          [](const Circle& c) { return c.Height(); },
                          [](const Polygon& p) { return p.Height(); },
                          [](const RegularPolygon& rp) { return rp.Height(); },
                          [](auto&&) -> double { throw std::logic_error{"Unsupported value"}; }
                      }, shape);
}

inline bool BoundingBoxesOverlap(const Shape& shape1, const Shape& shape2) {
    return GetBoundBox(shape1).Overlaps(GetBoundBox(shape2));
}

inline std::optional<double> DistanceBetweenShapes(const Shape& shape1, const Shape& shape2) {
    return std::visit(ShapeToShapeDistanceVisitor{}, shape1, shape2);
}
} // namespace geometry::queries
