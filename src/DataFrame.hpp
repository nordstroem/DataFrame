#include "BooleanExpression.hpp"
#include <nlohmann/json.hpp>

namespace df {

using json = nlohmann::json;

class Series {
public:
    explicit Series(const json& data);

    template <typename T>
    T get(const std::string& column) const
    {
        return _data[column].get<T>();
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

    DataFrameIterator(json const& data, size_t index);
    bool operator!=(DataFrameIterator const& other) const;

    DataFrameIterator& operator++();
    Series operator*() const;

private:
    Series getSeries() const;
    json const& _data;
    size_t _index;
};

class DataFrame {
public:
    DataFrame(const json& data);
    DataFrame query(std::unique_ptr<BooleanExpression> expression) const;
    DataFrameIterator begin() const;
    DataFrameIterator end() const;

private:
    json _data;
};

}
