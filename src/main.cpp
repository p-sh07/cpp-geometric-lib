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

void PrintAllIntersections(const ShapeContainer& shapes) {
    std::println("\n=== Intersections ===");

    //Filter only valid shapes & enum [i, shape] to iterate all pairs efficiently with drop
    auto intersect_enum = shapes.GetIntersectibleEnum();

    rng::for_each(intersect_enum, [&intersect_enum](const auto &pair) {
        const auto& [i, shape1] = pair;
        for (const auto& [j, shape2] : intersect_enum | views::drop(i + 1)) {
            if (auto result = intersections::GetIntersectPoint(shape1, shape2); result.has_value()) {
                std::println("{}[{}] & {}[{}] intersect at: {}",
                    std::holds_alternative<Line>(shape1) ? "Line"sv : "Circle"sv, i+1,
                    std::holds_alternative<Line>(shape2) ? "Line"sv : "Circle"sv, j+1,
                    result.value()
                );
            }
        }
    });

}
void PrintDistancesFromPointToShapes(Point2D p, ShapeContainer shapes) {
    std::println("\n=== Distance from Point Test ===");
    std::println("Testing point: {}", p);

    // Take any 5 shapes
    auto limited_shapes = shapes | views::take(5);

    // Compute distances and print results
    rng::for_each(limited_shapes, [p](const auto& shape) {
        double dist = DistanceToPoint(shape, p);
        std::string_view type =
            std::visit([](auto const& s) -> std::string_view {
                using T = std::decay_t<decltype(s)>;
                if constexpr (std::is_same_v<T, Line>) return "Line";
                else if constexpr (std::is_same_v<T, Triangle>) return "Triangle";
                else if constexpr (std::is_same_v<T, Rectangle>) return "Rectangle";
                else if constexpr (std::is_same_v<T, RegularPolygon>) return "RegularPolygon";
                else if constexpr (std::is_same_v<T, Circle>) return "Circle";
                else if constexpr (std::is_same_v<T, Polygon>) return "Polygon";
                else return "Unknown";
            }, shape);

        std::println("Расстояние от точки {} до фигуры {} равно {:.4f}", p, type, dist);
    });
}

void PerformShapeAnalysis(ShapeContainer shapes) {
    std::println("\n=== Shape Analysis ===");

    // --- Find all collisions using bounding boxes ---
    auto collisions = utils::FindAllCollisions(shapes);
    if (collisions.empty()) {
        std::println("Нет пересечений между фигурами.");
    } else {
        std::println("Обнаружено {} пересечений:", collisions.size());
        for (const auto& [s1, s2] : collisions) {
            std::println("  - {} & {} пересекаются",
                std::visit([](auto const& s){ return typeid(s).name(); }, s1),
                std::visit([](auto const& s){ return typeid(s).name(); }, s2));
        }
    }

    // --- Find the tallest shape ---
    if (auto tallest = utils::FindHighestShape(shapes)) {
        std::println("Самая высокая фигура: #{}", *tallest + 1);
    } else {
        std::println("Не удалось определить самую высокую фигуру.");
    }

    if (shapes.size() >= 2) {
        if (auto dist = queries::DistanceBetweenShapes(shapes[0], shapes[1]); dist.has_value()) {
            std::println("Расстояние между фигурами 1 и 2: {:.4f}", dist.value());
        } else {
            std::println("Расстояние между фигурами 1 и 2 не поддерживается.");
        }
    }
}

void PerformExtraShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Extra Analysis ===");

    // --- Shapes higher than 50.0 ---
    auto high_shapes = shapes
        | views::filter([](auto const& pair) {
            const auto& [i, s] = pair;
            return GetHeight(s) > 50.0;
        })
        | views::take(3);

    for (auto s : high_shapes)
        std::println("-Высота: {:.2f}", queries::GetHeight(s));

    // --- Shape with minimum and maximum height ---
    if (!shapes.empty()) {
        auto min_it = rng::min_element(shapes, {}, [](auto const& s) { return queries::GetHeight(s); });
        auto max_it = rng::max_element(shapes, {}, [](auto const& s) { return queries::GetHeight(s); });

        std::println("Минимальная высота: {:.2f}", queries::GetHeight(*min_it));
        std::println("Максимальная высота: {:.2f}", queries::GetHeight(*max_it));
    }
}

int main() {
    utils::ShapeGenerator generator(-50.0, 50.0, 5.0, 25.0);
    std::vector<Shape> shapes = generator.GenerateShapes(15);

    std::println("Generated {} random shapes", shapes.size());

    std::vector<Point2D> vec{{0,0}, {1.223424, 213}, {2.42, 5.24}, {123.2, 3444.1}};
    std::println("{}", vec);
    std::println("{:new_line}", vec);

    // Выведите индекс каждой фигуры и её высоту

    //
    // Вызываем разработанные функции
    //
    PrintAllIntersections(shapes);

    PrintDistancesFromPointToShapes(Point2D{10.0, 10.0}, shapes);

    PerformShapeAnalysis(shapes);

    PerformExtraShapeAnalysis(shapes);

    //
    // Рисуем все фигуры
    //
    // Важно: после изучения графика - нажмите Enter чтобы продолжить выполнение и построить 2ой график
    //
    geometry::visualization::Draw(shapes);

    //
    // Формируем список из вершин всех фигур
    //
    std::vector<Point2D> points;

    /* ваш код здесь */

    //
    // Находим список точек, для построения выпуклой оболочки - convex hull - алгоритмом Грэхема
    // Создаём из них объект класса `Polygon` и добавляем его в список shapes
    // Рисуем все фигуры
    //

    /* ваш код здесь */

    //
    // после изучения графика - нажмите Enter чтобы продолжить выполнение и построить 3ий график
    //

    {
        std::vector<Point2D> points = {{0, 0}, {10, 0}, {5, 8}, {15, 5}, {2, 12}};

        //
        // Используйте список точек points или свой, чтобы
        // выполнить алгоритм триангуляции Делоне алгоритмом Боуэра-Ватсона
        //
        // После успешного завершения алгоритма - выведите результат для проверки
        // используя geometry::visualization::Draw
        //
    }
    return 0;
}