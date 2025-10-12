//
// Created by Pavel on 08.10.2025.
//
#include <gtest/gtest.h>

#include "intersections.hpp"

using namespace geometry;
using intersections::Result;

//========= Simple checks ==========
TEST(FindIntersections, TwoLinesParallel) {
    const Line line_a({0.0, 0.0}, {0.0, 1.0});
    const Line line_b({1.0, 0.0}, {1.0, 1.0});

    EXPECT_EQ(intersections::GetIntersectPoint(line_a, line_b), std::nullopt);
}

TEST(FindIntersections, SameLines) {
    const Line line_a({0.0, 0.0}, {0.0, 1.0});
    const Line line_b({0.0, 0.0}, {0.0, 1.0});

    EXPECT_EQ(intersections::GetIntersectPoint(line_a, line_b), std::nullopt);
}

TEST(FindIntersections, ZeroLine) {
    const Line line_a({0.0, 0.0}, {0.0, 1.0});
    const Line line_b({0.0, 0.0}, {0.0, 0.0});

    EXPECT_EQ(intersections::GetIntersectPoint(line_a, line_b), std::nullopt);
}

TEST(FindIntersections, WrongShapes) {
    const Triangle triangle({0.0, 0.0}, {0.0, 1.0}, {1.0, 1.0});
    const Line line({0.0, 0.0}, {0.0, 1.0});

    EXPECT_THROW(intersections::GetIntersectPoint(triangle, line), std::logic_error);
}

//========= Test Suite ==========
class IntersectionCheck : public testing::TestWithParam<std::tuple<Shape, Shape, Result>> {
protected:
    Result FindIntersection() const { return geometry::intersections::GetIntersectPoint(shape1_, shape2_); }
    Result GetExpectedResult() const { return expected_result_; }

private:
    Shape shape1_{std::get<0>(GetParam())};
    Shape shape2_{std::get<1>(GetParam())};
    Result expected_result_{std::get<2>(GetParam())};
};

TEST_P(IntersectionCheck, CheckCorrectIntersections) {
    const auto result = FindIntersection();
    const auto expected = GetExpectedResult();

    if (!result.has_value()) {
        EXPECT_EQ(expected, std::nullopt); //both nullopt
    } else {
        ASSERT_TRUE(expected.has_value());
        ASSERT_EQ((*result).size(), (*expected).size());

        //Check double equality
        for (size_t i = 0; i < (*result).size(); ++i) {
            EXPECT_DOUBLE_EQ((*result).at(i).x, (*expected).at(i).x);
            EXPECT_DOUBLE_EQ((*result).at(i).y, (*expected).at(i).y);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    FindIntersections, IntersectionCheck, ::testing::Values(
        //No intersection
        std::make_tuple(Line{{0.0, 0.0}, {5.0, 0.0}}, Line{{0.0, 1.0}, {5.0, 1.0}}, Result{}),
        std::make_tuple(Circle{{0.0, 0.0}, 1.0}, Circle{{5.0, 0.0}, 1.0}, Result{}),
        std::make_tuple(Circle{{0.0, 0.0}, 1.0}, Line{{2.0, 0.0}, {2.0, 5.0}}, Result{}),
        //Have intersection(s)
        std::make_tuple(Line{{0.0, 0.0}, {5.0, 5.0}}, Line{{0.0, 5.0}, {5.0, 0.0}}, Result{{{2.5, 2.5}}}),
        std::make_tuple(Line{{5.0, 0.0}, {5.0, 5.0}}, Circle{{5.0, 3.0}, 2}, Result{{{5.0, 1.0}, {5.0, 5.0}}}),
        std::make_tuple(Line{{0.0, 0.0}, {0.0, 3.0}}, Circle{{0.0, 3.0}, 2}, Result{{{0.0, 1.0}}}),
        std::make_tuple(Circle{{0.0, 0.0}, 5.0}, Circle{{6.0, 0.0}, 5.0}, Result{{{3.0, 4.0}, {3.0, -4.0}}})
));
