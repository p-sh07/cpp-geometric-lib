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

    auto intersectible = shapes.shapes | views::filter([](const auto &shape) {
                             return std::holds_alternative<Line>(shape) || std::holds_alternative<Circle>(shape);
                         });  // | views_v3::enumerate; - sadly doesn't compile for obscure reasons

    auto intersectible_enum =
        intersectible | views_v3::enumerate;  // must be an lvalue to use with ranges_v3, otherwise compile err.

    // Print if has intersection
    rng::for_each(intersectible_enum, [&intersectible](const auto &pair) {
        auto [i, shape1] = pair;
        for (auto shape2 : intersectible | views::drop(i + 1)) {
            if (auto result = intersections::GetIntersectPoint(shape1, shape2); result.has_value()) {
                std::println("{}", result.value());
            } else {
                std::println("-no intersection");
            }
        }
    });

}

void PrintDistancesFromPointToShapes(Point2D p, ShapeContainer shapes) {
    std::println("\n=== Distance from Point Test ===");
    std::println("Testing point: {} ", p);

    /*
     * Используйте ranges чтобы выбрать любые 5 фигур из списка.
     * Затем найдите расстояния от заданной точки до всех выбранных фигур.
     * Выведите результат в формате "Расстояние от точки P до фигуры S равно D"
     */
}

void PerformShapeAnalysis(ShapeContainer shapes) {
    std::println("\n=== Shape Analysis ===");

    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Найти все пересечения между фигурами используя метод Bounding Box
     *     - Найти самую высокую фигуру (чья высота наибольшая)
     *     - Вывести расстояние между любыми двумя фигурами, которые поддерживают данную функциональность
     */
}

void PerformExtraShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Extra Analysis ===");

    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Вывести 3 любые фигуры, которые находятся выше 50.0
     *     - Вывести фигуры с наименьшей и с наибольшей высотами
     */
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