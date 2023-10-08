#include "DataFrame.hpp"
#include <cassert>
#include <fstream>
#include <unordered_map>

namespace df {

Series::Series(const json& data)
    : _data(data)
{
}

template <typename T>
T Series::get(std::string_view column) const
{
    return _data[column].get<T>();
}

template int Series::get<int>(std::string_view) const;
template uint64_t Series::get<uint64_t>(std::string_view) const;
template float Series::get<float>(std::string_view) const;
template double Series::get<double>(std::string_view) const;
template std::string Series::get<std::string>(std::string_view) const;

const json& Series::data() const
{
    return _data;
}

DataFrameIterator::DataFrameIterator(json const& data, size_t index)
    : _data(data)
    , _index(index)
{
}

bool DataFrameIterator::operator!=(DataFrameIterator const& other) const
{
    return &_data != &other._data || _index != other._index;
}

DataFrameIterator& DataFrameIterator::operator++()
{
    _index++;
    return *this;
}

Series DataFrameIterator::operator*() const
{
    return getSeries();
}

Series DataFrameIterator::getSeries() const
{
    json series;
    for (size_t i = 0; i < _data["columns"].size(); i++) {
        series[_data["columns"][i]] = _data["data"][_index][i];
    }
    return Series(series);
}

DataFrame::DataFrame(const json& data)
    : _data(data)
{
    assert(data.find("columns") != data.end());
    assert(data.find("data") != data.end());
}

DataFrame DataFrame::query(std::unique_ptr<BooleanExpression> expression) const
{
    std::unordered_map<std::string, size_t> columnMap;
    for (size_t i = 0; i < _data["columns"].size(); i++) {
        columnMap[_data["columns"][i]] = i;
    }

    auto const getOr = [&](const json& col, size_t index) -> json {
        if (col.is_string() && columnMap.find(col) != columnMap.end()) {
            return _data["data"][index][columnMap[col]];
        }
        return json(col);
    };

    json df;
    df["columns"] = _data["columns"];
    df["data"] = json::array();

    for (size_t i = 0; i < _data["data"].size(); i++) {
        auto const rowEvaluator = [&](const json& col1, Operator op, const json& col2) {
            switch (op) {
            case Operator::Equal:
                return getOr(col1, i) == getOr(col2, i);
            case Operator::NotEqual:
                return getOr(col1, i) != getOr(col2, i);
            case Operator::Less:
                return getOr(col1, i) < getOr(col2, i);
            case Operator::LessOrEqual:
                return getOr(col1, i) <= getOr(col2, i);
            case Operator::Greater:
                return getOr(col1, i) > getOr(col2, i);
            case Operator::GreaterOrEqual:
                return getOr(col1, i) >= getOr(col2, i);
            }
            return true;
        };
        if (expression->eval(rowEvaluator)) {
            df["data"].push_back(_data["data"][i]);
        }
    }

    return DataFrame(df);
}

DataFrame DataFrame::queryEq(std::string_view column, const json& value) const
{
    json df;
    df["columns"] = _data["columns"];
    df["data"] = json::array();

    int columnIndex = -1;
    for (size_t i = 0; i < _data["columns"].size(); i++) {
        if (_data["columns"][i] == column) {
            columnIndex = i;
        }
    }

    if (columnIndex == -1) {
        return DataFrame(df);
    }

    for (size_t i = 0; i < _data["data"].size(); i++) {
        if (_data["data"][i][columnIndex] == value) {
            df["data"].push_back(_data["data"][i]);
        }
    }

    return DataFrame(df);
}

size_t DataFrame::size() const
{
    return _data["data"].size();
}

Series DataFrame::first() const
{
    return at(0);
}

Series DataFrame::at(size_t index) const
{
    assert(index < size());
    return *DataFrameIterator(_data, index);
}

DataFrameIterator DataFrame::begin() const
{
    return DataFrameIterator(_data, 0);
}

DataFrameIterator DataFrame::end() const
{
    return DataFrameIterator(_data, _data["data"].size());
}

DataFrame fromJson(const json& data)
{
    return DataFrame(data);
}

DataFrame fromJson(std::string_view path)
{
    std::ifstream file(std::string { path });
    assert(file.is_open());
    return DataFrame(json::parse(file));
}

DataFrame fromCsv(std::string_view path, std::string_view delimiter)
{
    std::ifstream file(std::string { path });
    assert(file.is_open());
    return fromCsv(file, delimiter);
}

DataFrame fromCsv(std::istream& stream, std::string_view delimiter)
{
    json df;
    df["data"] = json::array();

    std::string line;
    size_t lineCount = 0;
    while (std::getline(stream, line)) {
        const auto stringRow = splitString(line, delimiter);
        if (lineCount == 0) {
            df["columns"] = stringRow;
        } else {
            json row = json::array();
            for (const auto& stringValue : stringRow) {
                const json value = json::parse(stringValue, nullptr, false);
                row.push_back(value.is_discarded() ? json(stringValue) : value);
            }
            df["data"].push_back(row);
        }
        lineCount++;
    }

    return DataFrame(df);
}

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
