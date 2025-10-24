#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <optional>
#include <variant>
#include <iostream>

#include "intersections.hpp"

namespace geometry::queries {
template<class... Ts>
struct Multilambda : Ts... {
    using Ts::operator()...;
};

//===========================================================
//To find distance from pt to line & line to line (hence outside class)
double GetDistanceToLine(const Point2D& point, const Line& line);

/*===========================================================
 * Класс для поиска расстояния от точки до фигуры
 *===========================================================*/

struct PointToShapeDistanceVisitor {
    Point2D point;

    explicit PointToShapeDistanceVisitor(const Point2D& p) : point(p) {
    }

    template<typename T>
    double operator()(T&& unsupported_shape) const {
        throw std::logic_error("Unsupported Shape type for distance to/from point");
    }

    double operator()(const BoundingBox& b) const;
    double operator()(const Line& line) const;
    double operator()(const Circle& circle) const;
    double operator()(const Triangle& t) const;
    double operator()(const Rectangle& r) const;
    double operator()(const RegularPolygon& p) const;
    double operator()(const Polygon& p) const;

private:
    // Compute minimal distance from point to closed polygon
    double DistanceToPolyline(std::span<Point2D> vertices) const;
};


/*===========================================================
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
 * =========================================================== d b*/
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

    std::optional<double> operator()(const Circle& c1, const Circle& c2) const;

    std::optional<double> operator()(const Line& l1, const Line& l2) const;
};

//========== Helper functions =========
double DistanceToPoint(const Shape& shape, const Point2D& point);
BoundingBox GetBoundBox(const Shape& shape);
double GetHeight(const Shape& shape);
std::vector<Point2D> GetVertices(const Shape& shape);
bool BoundingBoxesOverlap(const Shape& shape1, const Shape& shape2);
std::optional<double> DistanceBetweenShapes(const Shape& shape1, const Shape& shape2);
std::vector<Point2D> GetAllShapeVertices(std::span<Shape> shapes);
} // namespace geometry::queries
