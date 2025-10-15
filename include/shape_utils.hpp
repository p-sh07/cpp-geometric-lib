#pragma once
#include "geometry.hpp"
#include "queries.hpp"
#include <print>
#include <random>
#include <ranges>
#include <utility>
#include <vector>

namespace geometry::utils {

class ShapeGenerator {
public:
    explicit ShapeGenerator(double min_coord = -100.0, double max_coord = 100.0, double min_size = 1.0, double max_size = 20.0)
        : gen(std::random_device()()), coord_dist(min_coord, max_coord),
        size_dist(min_size, max_size), sides_dist(3, 12), type_dist(0, 4) {
    }

    Point2D GenerateRandomPoint() {
        return{coord_dist(gen), coord_dist(gen)};
    }

    Shape GenerateRandomShape() {
        Point2D center{coord_dist(gen), coord_dist(gen)};
        double size = size_dist(gen);

        switch (type_dist(gen)) {
            case 0: {
                Point2D end{center.x + size, center.y + size};
                return Line{center, end};
            }
            case 1: {
                Point2D a{center.x, center.y};
                Point2D b{center.x + size, center.y};
                Point2D c{center.x + size / 2, center.y + size};
                return Triangle{a, b, c};
            }
            case 2: {
                return Rectangle{center, size, size * 0.8};
            }
            case 3: {
                int sides = sides_dist(gen);
                return RegularPolygon{center, size, sides};
            }
            case 4: {
                return Circle{center, size};
            }
            default: {
                Point2D end{center.x + size, center.y + size};
                return Line{center, end};
            }
        }
    }

    std::vector<Shape> GenerateShapes(size_t count) {
        //TODO: or use generate_n?
        return std::views::iota(0u, count)
            | std::views::transform([this](auto) { return GenerateRandomShape(); })
            | std::ranges::to<std::vector>();
    }

    //Generates from 1 to 20 random pts
    std::vector<Point2D> GeneratePoints(size_t max_count = 20) {
        std::uniform_int_distribution<size_t> rand_count(1u, max_count);
        auto count = rand_count(gen);
        return std::views::iota(0u, count)
            | std::views::transform([this](auto) { return GenerateRandomPoint(); })
            | std::ranges::to<std::vector>();
    }

private:
    std::mt19937 gen;
    std::uniform_real_distribution<double> coord_dist;
    std::uniform_real_distribution<double> size_dist;
    std::uniform_int_distribution<int> sides_dist;
    std::uniform_int_distribution<int> type_dist;
};

inline std::vector<std::pair<Shape, Shape>> FindAllCollisions(ShapeContainer shapes) {
    std::vector<std::pair<Shape, Shape>> collisions;
    auto shapes_enum = shapes.GetEnum();

    std::ranges::for_each(shapes_enum, [&shapes_enum, &collisions](const auto &pair) {
        const auto& [i, shape1] = pair;
        for (const auto& [j, shape2] : shapes_enum | std::views::drop(i + 1)) {
            if (queries::BoundingBoxesOverlap(shape1, shape2)) {
                collisions.emplace_back(shape1, shape2);
            }
        }
    });
    return collisions;
}

inline std::optional<size_t> FindHighestShape(ShapeContainer shapes) {
    auto max_shape = std::ranges::max_element(shapes.data_, [](const auto& s1, const auto& s2) {
        return queries::GetHeight(s1) < queries::GetHeight(s2);
    });

    if (shapes.empty() || max_shape == shapes.data_.end()) {
        return std::nullopt;
    }

    return queries::GetHeight(*max_shape);
}

inline std::string_view PrintShapeName(const Shape& shape) {
    if (std::holds_alternative<Line>(shape)) return "Line";
    if (std::holds_alternative<Triangle>(shape)) return "Triangle";
    if (std::holds_alternative<Rectangle>(shape)) return "Rectangle";
    if (std::holds_alternative<RegularPolygon>(shape)) return "RegularPolygon";
    if (std::holds_alternative<Circle>(shape)) return "Circle";
    if (std::holds_alternative<Polygon>(shape)) return "Polygon";
    return "Unknown";
}

}  // namespace geometry::utils