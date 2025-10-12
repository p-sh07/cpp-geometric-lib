//
// Created by Pavel on 11.10.2025.
//
#include <gtest/gtest.h>

#include "queries.hpp"

using namespace geometry;
using namespace geometry::queries;

using Result = std::optional<double>;

TEST(FindDistances, TwoLinesParallel) {
    const Point2D pt{0.0, 0.0};
    const Line line({1.0, 0.0}, {1.0, 1.0});

    EXPECT_EQ(GetDistanceToLine(pt, line), 1.0);
}


//========= Test Suite: Point2D–Shape Distances ==========
class PointToShapeDistanceCheck : public testing::TestWithParam<std::tuple<Point2D, Shape, double>> {
protected:
    double ComputeDistance() const {
        return DistanceToPoint(shape_, point_);
    }

    double GetExpectedDistance() const {
        return expected_distance_;
    }

private:
    Point2D point_{std::get<0>(GetParam())};
    Shape shape_{std::get<1>(GetParam())};
    double expected_distance_{std::get<2>(GetParam())};
};

TEST_P(PointToShapeDistanceCheck, PointToShapeDist) {
    const double result = ComputeDistance();
    const double expected = GetExpectedDistance();
    EXPECT_DOUBLE_EQ(result, expected);
}

INSTANTIATE_TEST_SUITE_P(
    PointToShapeDist,
    PointToShapeDistanceCheck,
    ::testing::Values(
        // Point2D–Line
        std::make_tuple(Point2D{0.0, 0.0}, Shape{Line{{0.0, 5.0}, {5.0, 5.0}}}, 5.0),
        std::make_tuple(Point2D{0.0, 0.0}, Shape{Line{{0.0, 3.0}, {0.0, 5.0}}}, 3.0),
        std::make_tuple(Point2D{2.0, 2.0}, Shape{Line{{0.0, 0.0}, {4.0, 0.0}}}, 2.0),
        std::make_tuple(Point2D{1.0, 1.0}, Shape{Line{{0.0, 0.0}, {2.0, 2.0}}}, 0.0),

        // Point2D–Circle
        std::make_tuple(Point2D{0.0, 0.0}, Shape{Circle{{0.0, 0.0}, 1.0}}, 0.0),
        std::make_tuple(Point2D{0.0, 0.0}, Shape{Circle{{3.0, 0.0}, 1.0}}, 2.0),
        std::make_tuple(Point2D{5.0, 5.0}, Shape{Circle{{0.0, 0.0}, 2.0}}, std::sqrt(50.0) - 2.0),

        // Point2D–Rectangle
        std::make_tuple(Point2D{0.5, 0.5}, Shape{Rectangle{{0.0, 0.0}, 1.0, 1.0}}, 0.5), //TODO: if pt inside shape, dist == 0?
        std::make_tuple(Point2D{2.0, 0.5}, Shape{Rectangle{{0.0, 0.0}, 1.0, 1.0}}, 1.0),
        std::make_tuple(Point2D{2.0, 2.0}, Shape{Rectangle{{0.0, 0.0}, 1.0, 1.0}}, std::sqrt(2.0)),

        // Point2D–Triangle
        std::make_tuple(Point2D{0.0, 0.0}, Shape{Triangle{{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}}}, 0.0),
        std::make_tuple(Point2D{2.0, 2.0}, Shape{Triangle{{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}}}, std::sqrt(4.5)),
        std::make_tuple(Point2D{0.5, 0.25}, Shape{Triangle{{20.0, 20.0}, {25.0, 20.0}, {20.0, 25.0}}}, std::sqrt(19.5 * 19.5 + 19.75 * 19.75)), //dist to closest pt -> 20,20

        // Point2D–Polygon
        std::make_tuple(
            Point2D{0.5, 0.5},
            Shape{Polygon( std::vector<Point2D>{{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}}, BoundingBox{0.0, 0.0, 1.0, 1.0})},
            0.5
        ),
        std::make_tuple(
            Point2D{2.0, 2.0},
            Shape{Polygon(std::vector<Point2D>{{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}}, BoundingBox{0.0, 0.0, 1.0, 1.0})},
            std::sqrt(2.0)
        ),
        std::make_tuple(
            Point2D{-1.0, 0.5},
            Shape{Polygon(std::vector<Point2D>{{0.0, 0.0}, {2.0, 0.0}, {2.0, 2.0}, {0.0, 2.0}},BoundingBox{0.0, 0.0, 2.0, 2.0})},
            1.0
        )
    )
);

//========= Test Suite: Shape-Shape distances ==========
class DistanceCheck : public testing::TestWithParam<std::tuple<Shape, Shape, Result>> {
protected:
    Result ComputeDistance() const {
        return DistanceBetweenShapes(shape1_, shape2_);
    }

    Result GetExpectedDistance() const {
        return expected_distance_;
    }

private:
    Shape shape1_{std::get<0>(GetParam())};
    Shape shape2_{std::get<1>(GetParam())};
    Result expected_distance_{std::get<2>(GetParam())};
};

