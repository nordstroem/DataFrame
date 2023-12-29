#pragma once
#include "BooleanExpression.hpp"
#include <istream>
#include <memory>
#include <nlohmann/json.hpp>

namespace df {

using json = nlohmann::json;

template <typename T>
struct SeriesConverter {
    static T convert(const json& data, std::string_view column)
    {
        return data[column].get<T>();
    }
};

class Series {
public:
    explicit Series(const json& data);
    template <typename T>
    T get(const std::string_view columns) const
    {
        return SeriesConverter<T>::convert(_data, columns);
    }
    const json& data() const;

private:
    json _data;
};

struct DataFrameIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Series;
    using pointer = value_type*;
    using reference = value_type&;

    DataFrameIterator(const json& data, size_t index);
    bool operator!=(const DataFrameIterator& other) const;

    DataFrameIterator& operator++();
    Series operator*() const;

private:
    Series getSeries() const;
    json const& _data;
    size_t _index;
};

class DataFrame {
public:
    explicit DataFrame(const json& data);

    void addRow(const json& row);
    DataFrame query(std::unique_ptr<BooleanExpression> expression) const;
    DataFrame queryEq(std::string_view column, const json& value) const;
    size_t size() const;
    Series at(size_t index) const;
    Series first() const;
    void toCsv(std::ostream& stream, std::string_view delimiter = ",") const;
    void toCsv(std::string_view path, std::string_view delimiter = ",") const;

    DataFrameIterator begin() const;
    DataFrameIterator end() const;

private:
    json _data;
    size_t _size;
};

DataFrame fromJson(std::string_view path);
DataFrame fromJson(const json& data);
DataFrame fromCsv(std::string_view path, std::string_view delimiter = ",");
DataFrame fromCsv(std::istream& stream, std::string_view delimiter = ",");

std::vector<std::string> splitString(std::string str, std::string_view delimiter);

}
