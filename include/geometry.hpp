#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <expected>
#include <format>
#include <numbers>
#include <numeric>
#include <optional>
#include <print>
#include <random>
#include <ranges>
#include <variant>
#include <vector>

#include <range/v3/view/enumerate.hpp>

namespace geometry {
//since c++ 17 - helps avoid  ODR-issues, can take references to it.
inline constexpr double EPSILON = 10e-9;
using std::literals::operator ""sv;

struct Point2D {
    double x, y;

    constexpr Point2D() : x(0), y(0) {
    }

    constexpr Point2D(double x, double y) : x(x), y(y) {
    }

    // Comparison
    bool operator<(const Point2D& other) const {
            return std::tie(x, y) < std::tie(other.x, other.y);
        }
    bool operator==(const Point2D& other) const { return x == other.x && y == other.y; }

    // Binary math operators
    [[nodiscard]] Point2D operator+(const Point2D& other) const noexcept { return {x + other.x, y + other.y}; }
    [[nodiscard]] Point2D operator-(const Point2D& other) const noexcept { return {x - other.x, y - other.y}; }
    [[nodiscard]] Point2D operator*(double value) const noexcept { return {x * value, y * value}; }
    [[nodiscard]] Point2D operator/(double value) const noexcept { return {x / value, y / value}; }

    // Binary geometry operations
    [[nodiscard]] double Dot(const Point2D& other) const noexcept { return x * other.x + y * other.y; }
    [[nodiscard]] double Cross(const Point2D& other) const noexcept { return x * other.y - y * other.x; }
    [[nodiscard]] double Length() const noexcept { return std::sqrt(x * x + y * y); }
    [[nodiscard]] double DistanceTo(const Point2D& other) const noexcept { return (*this - other).Length(); }

    [[nodiscard]] Point2D Normalize() const noexcept {
        const double len = Length();
        return len > 0 ? Point2D{x / len, y / len} : Point2D{0, 0};
    }
};

template<size_t N>
struct Lines2D {
    std::array<double, N> x;
    std::array<double, N> y;
};

struct Lines2DDyn {
    Lines2DDyn() = default;

    explicit Lines2DDyn(const std::vector<Point2D>& vertice_vec) {
        Reserve(vertice_vec.size() + 1);
        std::ranges::for_each(vertice_vec, [&](auto pt) { PushBack(pt); });
        PushBack(Front()); //add first point as last
    }

    std::vector<double> x;
    std::vector<double> y;

    void Reserve(size_t n) {
        x.reserve(n);
        y.reserve(n);
    }

    void PushBack(Point2D p) {
        x.push_back(p.x);
        y.push_back(p.y);
    }

    void PushBack(double px, double py) {
        x.push_back(px);
        y.push_back(py);
    }

    Point2D Front() const { return {x.front(), y.front()}; }
};

struct BoundingBox {
    double min_x, min_y, max_x, max_y;

    bool Overlaps(const BoundingBox& other) const {
        return (max_x >= other.min_x) && (min_x <= other.max_x)     //x overlap
               && (max_y >= other.min_y) && (min_y <= other.max_y); //and y overlap
    }

    double Width() const { return max_x - min_x; }
    double Height() const { return max_y - min_y; }
    Point2D Center() const { return {min_x + Width() / 2, min_y + Height() / 2}; }
};

inline BoundingBox BboxFromPoints(std::span<Point2D> points) {
    if (points.empty()) {
        return {};
    }

    auto [min_x_it, max_x_it] = std::minmax_element(points.begin(), points.end(),
                                                    [](const Point2D& a, const Point2D& b) { return a.x < b.x; });
    auto [min_y_it, max_y_it] = std::minmax_element(points.begin(), points.end(),
                                                    [](const Point2D& a, const Point2D& b) { return a.y < b.y; });

    return {min_x_it->x, min_y_it->y, max_x_it->x, max_y_it->y};
}

struct Line {
    Point2D start, end;

    double Length() const { return end.DistanceTo(start); }
    Point2D Direction() const { return (end - start) / Length(); } //unit vector for direction
    BoundingBox BoundBox() const { return {std::min(start.x, end.x), std::min(start.y, end.y), std::max(start.x, end.x), std::max(start.y, end.y)}; }
    double Height() const { return std::max(start.y, end.y); }


    Point2D Center() { return {start.x + (end.x - start.x) / 2, start.y + (end.y - start.y) / 2}; }
    std::array<Point2D, 2> Vertices() { return {Point2D{start.x, start.y}, {end.x, end.y}}; }
    Lines2D<2> Lines() const { return {{start.x, end.x}, {start.y, end.y}}; }
};

struct Triangle {
    Point2D a, b, c;
    Point2D Center() const { return {(a + b + c) / 3.0}; }
    std::array<Point2D, 3> Vertices() const { return {a, b, c}; }

