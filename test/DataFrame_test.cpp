#include "DataFrame.hpp"
#include "gtest/gtest.h"

using namespace df;
using namespace nlohmann::literals;

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
