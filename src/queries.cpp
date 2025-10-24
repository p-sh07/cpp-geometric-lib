//
// Created by Pavel on 20.10.2025.
//
#include "queries.hpp"

namespace geometry::queries {
double GetDistanceToLine(const Point2D& point, const Line& line) {
    Point2D ap = point - line.start;
    Point2D ab = line.end - line.start;

    //clamp for cases where point would be on the same line, but is outside of segment
    double proj     = std::clamp(ap.Dot(ab) / ab.Dot(ab), 0.0, 1.0);
    Point2D closest = line.start + ab * proj;

    return point.DistanceTo(closest);

}

double PointToShapeDistanceVisitor::operator()(const BoundingBox& b) const {
    double clamped_x = std::clamp(point.x, b.min_x, b.max_x);
    double clamped_y = std::clamp(point.y, b.min_y, b.max_y);
    return point.DistanceTo(Point2D{clamped_x, clamped_y});
}

double PointToShapeDistanceVisitor::operator()(const Line& line) const {
    return GetDistanceToLine(point, line);
}

double PointToShapeDistanceVisitor::operator()(const Circle& circle) const {
    double center_dist = point.DistanceTo(circle.center_p);
    //return center_dist - circle.radius;
    return std::max(0.0, center_dist - circle.radius); //assume within circle counts as 0 dist
}

double PointToShapeDistanceVisitor::operator()(const Triangle& t) const {
    auto v = t.Vertices();
    return DistanceToPolyline(v);
}

double PointToShapeDistanceVisitor::operator()(const Rectangle& r) const {
    auto v = r.Vertices();
    return DistanceToPolyline(v);
}

double PointToShapeDistanceVisitor::operator()(const RegularPolygon& p) const {
    auto v = p.Vertices();
    return DistanceToPolyline(v);
}

double PointToShapeDistanceVisitor::operator()(const Polygon& p) const {
    auto v = p.Vertices();
    return DistanceToPolyline(v);
}

double PointToShapeDistanceVisitor::DistanceToPolyline(std::span<Point2D> vertices) const {
    double min_dist = std::numeric_limits<double>::infinity();
    size_t count    = vertices.size();

    for (size_t i = 0; i < count; ++i) {
        const Point2D& a = vertices[i];
        const Point2D& b = vertices[(i + 1) % count]; // next vertex or [0], assume closed shape
        min_dist         = std::min(min_dist, GetDistanceToLine(point, Line{a,b}));
    }

    return min_dist;
}

std::optional<double> ShapeToShapeDistanceVisitor::operator()(const Circle& c1, const Circle& c2) const {
    //TODO: nullopt or allow?
    if (c1.radius == 0 || c2.radius == 0) {
        return std::nullopt;
    }

    const double center_dist = c1.Center().DistanceTo(c2.Center());
    const double r_sum       = c1.Radius() + c2.Radius();

    return std::max(0.0, center_dist - r_sum);
}

std::optional<double> ShapeToShapeDistanceVisitor::operator()(const Line& l1, const Line& l2) const {
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

//========== Helper functions =========
double DistanceToPoint(const Shape& shape, const Point2D& point) {
    return std::visit(PointToShapeDistanceVisitor{point}, shape);
}

BoundingBox GetBoundBox(const Shape& shape) {
    return std::visit(Multilambda{
                          [](const Line& l) { return l.BoundBox(); },
                          [](const Triangle& t) { return t.BoundBox(); },
                          [](const Circle& c) { return c.BoundBox(); },
                          [](const Rectangle& r){ return r.BoundBox(); },
                          [](const Polygon& p){ return p.BoundBox(); },
                          [](const RegularPolygon& p){ return p.BoundBox(); },
                          [](auto&&) -> BoundingBox { throw std::logic_error{"Unsupported value"}; }
                      }, shape);
}

double GetHeight(const Shape& shape) {
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

//TODO: Some of the vertex points here are temporary/rvalue (calculated at Vertices() call, e.g. rectangle)
//Any other way to avoid copying points?
std::vector<Point2D> GetVertices(const Shape& shape) {
    return std::visit(Multilambda{
                          [](const Line& l) { return std::vector<Point2D>{l.start, l.end}; },
                          [](const Triangle& t) { return t.VerticesVec(); },
                          [](const Rectangle& r) { return r.VerticesVec(); },
                          [](const Circle& c) { return c.Vertices(); },
                          [](const Polygon& p) { return p.Vertices(); },
                          [](const RegularPolygon& rp) { return rp.Vertices(); },
                          [](auto&&) -> double { throw std::logic_error{"Unsupported value"}; }
                      }, shape);
}

bool BoundingBoxesOverlap(const Shape& shape1, const Shape& shape2) {
    try {
        return GetBoundBox(shape1).Overlaps(GetBoundBox(shape2));
    } catch (std::logic_error& err) {
        //TODO: std::expected
        std::println(std::cerr, "Bounding box overlap error: {}", err.what());
        return false;
    }
}

std::optional<double> DistanceBetweenShapes(const Shape& shape1, const Shape& shape2) {
    return std::visit(ShapeToShapeDistanceVisitor{}, shape1, shape2);
}

std::vector<Point2D> GetAllShapeVertices(std::span<Shape> shapes) {
    std::vector<Point2D> result;
    for (const auto& shape : shapes) {
        try {
            result.append_range(std::move(GetVertices(shape)));
        } catch (std::exception& ex) {
            std::println(std::cerr, "GetVertices error: {}", ex.what());
        }
    }
    return result;
}
} //namespace queries