    //TODO: Я не смог избавится от VerticesVec, потому что для полноценной замены на span нужно, чтобы объект был не временный (не rvalue)
    //А в некоторых из фигур Vertices высчитывается "на лету" - например, Rectangle. Создать std::span из rvalue array не получается (и не должно)))
    //Попытка заменить на std::span привела к огромной куче рефакторинга в функции-visitor GetVertices()... и, честно говоря, не думаю, что это катастрофично, копируются легкие точки
    std::vector<Point2D> VerticesVec() const { return {a, b, c}; }
    Lines2D<4> Lines() const { return {{a.x, b.x, c.x, a.x}, {a.y, b.y, c.y, a.y}}; }
    double Area() const { return std::abs((a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y)) / 2.0); }
    double Height() const { return std::max({a.y, b.y, c.y}); }

    BoundingBox BoundBox() const {
        auto v = Vertices();
        return BboxFromPoints(v);
    }

    //TODO: Possible to do this to use span for vertices?:
    /**
    union {
        struct {
            Point2D a, b, c;
        };
        std::array<Point2D, 3> vertices;
    };

    std::span<const Point2D> VerticesSpan() const {
        return std::span<const Point2D>(vertices.data(), 3);
    }
    */
};

struct Rectangle {
    Point2D bottom_left;
    double width, height;

    std::array<Point2D, 4> Vertices() const {
        return {bottom_left, {bottom_left.x + width, bottom_left.y}, {bottom_left.x + width, bottom_left.y + height}, {bottom_left.x, bottom_left.y + height}};
    }

    std::vector<Point2D> VerticesVec() const {
        return {bottom_left, {bottom_left.x + width, bottom_left.y}, {bottom_left.x + width, bottom_left.y + height}, {bottom_left.x, bottom_left.y + height}};
    }

    Point2D Center() const { return {bottom_left.x + width / 2.0, bottom_left.y + height / 2.0}; }

    Lines2D<5> Lines() const {
        const auto v = Vertices();
        return {{v[0].x, v[1].x, v[2].x, v[3].x, v[0].x}, {v[0].y, v[1].y, v[2].y, v[3].y, v[0].y}};
    }

    double Height() const {
        return std::ranges::max(Vertices(), {}, &Point2D::y).y; //Y value of point with highest Y
    }

    BoundingBox BoundBox() const {
        return {bottom_left.x, bottom_left.y, bottom_left.x + width, bottom_left.y + height};
    }
};

struct RegularPolygon {
    Point2D center_p;
    double radius;
    int sides;

    constexpr RegularPolygon(Point2D center, double radius, int sides)
        : center_p(center), radius(radius), sides(sides) {
    }

    Point2D Center() const { return center_p; }

    std::vector<Point2D> Vertices() const {
        std::vector<Point2D> points;
        points.reserve(sides);

        for (size_t i = 0; i < sides; ++i) {
            const double angle = 2 * std::numbers::pi * i / sides;
            points.emplace_back(center_p.x + radius * std::cos(angle), center_p.y + radius * std::sin(angle));
        }

        return points;
    }

    Lines2DDyn Lines() const { return Lines2DDyn{Vertices()}; }
    double Height() const { return center_p.y + radius; }

    BoundingBox BoundBox() const {
        auto v = Vertices();
        return BboxFromPoints(v);
    }
};

struct Circle {
    Point2D center_p;
    double radius;

    constexpr Circle(Point2D center, double radius) : center_p(center), radius(radius) {
    }

    BoundingBox BoundBox() const {
        return {center_p.x - radius, center_p.y - radius, center_p.x + radius, center_p.y + radius};
    }

    double Height() const { return center_p.y + radius; }
    double Radius() const { return radius; }
    Point2D Center() const { return center_p; }

    std::vector<Point2D> Vertices(size_t N = 30) const {
        std::vector<Point2D> points;
        points.reserve(N);

        for (size_t i = 0; i < N; ++i) {
            const double angle = 2 * std::numbers::pi * i / N;
            points.emplace_back(center_p.x + radius * std::cos(angle), center_p.y + radius * std::sin(angle));
        }

        return points;
    }

    Lines2DDyn Lines(int N = 100) const { return Lines2DDyn{Vertices(N)}; }
};

class Polygon {
public:
    explicit Polygon(std::vector<Point2D> points)
        : bounding_box_(std::move(BboxFromPoints(points)))
        , points_(std::move(points)) {
    }

    Point2D Center() {
        Point2D sum = std::accumulate(points_.begin(), points_.end(), Point2D{0.0, 0.0});
        return sum / points_.size();
    }

    std::vector<Point2D> Vertices() const { return points_; }

    Lines2DDyn Lines() const {
        return Lines2DDyn{Vertices()};;
    }

    double Height() const {
        if (points_.empty()) return 0.0;
        return std::ranges::max(points_, {}, &Point2D::y).y;
    }

