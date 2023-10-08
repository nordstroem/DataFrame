#include "DataFrame.hpp"
#include <Eigen/Core>
#include <cassert>

namespace df {

using namespace Eigen;

template <>
Vector3f Series::get(const std::string& columns) const
{
    const auto columnSplit = splitString(columns, ",");
    assert(columnSplit.size() == 3);

    return Vector3f(_data[columnSplit[0]],
        _data[columnSplit[1]],
        _data[columnSplit[2]]);
}

template <>
Vector2f Series::get(const std::string& columns) const
{
    const auto columnSplit = splitString(columns, ",");
    assert(columnSplit.size() == 2);

    return Vector2f(_data[columnSplit[0]],
        _data[columnSplit[1]]);
}

template <>
Vector3i Series::get(const std::string& columns) const
{
    const auto columnSplit = splitString(columns, ",");
    assert(columnSplit.size() == 3);

    return Vector3i(_data[columnSplit[0]],
        _data[columnSplit[1]],
        _data[columnSplit[2]]);
}

template <>
Vector2i Series::get(const std::string& columns) const
{
    const auto columnSplit = splitString(columns, ",");
    assert(columnSplit.size() == 2);

    return Vector2i(_data[columnSplit[0]],
        _data[columnSplit[1]]);
}

}
