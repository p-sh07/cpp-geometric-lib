#include "convex_hull.hpp"
#include "geometry.hpp"
#include "intersections.hpp"
#include "queries.hpp"
#include "shape_utils.hpp"
#include "triangulation.hpp"
#include "visualization.hpp"

#include <algorithm>
#include <print>
#include <ranges>
#include <range/v3/view/enumerate.hpp>

using namespace geometry;

namespace rng = std::ranges;
namespace views = std::ranges::views;
namespace views_v3 = ranges::views;

void PrintAllIntersections(ShapeContainer shapes) {
    std::println("\n=== Intersections ===");

    //Filter only valid shapes & enum [i, shape] to iterate all pairs efficiently with drop
    auto intersectible = shapes.GetIntersectible();
    const auto intersect_enum = intersectible | ranges::views::enumerate; //TODO: this doesn't work when passed from inside a member function, relies on source view still being in scope?

    rng::for_each(intersect_enum, [&intersect_enum](const auto &pair) {
        const auto& [i, shape1] = pair;
        for (const auto& [j, shape2] : intersect_enum | views::drop(i + 1)) {
            if (auto result = intersections::GetIntersectPoint(shape1, shape2); result.has_value()) {
                std::println("{}[{}] & {}[{}] пересекаются в: {}",
                    utils::PrintShapeName(shape1), i+1,
                    utils::PrintShapeName(shape2), j+1,
                    result.value()
                );
            }
        }
    });

}
void PrintDistancesFromPointToShapes(Point2D p, const ShapeContainer& shapes) {
    std::println("\n=== Distance from Point Test ===");
    std::println("Testing point: {}", p);

    // Compute distances and print results for 5 shapes
    rng::for_each(shapes.GetSample(5), [p](const auto& shape) {
        std::println("Расстояние от точки {} до фигуры {} равно {:.2f}", p,
            utils::PrintShapeName(shape), queries::DistanceToPoint(shape, p)
        );
    });
}

void PerformShapeAnalysis(const ShapeContainer& shapes) {
    std::println("\n=== Shape Analysis ===");

    auto collisions = utils::FindAllCollisions(shapes);
    if (collisions.empty()) {
        std::println("Нет коллизий между фигурами");
    } else {
        std::println("Обнаружено {} коллизий:", collisions.size());
        for (const auto& [s1, s2] : collisions) {
            std::println("  - {} & {}", utils::PrintShapeName(s1), utils::PrintShapeName(s2));
        }
    }

    if (auto height = utils::FindHighestShape(shapes)) {
        std::println("Высота самой высокой фигуры (y_max): {}", *height);
    } else {
        std::println("Не удалось определить самую высокую фигуру.");
    }

    if (shapes.size() >= 2) {
        const auto distance_enum = shapes.data_ | ranges::views::enumerate;

        rng::for_each(distance_enum, [distance_enum](const auto& pair) {
            const auto& [i, shape1] = pair;
            for (const auto& [j, shape2] : distance_enum | views::drop(i + 1)) {
                if (auto result = queries::DistanceBetweenShapes(shape1, shape2)) {
                    std::println("Расстояние между {}[{}] & {}[{}] == {:.4f}",
                        utils::PrintShapeName(shape1), i+1,
                        utils::PrintShapeName(shape2), j+1,
                        result.value()
                    );
                }
            }
        });
    }
}

void PerformExtraShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Extra Analysis ===");

    // Shapes higher than 50.0
    auto high_shapes = shapes
        | views::filter([](auto const& s) { return queries::GetHeight(s) > 50.0; })
        | views::take(3);

    for (auto s : high_shapes) {
        std::println("-Высота: {:.2f}", queries::GetHeight(s));
    }

    // Shape with minimum and maximum height
    if (!shapes.empty()) {
        auto min_it = rng::min_element(shapes, {}, [](auto const& s) { return queries::GetHeight(s); });
        auto max_it = rng::max_element(shapes, {}, [](auto const& s) { return queries::GetHeight(s); });

        std::println("Минимальная высота: {:.2f}", queries::GetHeight(*min_it));
        std::println("Максимальная высота: {:.2f}", queries::GetHeight(*max_it));
    }
}

int main() {
    utils::ShapeGenerator generator(-50.0, 50.0, 5.0, 25.0);
    ShapeContainer shapes(generator.GenerateShapes(15));

    std::println("Сгенерировано {} случайных фигур", shapes.size());

    PrintAllIntersections(shapes);
    PrintDistancesFromPointToShapes(Point2D{10.0, 10.0}, shapes);
    PerformShapeAnalysis(shapes);
    PerformExtraShapeAnalysis(shapes.data_);

    visualization::Draw(shapes.data_);


    //========= Convex hull ===========
    if (auto hull_result = convex_hull::GrahamScan(queries::GetAllShapeVertices(shapes))) {
        //create a polygon from resulting points
        shapes.data_.push_back(Shape{Polygon{*hull_result, BboxFromPoints(*hull_result)}});
        visualization::Draw(shapes.data_);
    } else {
        std::println("Ошибка при построении выпуклой оболочки: {}", GeometryErrorString[static_cast<size_t>(hull_result.error())]);
    }

    //========= Delaunay triangulation ===========
    if (auto triangulation = triangulation::DelaunayTriangulation(generator.GeneratePoints(20))) {
        //Make Shape triangles from delaunay triangles
        std::vector<Shape> delaunay_shapes = *triangulation
            | std::views::transform([](const auto& t) { return Shape{Triangle{t.a, t.b, t.c}}; })
            | std::ranges::to<std::vector>();

        visualization::Draw(delaunay_shapes);
    } else {
        std::println("Ошибка триангуляции Делоне: {}", GeometryErrorString[static_cast<size_t>(triangulation.error())]);
    }
}
