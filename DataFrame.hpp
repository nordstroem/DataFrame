#pragma once
#include "BooleanExpression.hpp"
#include <cassert>
#include <functional>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

namespace df {

template <typename T>
struct RowConverter {
    static T convert(std::vector<std::string> const& str);
    static T convert(std::vector<std::string>&& str);
};

template <>
struct RowConverter<std::vector<std::string>> {
    static std::vector<std::string> convert(std::vector<std::string> const& str) { return str; }
    static std::vector<std::string> convert(std::vector<std::string>&& str) { return str; }
};

template <>
struct RowConverter<std::string> {
    static std::string convert(std::vector<std::string> const& str) { return str[0]; }
};

template <>
struct RowConverter<float> {
    static float convert(std::vector<std::string> const& str) { return std::stof(str[0]); }
};

template <>
struct RowConverter<int> {
    static float convert(std::vector<std::string> const& str) { return std::stoi(str[0]); }
};

template <>
struct RowConverter<size_t> {
    static size_t convert(std::vector<std::string> const& str) { return std::stoul(str[0]); }
};

class Series {
public:
    void set(std::string const& column, std::string const& value);

    template <typename... Columns>
    Series loc(Columns... columns) const
    {
        Series result;
        for (auto const& columnName : { columns... }) {
            assert(_headerMap.find(columnName) != _headerMap.end());
            result.set(columnName, _data[_headerMap.at(columnName)]);
        }
        return result;
    }

    template <typename T = std::vector<std::string>>
    T get() const
    {
        return RowConverter<T>::convert(_data);
    }

    template <typename T = std::string>
    T get(std::string const& column) const
    {
        return RowConverter<T>::convert(loc(column)._data);
    }

    template <typename T, typename... Columns>
    T get(Columns... columns) const
    {
        return RowConverter<T>::convert(loc(columns...)._data);
    }

    size_t size() const;

    friend class DataFrame;

private:
    std::vector<std::string> _header;
    std::vector<std::string> _data;
    std::unordered_map<std::string, int> _headerMap;
};

class DataFrame;

struct TableIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Series;
    using pointer = value_type*;
    using reference = value_type&;

    TableIterator(DataFrame const& table, size_t index);
    bool operator!=(TableIterator const& other) const;

    TableIterator& operator++();
    Series operator*() const;
    Series operator->() const;

private:
    DataFrame const& _table;
    size_t _index;
};

template <typename T>
struct SeriesConverter {
    static T convert(Series const& series) { return series.get<T>(); }
};

template <>
struct SeriesConverter<Series> {
    static Series convert(Series const& series) { return series; }
};

class DataFrame {
public:
    DataFrame(std::string_view fileName, std::vector<std::string> const& header = {}, std::string_view delimiter = ",");

    template <typename... Columns>
    DataFrame loc(Columns... columns) const
    {
        DataFrame result({ columns... });
        for (size_t i = 0; i < _table.size(); ++i) {
            std::vector<std::string> row;
            row.reserve(sizeof...(columns));
            for (auto const& columnName : { columns... }) {
                auto it = _headerMap.find(columnName);
                assert(it != _headerMap.end());
                row.push_back(_table[i][it->second]);
            }
            result.addRow(std::move(row));
        }
        return result;
    }

    template <typename T = Series>
    T get(size_t index) const
    {
        assert(index < _table.size());
        Series row;
        for (size_t i = 0; i < _header.size(); ++i) {
            row.set(_header[i], _table[index][i]);
        }
        return SeriesConverter<T>::convert(row);
    }

    template <typename T = std::string>
    T get(size_t index, std::string const& column) const
    {
        assert(index < _table.size());
        assert(_headerMap.find(column) != _headerMap.end());
        Series row;
        size_t const columnIndex = _headerMap.at(column);
        row.set(_header[columnIndex], _table[index][columnIndex]);
        return SeriesConverter<T>::convert(row);
    }

    template <typename T = Series, typename... Columns>
    T get(size_t index, Columns... columns) const
    {
        assert(index < _table.size());

        Series row;
        for (auto const& columnName : { columns... }) {
            assert(_headerMap.find(columnName) != _headerMap.end());
            row.set(columnName, _table[index][_headerMap.at(columnName)]);
        }
        return SeriesConverter<T>::convert(row);
    }

    DataFrame query(std::function<bool(Series const&)> const& predicate) const;
    DataFrame query(std::unique_ptr<BooleanExpression> expression) const;

    std::vector<std::string> header() const;
    size_t size() const;
    TableIterator begin() const;
    TableIterator end() const;

private:
    void addRow(std::vector<std::string>&& row);
    explicit DataFrame(std::vector<std::string> const& header);

    std::vector<std::string> _header;
    std::unordered_map<std::string, size_t> _headerMap;
    std::vector<std::vector<std::string>> _table;
};

} // namespace df