    BoundingBox BoundBox() const { return bounding_box_; }

private:
    BoundingBox bounding_box_;
    std::vector<Point2D> points_;
};


using Shape = std::variant<Line, Triangle, Rectangle, RegularPolygon, Circle, Polygon>;

enum class GeometryError { Unsupported, NoIntersection, InvalidInput, DegenerateCase, InsufficientPoints };

static constexpr std::array GeometryErrorString{
    "Unsupported"sv, "NoIntersection"sv, "InvalidInput"sv, "DegenerateCase"sv, "InsufficientPoints"sv
};

template<typename T>
using GeometryResult = std::expected<T, GeometryError>;
} // namespace geometry

template<>
struct std::formatter<geometry::Point2D> {
    constexpr auto parse(std::format_parse_context& ctx) const { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const geometry::Point2D& p, FormatContext& ctx) const {
        return format_to(ctx.out(), "({:.2f}, {:.2f})", p.x, p.y);
    }
};

using std::literals::operator ""sv;

template<>
struct std::formatter<std::vector<geometry::Point2D>> {
    bool use_new_line = false;

    constexpr auto parse(std::format_parse_context& ctx) {
        use_new_line = std::string_view(ctx).starts_with("new_line"sv);
        return use_new_line ? ctx.begin() + "new_line"sv.size() : ctx.begin();
    }

    template<typename FormatContext>
    auto format(const std::vector<geometry::Point2D>& points, FormatContext& ctx) const {
        bool first = true;
        std::ranges::for_each(points, [&](const auto& point) {
            if (use_new_line) {
                std::format_to(ctx.out(), "\t{}\n", point);
            } else {
                if (!first) {
                    std::format_to(ctx.out(), ", ");
                }
                std::format_to(ctx.out(), "{}", point);
                first = false;
            }
        });
        return ctx.out();
    }
};

template<>
struct std::formatter<geometry::Line> {
    constexpr auto parse(std::format_parse_context& ctx) const { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const geometry::Line& l, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "Line({}, {})", l.start, l.end);
    }
};

template<>
struct std::formatter<geometry::Circle> {
    constexpr auto parse(std::format_parse_context& ctx) const { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const geometry::Circle& c, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "Circle(center={}, r={:.2f})", c.center_p, c.radius);
    }
};

template<>
struct std::formatter<geometry::Rectangle> {
    constexpr auto parse(std::format_parse_context& ctx) const { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const geometry::Rectangle& r, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "Rectangle(bottom_left={}, w={:.2f}, h={:.2f})", r.bottom_left, r.width,
                              r.height);
    }
};

template<>
struct std::formatter<geometry::RegularPolygon> {
    constexpr auto parse(std::format_parse_context& ctx) const { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const geometry::RegularPolygon& p, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "RegularPolygon(center={}, r={:.2f}, sides={})", p.center_p, p.radius,
                              p.sides);
    }
};

template<>
struct std::formatter<geometry::Triangle> {
    constexpr auto parse(std::format_parse_context& ctx) const { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const geometry::Triangle& t, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "Triangle({}, {}, {})", t.a, t.b, t.c);
    }
};

template<>
struct std::formatter<geometry::Polygon> {
    constexpr auto parse(std::format_parse_context& ctx) const { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const geometry::Polygon& poly, FormatContext& ctx) const {
        auto out = ctx.out();
        out      = std::format_to(out, "Polygon[{} points]: [", poly.Vertices().size());

        for (const auto& p : poly.Vertices()) {
            out = std::format_to(out, "{} ", p);
        }

        return std::format_to(out, "]");
    }
};

//To print shape name (as string):
template<>
struct std::formatter<geometry::Shape> {
    constexpr auto parse(std::format_parse_context& ctx) const { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const geometry::Shape& s, FormatContext& ctx) const {
        if (std::holds_alternative<geometry::Line>(s)) {
            return std::format_to(ctx.out(), "Line");
        }
        if (std::holds_alternative<geometry::Triangle>(s)) {
            return std::format_to(ctx.out(), "Triangle");
        }
        if (std::holds_alternative<geometry::Rectangle>(s)) {
            return std::format_to(ctx.out(), "Rectangle");
        }
        if (std::holds_alternative<geometry::RegularPolygon>(s)) {
            return std::format_to(ctx.out(), "Regular Polygon");
        }
        if (std::holds_alternative<geometry::Circle>(s)) {
            return std::format_to(ctx.out(), "Circle");
        }
        if (std::holds_alternative<geometry::Polygon>(s)) {
            return std::format_to(ctx.out(), "Polygon");
        }
        //Unknown shape
        return std::format_to(ctx.out(), "Unknown");
    }
};

//For case when using Sample (returns vector<reference_wrapper> to avoid copying shapes)
template<>
struct std::formatter<std::reference_wrapper<geometry::Shape>> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const std::reference_wrapper<geometry::Shape>& s, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "{}", s.get());
    }
};