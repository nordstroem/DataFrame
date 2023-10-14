#include "DataFrame.hpp"
#include <cassert>
#include <fstream>
#include <unordered_map>

namespace df {

namespace {
    json splitToColumnFormat(const auto& data)
    {
        json df;
        std::unordered_map<std::string, size_t> columnMap;
        for (size_t i = 0; i < data["columns"].size(); i++) {
            columnMap[data["columns"][i]] = i;
            df[data["columns"][i]] = json::array();
        }
        for (size_t row = 0; row < data["data"].size(); row++) {
            for (size_t col = 0; col < data["columns"].size(); row++) {
                df[columnMap[data["columns"][col]]].push_back(data["data"][row][col]);
            }
        }
        return df;
    }
}

Series::Series(const json& data)
    : _data(data)
{
}

const json& Series::data() const
{
    return _data;
}

DataFrameIterator::DataFrameIterator(const json& data, size_t index)
    : _data(data)
    , _index(index)
{
}

bool DataFrameIterator::operator!=(const DataFrameIterator& other) const
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
    for (const auto& column : _data.items()) {
        series[column.key()] = _data[column.key()][_index];
    }
    return Series(series);
}

DataFrame::DataFrame(const json& data)
{
    const bool isSplitFormat = data.find("columns") != data.end() && data.find("data") != data.end();
    _data = isSplitFormat ? splitToColumnFormat(data) : data;

    _size = 0;
    for (const auto& column : _data) {
        assert(_size == 0 || _size == column.size());
        _size = column.size();
    }
}

DataFrame DataFrame::query(std::unique_ptr<BooleanExpression> expression) const
{
    auto const getOr = [&](const json& col, size_t index) -> const json& {
        if (col.is_string() && _data.find(col) != _data.end()) {
            return _data[col][index];
        }
        return col;
    };

    json df;
    for (size_t row = 0; row < size(); row++) {
        const auto rowEvaluator = [&](const json& col1, Operator op, const json& col2) {
            switch (op) {
            case Operator::Equal:
                return getOr(col1, row) == getOr(col2, row);
            case Operator::NotEqual:
                return getOr(col1, row) != getOr(col2, row);
            case Operator::Less:
                return getOr(col1, row) < getOr(col2, row);
            case Operator::LessOrEqual:
                return getOr(col1, row) <= getOr(col2, row);
            case Operator::Greater:
                return getOr(col1, row) > getOr(col2, row);
            case Operator::GreaterOrEqual:
                return getOr(col1, row) >= getOr(col2, row);
            }
            return true;
        };
        if (expression->eval(rowEvaluator)) {
            for (const auto& col : _data.items()) {
                df[col.key()].push_back(col.value()[row]);
            }
        }
    }

    return DataFrame(df);
}

DataFrame DataFrame::queryEq(std::string_view column, const json& value) const
{
    json df;
    if (_data.find(column) == _data.end()) {
        return DataFrame(df);
    }

    for (const auto& col : _data.items()) {
        df[col.key()] = json::array();
    }

    const json& columnOfInterest = _data[column];

    for (size_t row = 0; row < size(); row++) {
        if (columnOfInterest[row] == value) {
            for (const auto& col : _data.items()) {
                df[col.key()].push_back(col.value()[row]);
            }
        }
    }

    return DataFrame(df);
}

size_t DataFrame::size() const
{
    return _size;
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
    return DataFrameIterator(_data, _size);
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
    std::string line;
    size_t lineCount = 0;
    std::unordered_map<size_t, std::string> columnMap;

    while (std::getline(stream, line)) {
        const auto stringRow = splitString(line, delimiter);
        if (lineCount == 0) {
            for (size_t i = 0; i < stringRow.size(); i++) {
                df[stringRow[i]] = json::array();
                columnMap[i] = stringRow[i];
            }
        } else {
            for (size_t i = 0; i < stringRow.size(); i++) {
                const json value = json::parse(stringRow[i], nullptr, false);
                df[columnMap[i]].push_back(value.is_discarded() ? json(columnMap[i]) : value);
            }
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
