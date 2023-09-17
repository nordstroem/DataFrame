#include "DataFrame.hpp"
#include "doctest.h"
#include <fstream>

namespace df {

namespace {
    std::vector<std::string> splitString(std::string str, std::string_view delimiter)
    {
        std::vector<std::string> row;
        size_t pos = 0;
        std::string token;
        while ((pos = str.find(delimiter.data())) != std::string::npos) {
            token = str.substr(0, pos);
            row.push_back(token);
            str.erase(0, pos + delimiter.length());
        }
        row.push_back(str);
        return row;
    }

}

void Series::set(std::string const& column, std::string const& value)
{
    if (_headerMap.find(column) == _headerMap.end()) {
        _data.push_back(value);
        _headerMap[column] = _header.size();
        _header.push_back(column);
    } else {
        _data[_headerMap[column]] = value;
    }
}

size_t Series::size() const { return _header.size(); }

TableIterator::TableIterator(DataFrame const& table, size_t index)
    : _table(table)
    , _index(index)
{
}

bool TableIterator::operator!=(TableIterator const& other) const { return _index != other._index || &_table != &other._table; }

TableIterator& TableIterator::operator++()
{
    _index++;
    return *this;
}

Series TableIterator::operator*() const { return _table.get(_index); }

Series TableIterator::operator->() const { return _table.get(_index); }

DataFrame::DataFrame(std::string_view fileName,
    std::vector<std::string> const& header,
    std::string_view delimiter)
    : _header(header)
{
    bool const hasHeader = header.size() == 0;
    std::ifstream file(fileName.data());
    assert(file.is_open());
    std::string line;
    size_t lineCount = 0;
    while (std::getline(file, line)) {
        std::vector<std::string> const row = splitString(line, delimiter);
        if (hasHeader && lineCount == 0) {
            _header = row;
        } else {
            assert(row.size() == _header.size());
            _table.push_back(row);
        }

        lineCount++;
    }

    for (size_t i = 0; i < _header.size(); ++i) {
        _headerMap[_header[i]] = i;
    }
}

DataFrame DataFrame::query(std::function<bool(Series const&)> const& predicate) const
{
    DataFrame result(_header);
    for (Series row : *this) {
        if (predicate(row)) {
            result.addRow(std::move(row._data));
        }
    }

    return result;
}

std::vector<std::string> DataFrame::header() const { return _header; }

size_t DataFrame::size() const { return _table.size(); }

TableIterator DataFrame::begin() const { return TableIterator(*this, 0); }

TableIterator DataFrame::end() const { return TableIterator(*this, _table.size()); }

void DataFrame::addRow(std::vector<std::string>&& row)
{
    assert(row.size() == _header.size());
    _table.push_back(row);
}

DataFrame::DataFrame(std::vector<std::string> const& header)
    : _header(header)
{
    for (size_t i = 0; i < _header.size(); ++i) {
        _headerMap[_header[i]] = i;
    }
}

TEST_CASE("getFullRow")
{

    DataFrame df("test.csv", { "a", "b", "c" });
    Series const row = df.get(0);
    CHECK_EQ(row.get("a"), "1");
    CHECK_EQ(row.get("b"), "2");
    CHECK_EQ(row.get("c"), "3");
}

TEST_CASE("getRowSubset")
{
    DataFrame df("test.csv", { "a", "b", "c" });
    auto const row = df.get(0, "a", "c");
    CHECK_EQ(row.get("a"), "1");
    CHECK_EQ(row.get("c"), "3");
}

TEST_CASE("query")
{
    DataFrame df("test.csv", { "a", "b", "c" });
    auto const filtereddf = df.query([](Series const& row) { return row.get<int>("a") == 1 && row.get<int>("b") == 2; });
    auto const row = filtereddf.get(0);
    CHECK_EQ(row.get("a"), "1");
    CHECK_EQ(row.get("b"), "2");
    CHECK_EQ(row.get("c"), "3");
}

TEST_CASE("getRows")
{
    DataFrame df("test.csv", { "a", "b", "c" });
    auto const newdf = df.loc("a", "c");
    CHECK_EQ(newdf.size(), 2);
    CHECK_EQ(newdf.header(), std::vector<std::string>({ "a", "c" }));
}

TEST_CASE("getValue")
{
    DataFrame df("test.csv", { "a", "b", "c" });
    CHECK_EQ(df.get<int>(0, "a"), 1);
    CHECK_EQ(df.get<int>(0, "b"), 2);
    CHECK_EQ(df.get<std::string>(0, "c"), "3");
    CHECK_EQ(df.get<float>(1, "a"), 4.0f);
}

TEST_CASE("getSeriesSubset")
{
    Series series;
    series.set("a", "1.0");
    series.set("b", "2.3");
    series.set("c", "3.5");
    Series const subset = series.loc("a", "c");
    CHECK_EQ(subset.size(), 2);
    CHECK_EQ(subset.get("a"), "1.0");
    CHECK_EQ(subset.get("c"), "3.5");
}

TEST_CASE("iterateThroughdf")
{
    DataFrame const df("test_with_header.csv");
    size_t count = 1;
    for (auto const& row : df) {
        CHECK_EQ(row.get<int>("a"), count++);
        CHECK_EQ(row.get<int>("b"), count++);
        CHECK_EQ(row.get<int>("c"), count++);
    }
}

TEST_CASE("query")
{
    DataFrame const df("test_with_header.csv");
    const auto filteredDF = df.query([](const Series& row) { return row.get<int>("a") == 1; });
    CHECK_EQ(filteredDF.size(), 1);
}

} // namespace df
