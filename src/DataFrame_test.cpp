#include "DataFrame.hpp"
#include "gtest/gtest.h"
#include <Eigen/Core>
#include <iostream>

using namespace df;
using namespace nlohmann::literals;
using namespace Eigen;

namespace {
static const json testJson = R"(
        {
            "columns": ["a","b","c"],
            "data": [[1,2,3],[4,5,6]]
        }
    )"_json;

static const DataFrame dataFrame(testJson);
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

TEST(DataFrame, fromCsv)
{
    const std::string csvFile = "x,y,z\n1,1,1\n2,2,2\n";
    std::stringstream ss;
    ss << csvFile;
    const auto dataFrame = fromCsv(ss);
    EXPECT_EQ(dataFrame.size(), 2);
    EXPECT_EQ(dataFrame.at(0).get<int>("x"), 1);
    EXPECT_EQ(dataFrame.at(1).get<int>("x"), 2);
}