TEST_P(DistanceCheck, ShapeToShapeDist) {
    const auto result = ComputeDistance();
    const auto expected = GetExpectedDistance();

    if (!expected.has_value()) {
        EXPECT_EQ(result, std::nullopt);
    } else {
        ASSERT_TRUE(result.has_value());
        EXPECT_DOUBLE_EQ(*result, *expected);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ShapeToShapeDist, DistanceCheck, ::testing::Values(
        // Circle–Circle
        std::make_tuple(Shape{Circle{{0.0, 0.0}, 1.0}}, Shape{Circle{{2.0, 0.0}, 1.0}}, Result{0.0}),
        std::make_tuple(Shape{Circle{{0.0, 0.0}, 1.0}}, Shape{Circle{{5.0, 0.0}, 1.0}}, Result{3.0}),
        std::make_tuple(Shape{Circle{{0.0, 0.0}, 2.0}}, Shape{Circle{{1.0, 0.0}, 2.0}}, Result{0.0}),

        // Line–Line
        std::make_tuple(Shape{Line{{0.0, 0.0}, {10.0, 0.0}}}, Shape{Line{{0.0, 5.0}, {10.0, 5.0}}}, Result{5.0}),
        std::make_tuple(Shape{Line{{0.0, 0.0}, {5.0, 5.0}}}, Shape{Line{{0.0, 5.0}, {5.0, 0.0}}}, Result{0.0}), //TODO: Assume if intersect, dist == 0;
        std::make_tuple(Shape{Line{{0.0, 0.0}, {1.0, 0.0}}}, Shape{Line{{5.0, 5.0}, {6.0, 5.0}}}, Result{std::sqrt(41)}),
        std::make_tuple(Shape{Line{{0.0, 0.0}, {0.0, 0.0}}}, Shape{Line{{3.0, 4.0}, {3.0, 4.0}}}, Result{}),

        // Unsupported types (expect std::nullopt)
        std::make_tuple(Shape{Triangle{{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}}}, Shape{Rectangle{{0.0, 0.0}, 1.0, 1.0}}, Result{}),
        std::make_tuple(Shape{Circle{{0.0, 0.0}, 1.0}}, Shape{Line{{2.0, 0.0}, {2.0, 5.0}}}, Result{})
    )
);


//========= Helper functions test ==========

class QueryHelperFunctions : public testing::TestWithParam<std::tuple<Shape, Shape, BoundingBox, double, bool>> {
protected:
    static BoundingBox ComputeBoundBox(const Shape& s) {
        try { //TODO: Catch doesn't seem to work with Gtest...
            return GetBoundBox(s);
        } catch (std::logic_error& ex) {
            std::println("*Caught exception in test: {}", ex.what());
            return {};
        }
    }
    static double ComputeHeight(const Shape& s) {
        try {
            return GetHeight(s);
        } catch (std::logic_error& ex) {
            std::println("*Caught exception in test: {}", ex.what());
            return 0.0;
        }
    }
    static bool ComputeOverlap(const Shape& s1, const Shape& s2) {
        try {
            return BoundingBoxesOverlap(s1, s2);
        } catch (std::logic_error& ex) {
            std::println("*Caught exception in test: {}", ex.what());
            return false;
        }
    }
};

TEST_P(QueryHelperFunctions, HelperFucntionsCheck) {
    const auto& [shape1, shape2, expected_box, expected_height, expected_overlap] = GetParam();

    // GetBoundBox
    const auto box = GetBoundBox(shape1);
    EXPECT_DOUBLE_EQ(box.min_x, expected_box.min_x);
    EXPECT_DOUBLE_EQ(box.min_y, expected_box.min_y);
    EXPECT_DOUBLE_EQ(box.max_x, expected_box.max_x);
    EXPECT_DOUBLE_EQ(box.max_y, expected_box.max_y);

    // GetHeight
    EXPECT_DOUBLE_EQ(GetHeight(shape1), expected_height);

    // BoundingBoxesOverlap
    EXPECT_EQ(BoundingBoxesOverlap(shape1, shape2), expected_overlap);
}

//TODO: bound box not defined for some shapes, define or ignore std::logic_error? Add BB if needed
INSTANTIATE_TEST_SUITE_P(
    HelperFucntionsCheck, QueryHelperFunctions, ::testing::Values(
        //==== Line ====
       std::make_tuple(
           Shape{Line{{0.0, 0.0}, {2.0, 2.0}}},
           Shape{Line{{1.0, 1.0}, {3.0, 3.0}}},
           BoundingBox(0.0, 0.0, 2.0, 2.0),
           2.0,
           true
       ),

       //==== Triangle ====
       std::make_tuple(
           Shape{Triangle{{0.0, 0.0}, {2.0, 0.0}, {0.0, 3.0}}},
           Shape{Triangle{{3.0, 3.0}, {4.0, 3.0}, {3.0, 4.0}}},
           BoundingBox(0.0, 0.0, 2.0, 3.0),
           3.0,
           false
       ),

       //==== Circle ====
       std::make_tuple(
           Shape{Circle{{1.0, 1.0}, 1.0}},
           Shape{Circle{{2.0, 2.0}, 0.5}},
           BoundingBox(0.0, 0.0, 2.0, 2.0),
           2.0,
           true
       ),

       //==== Rectangle ====
       std::make_tuple(
           Shape{Rectangle{{0.0, 0.0}, 2.0, 1.0}},
           Shape{Rectangle{{3.0, 3.0}, 1.0, 1.0}},
           BoundingBox(0.0, 0.0, 2.0, 1.0),
           1.0,
           false
       ),

       //==== Polygon ====
       std::make_tuple(
           Shape{Polygon{
               std::vector<Point2D>{{0.0, 0.0}, {2.0, 0.0}, {2.0, 1.0}, {0.0, 1.0}},
               BoundingBox(0.0, 0.0, 2.0, 1.0)
           }},
           Shape{Circle{{3.0, 3.0}, 0.5}},
           BoundingBox(0.0, 0.0, 2.0, 1.0),
           1.0,
           false
       ),

       //==== RegularPolygon ====
       std::make_tuple(
           Shape{RegularPolygon{{0.0, 0.0}, 1.0, 6}},
           Shape{RegularPolygon{{1.5, 0.0}, 1.0, 6}},
           BoundingBox(-1.0, -1.0, 1.0, 1.0),
           2.0,
           true
       )
    )
);