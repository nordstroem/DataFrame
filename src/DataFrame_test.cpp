#include "DataFrame.hpp"
#include "gtest/gtest.h"
#include <Eigen/Core>

using namespace df;
using namespace nlohmann::literals;
using namespace Eigen;

namespace {
static const json jdf = R"(
        {
            "columns": ["a","b","c"],
            "data": [[1,2,3],[4,5,6]]
        }
    )"_json;

static const DataFrame dataFrame(jdf);
}

TEST(DataFrame, iterateThroughDF)
{
    size_t count = 1;
    for (auto const& row : dataFrame) {
        EXPECT_EQ(row.get<int>("a"), count++);
        EXPECT_EQ(row.get<int>("b"), count++);
        EXPECT_EQ(row.get<int>("c"), count++);
    }
}

TEST(DataFrame, queryVector3f)
{
    const auto row = dataFrame.first();
    const auto v = row.get<Vector3f>("a,b,c");
    EXPECT_EQ(v.x(), 1.0f);
    EXPECT_EQ(v.y(), 2.0f);
    EXPECT_EQ(v.z(), 3.0f);
}

TEST(DataFrame, queryVector2i)
{
    const auto row = dataFrame.first();
    const auto v = row.get<Vector2i>("a,b");
    EXPECT_EQ(v.x(), 1);
    EXPECT_EQ(v.y(), 2);
}

TEST(DataFrame, queryWithExpression)
{
    const auto filteredDF = dataFrame.query("a"_c == 1);
    EXPECT_EQ(filteredDF.size(), 1);
    EXPECT_EQ(filteredDF.first().get<int>("a"), 1);
}

TEST(DataFrame, queryWithOrExpression)
{
    const auto filteredDF = dataFrame.query("a"_c == 1 || "b"_c == 5);
    EXPECT_EQ(filteredDF.size(), 2);
    EXPECT_EQ(filteredDF.at(0).get<int>("a"), 1);
    EXPECT_EQ(filteredDF.at(1).get<int>("a"), 4);
}

TEST(DataFrame, queryWithAndExpression)
{
    const auto filteredDF = dataFrame.query("a"_c == 1 && "b"_c == 5);
    EXPECT_EQ(filteredDF.size(), 0);
}

TEST(DataFrame, queryWithLessExpression)
{
    const auto filteredDF = dataFrame.query("a"_c < 2);
    EXPECT_EQ(filteredDF.size(), 1);
    EXPECT_EQ(filteredDF.first().get<int>("a"), 1);
}

TEST(DataFrame, queryEq)
{
    const auto filteredDF = dataFrame.queryEq("a", 1);
    EXPECT_EQ(filteredDF.size(), 1);
    EXPECT_EQ(filteredDF.first().get<int>("a"), 1);
}
