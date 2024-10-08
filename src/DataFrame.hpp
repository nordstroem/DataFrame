#pragma once
#include "BooleanExpression.hpp"
#include <istream>
#include <memory>
#include <nlohmann/json.hpp>

namespace jdf {

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
    /**
     * Constructs a new DataFrame.
     * @param data A json object in one of the following formats:
     * 1. Split format:
     *   {
     *    "columns": ["col1", "col2", ...],
     *    "data": [[1, 2, ...], [3, 4, ...], ...]
     *   }
     * 2. Column format:
     *   {
     *    "col1": [1, 3, ...],
     *    "col2": [2, 4, ...],
     *    ...
     *   }
     * 3. Array: (empty with only column names)
     *   ["col1", "col2", ...]
     * @note DataFrames can be constructed directly from a json or csv file by using the
     * free functions fromJson and fromCsv.
     */
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

/**
 * A wrapper around a DataFrame that writes the DataFrame to a file, in csv format, when it goes out of scope.
 */
class DataFrameWriter {
public:
    DataFrameWriter(const json& data, std::string_view path);
    ~DataFrameWriter();

    DataFrameWriter(const DataFrameWriter&) = delete;
    DataFrameWriter(DataFrameWriter&&) = delete;
    DataFrameWriter& operator=(const DataFrameWriter&) = delete;
    DataFrameWriter& operator=(DataFrameWriter&&) = delete;

    DataFrame* operator->();
    DataFrame const* operator->() const;

private:
    DataFrame _dataFrame;
    std::string _path;
};

}
