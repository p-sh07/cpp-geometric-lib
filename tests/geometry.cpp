//
// Created by Pavel on 20.10.2025.
//
#include "geometry.hpp"
#include <gtest/gtest.h>

using namespace geometry;
// ***Т.к. это очень муторно и нудно (и времени вечно не хватает), сгенерировал с помощью Алисы =)
// Тестирование Point2D
TEST(Point2DTest, BasicOperations) {
    Point2D p1(1.0, 2.0);
    Point2D p2(3.0, 4.0);
    Point2D p3(3.0, 3.0);

    // Сравнение
    EXPECT_TRUE(p1 < p2);
    EXPECT_FALSE(p2 < p1);
    EXPECT_TRUE(p3 < p2);
    EXPECT_TRUE(p1 == Point2D(1.0, 2.0));

    // Арифметические операции
    EXPECT_DOUBLE_EQ((p1 + p2).x, 4.0);
    EXPECT_DOUBLE_EQ((p1 + p2).y, 6.0);

    EXPECT_DOUBLE_EQ((p2 - p1).x, 2.0);
    EXPECT_DOUBLE_EQ((p2 - p1).y, 2.0);

    EXPECT_DOUBLE_EQ((p1 * 2.0).x, 2.0);
    EXPECT_DOUBLE_EQ((p1 * 2.0).y, 4.0);

    EXPECT_DOUBLE_EQ((p2 / 2.0).x, 1.5);
    EXPECT_DOUBLE_EQ((p2 / 2.0).y, 2.0);

    // Геометрические операции
    EXPECT_DOUBLE_EQ(p1.Dot(p2), 11.0);
    EXPECT_DOUBLE_EQ(p1.Cross(p2), -2.0);
    EXPECT_DOUBLE_EQ(p1.Length(), std::sqrt(5.0));
    EXPECT_DOUBLE_EQ(p1.DistanceTo(p2), std::sqrt(8.0));

    // Нормализация
    Point2D normalized = p1.Normalize();
    EXPECT_DOUBLE_EQ(normalized.Length(), 1.0);
}

// Тестирование BoundingBox
TEST(BoundingBoxTest, Overlap) {
    BoundingBox box1(0.0, 0.0, 10.0, 10.0);
    BoundingBox box2(5.0, 5.0, 15.0, 15.0);
    BoundingBox box3(12.0, 12.0, 15.0, 15.0);

    EXPECT_TRUE(box1.Overlaps(box2));
    EXPECT_FALSE(box1.Overlaps(box3));

    EXPECT_DOUBLE_EQ(box1.Width(), 10.0);
    EXPECT_DOUBLE_EQ(box1.Height(), 10.0);

    EXPECT_DOUBLE_EQ(box1.Center().x, 5.0);
    EXPECT_DOUBLE_EQ(box1.Center().y, 5.0);
}

// Тестирование Line
TEST(LineTest, Properties) {
    Line line(Point2D(0.0, 0.0), Point2D(3.0, 4.0));

    EXPECT_DOUBLE_EQ(line.Length(), 5.0);

    Point2D direction = line.Direction();
    EXPECT_DOUBLE_EQ(direction.x, 0.6);
    EXPECT_DOUBLE_EQ(direction.y, 0.8);

    EXPECT_DOUBLE_EQ(line.Center().x, 1.5);
    EXPECT_DOUBLE_EQ(line.Center().y, 2.0);
}

// Тестирование Triangle
TEST(TriangleTest, Properties) {
    Triangle triangle(Point2D(0.0, 0.0), Point2D(3.0, 0.0), Point2D(0.0, 4.0));

    EXPECT_DOUBLE_EQ(triangle.Area(), 6.0);

    EXPECT_DOUBLE_EQ(triangle.Center().x, 1.0);
    EXPECT_DOUBLE_EQ(triangle.Center().y, 1.3333333333333333);

    EXPECT_DOUBLE_EQ(triangle.Height(), 4.0);
}

// Тестирование Rectangle
TEST(RectangleTest, BasicProperties) {
    Rectangle rect(Point2D(0.0, 0.0), 4.0, 3.0);

    // Проверка вершин
    auto vertices = rect.Vertices();
    EXPECT_DOUBLE_EQ(vertices[0].x, 0.0);
    EXPECT_DOUBLE_EQ(vertices[0].y, 0.0);
    EXPECT_DOUBLE_EQ(vertices[1].x, 4.0);
    EXPECT_DOUBLE_EQ(vertices[1].y, 0.0);
    EXPECT_DOUBLE_EQ(vertices[2].x, 4.0);
    EXPECT_DOUBLE_EQ(vertices[2].y, 3.0);
    EXPECT_DOUBLE_EQ(vertices[3].x, 0.0);
    EXPECT_DOUBLE_EQ(vertices[3].y, 3.0);

    // Проверка центра
    EXPECT_DOUBLE_EQ(rect.Center().x, 2.0);
    EXPECT_DOUBLE_EQ(rect.Center().y, 1.5);

    // Проверка ограничивающего прямоугольника
    auto bbox = rect.BoundBox();
    EXPECT_DOUBLE_EQ(bbox.min_x, 0.0);
    EXPECT_DOUBLE_EQ(bbox.min_y, 0.0);
    EXPECT_DOUBLE_EQ(bbox.max_x, 4.0);
    EXPECT_DOUBLE_EQ(bbox.max_y, 3.0);
}

