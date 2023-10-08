#include "DataFrame.hpp"
#include <cassert>
#include <unordered_map>

namespace df {

Series::Series(const json& data)
    : _data(data)
{
}

template <typename T>
T Series::get(const std::string& column) const
{
    return _data[column].get<T>();
}

template int Series::get<int>(const std::string&) const;
template uint64_t Series::get<uint64_t>(const std::string&) const;
template float Series::get<float>(const std::string&) const;
template double Series::get<double>(const std::string&) const;
template std::string Series::get<std::string>(const std::string&) const;

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

DataFrame DataFrame::queryEq(const std::string& column, json value) const
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

}
