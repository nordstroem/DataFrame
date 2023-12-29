#pragma once
#include "DataFrame.hpp"
#include <Eigen/Core>
#include <cassert>
#include <nlohmann/json.hpp>

namespace jdf {

template <>
struct SeriesConverter<Eigen::Vector3f> {
    static Eigen::Vector3f convert(const json& data, std::string_view columns)
    {
        const auto columnSplit = splitString(std::string(columns), ",");
        assert(columnSplit.size() == 3);

        return Eigen::Vector3f(data[columnSplit[0]],
            data[columnSplit[1]],
            data[columnSplit[2]]);
    }
};

template <>
struct SeriesConverter<Eigen::Vector2f> {
    static Eigen::Vector2f convert(const json& data, std::string_view columns)
    {
        const auto columnSplit = splitString(std::string(columns), ",");
        assert(columnSplit.size() == 2);

        return Eigen::Vector2f(data[columnSplit[0]],
            data[columnSplit[1]]);
    }
};

template <>
struct SeriesConverter<Eigen::Vector3i> {
    static Eigen::Vector3i convert(const json& data, std::string_view columns)
    {
        const auto columnSplit = splitString(std::string(columns), ",");
        assert(columnSplit.size() == 3);

        return Eigen::Vector3i(data[columnSplit[0]],
            data[columnSplit[1]],
            data[columnSplit[2]]);
    }
};

template <>
struct SeriesConverter<Eigen::Vector2i> {
    static Eigen::Vector2i convert(const json& data, std::string_view columns)
    {
        const auto columnSplit = splitString(std::string(columns), ",");
        assert(columnSplit.size() == 2);

        return Eigen::Vector2i(data[columnSplit[0]],
            data[columnSplit[1]]);
    }
};

}

namespace nlohmann {
template <>
struct adl_serializer<Eigen::Vector3f> {
    static Eigen::Vector3f from_json(const json& j)
    {
        if (j.is_object()) {
            return Eigen::Vector3f(j.at("x").get<float>(), j.at("y").get<float>(), j.at("z").get<float>());
        } else if (j.is_array()) {
            return Eigen::Vector3f(j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>());
        } else {
            assert(false);
            return Eigen::Vector3f::Zero();
        }
    }
    static void to_json(json& j, const Eigen::Vector3f& v)
    {
        j["x"] = v.x();
        j["y"] = v.y();
        j["z"] = v.z();
    }
};

template <>
struct adl_serializer<Eigen::Vector3i> {
    static Eigen::Vector3i from_json(const json& j)
    {
        if (j.is_object()) {
            return Eigen::Vector3i(j.at("x").get<int>(), j.at("y").get<int>(), j.at("z").get<int>());
        } else if (j.is_array()) {
            return Eigen::Vector3i(j.at(0).get<int>(), j.at(1).get<int>(), j.at(2).get<int>());
        } else {
            assert(false);
            return Eigen::Vector3i::Zero();
        }
    }
    static void to_json(json& j, const Eigen::Vector3i& v)
    {
        j["x"] = v.x();
        j["y"] = v.y();
        j["z"] = v.z();
    }
};

template <>
struct adl_serializer<Eigen::Vector2f> {
    static Eigen::Vector2f from_json(const json& j)
    {
        if (j.is_object()) {
            return Eigen::Vector2f(j.at("x").get<float>(), j.at("y").get<float>());
        } else if (j.is_array()) {
            return Eigen::Vector2f(j.at(0).get<float>(), j.at(1).get<float>());
        } else {
            assert(false);
            return Eigen::Vector2f::Zero();
        }
    }
    static void to_json(json& j, const Eigen::Vector2f& v)
    {
        j["x"] = v.x();
        j["y"] = v.y();
    }
};

template <>
struct adl_serializer<Eigen::Vector2i> {
    static Eigen::Vector2i from_json(const json& j)
    {
        if (j.is_object()) {
            return Eigen::Vector2i(j.at("x").get<int>(), j.at("y").get<int>());
        } else if (j.is_array()) {
            return Eigen::Vector2i(j.at(0).get<int>(), j.at(1).get<int>());
        } else {
            assert(false);
            return Eigen::Vector2i::Zero();
        }
    }
    static void to_json(json& j, const Eigen::Vector2i& v)
    {
        j["x"] = v.x();
        j["y"] = v.y();
    }
};

} // namespace nlohmann