// Тестирование RegularPolygon
TEST(RegularPolygonTest, Hexagon) {
    // Шестиугольник с центром в начале координат
    RegularPolygon hexagon(Point2D(0.0, 0.0), 5.0, 6);

    // Проверка центра
    EXPECT_DOUBLE_EQ(hexagon.Center().x, 0.0);
    EXPECT_DOUBLE_EQ(hexagon.Center().y, 0.0);

    // Проверка количества вершин
    auto vertices = hexagon.Vertices();
    EXPECT_EQ(vertices.size(), 6);

    // Проверка первой вершины (должна быть на оси X)
    EXPECT_DOUBLE_EQ(vertices[0].x, 5.0);
    EXPECT_DOUBLE_EQ(vertices[0].y, 0.0);

    // Проверка второй вершины (60 градусов)
    EXPECT_DOUBLE_EQ(vertices[1].x, 2.5);
    EXPECT_DOUBLE_EQ(vertices[1].y, 4.330127018922193);  // 5 * sin(60°)

    // Проверка высоты
    EXPECT_DOUBLE_EQ(hexagon.Height(), 5.0);
}

// Тестирование Circle
TEST(CircleTest, BasicProperties) {
    Circle circle(Point2D(0.0, 0.0), 5.0);

    // Проверка центра
    EXPECT_DOUBLE_EQ(circle.Center().x, 0.0);
    EXPECT_DOUBLE_EQ(circle.Center().y, 0.0);

    // Проверка радиуса
    EXPECT_DOUBLE_EQ(circle.Radius(), 5.0);

    // Проверка ограничивающего прямоугольника
    auto bbox = circle.BoundBox();
    EXPECT_DOUBLE_EQ(bbox.min_x, -5.0);
    EXPECT_DOUBLE_EQ(bbox.min_y, -5.0);
    EXPECT_DOUBLE_EQ(bbox.max_x, 5.0);
    EXPECT_DOUBLE_EQ(bbox.max_y, 5.0);

    // Проверка высоты
    EXPECT_DOUBLE_EQ(circle.Height(), 5.0);
}


// Тестирование класса Polygon
TEST(PolygonTest, BasicFunctionality) {
    // Создаем простой четырехугольник
    std::vector<Point2D> points = {
        Point2D(0.0, 0.0),
        Point2D(2.0, 0.0),
        Point2D(2.0, 2.0),
        Point2D(0.0, 2.0)
    };

    Polygon poly(points);

    // Проверка количества вершин
    EXPECT_EQ(poly.Vertices().size(), 4);

    // Проверка центра
    Point2D center = poly.Center();
    EXPECT_DOUBLE_EQ(center.x, 1.0);
    EXPECT_DOUBLE_EQ(center.y, 1.0);

    // Проверка высоты
    EXPECT_DOUBLE_EQ(poly.Height(), 2.0);

    // Проверка ограничивающего прямоугольника
    BoundingBox bbox = poly.BoundBox();
    EXPECT_DOUBLE_EQ(bbox.min_x, 0.0);
    EXPECT_DOUBLE_EQ(bbox.min_y, 0.0);
    EXPECT_DOUBLE_EQ(bbox.max_x, 2.0);
    EXPECT_DOUBLE_EQ(bbox.max_y, 2.0);
}

TEST(PolygonTest, EmptyPolygon) {
    // Создаем пустой полигон
    std::vector<Point2D> empty_points;
    Polygon empty_poly(empty_points);

    // Проверка пустых данных
    EXPECT_EQ(empty_poly.Vertices().size(), 0);
    EXPECT_DOUBLE_EQ(empty_poly.Height(), 0.0);

    // Проверка значений bounding box для пустого полигона
    BoundingBox empty_bbox = empty_poly.BoundBox();
    EXPECT_DOUBLE_EQ(empty_bbox.min_x, 0.0);
    EXPECT_DOUBLE_EQ(empty_bbox.min_y, 0.0);
    EXPECT_DOUBLE_EQ(empty_bbox.max_x, 0.0);
    EXPECT_DOUBLE_EQ(empty_bbox.max_y, 0.0);
}

