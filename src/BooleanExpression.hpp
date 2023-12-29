#pragma once
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>

namespace jdf {
using json = nlohmann::json;

class ExpressionValue {
public:
    template <typename T>
    ExpressionValue(T value)
        : value(value)
    {
    }
    const json value;
};

ExpressionValue operator""_c(const char* str, std::size_t);

enum class Operator {
    Equal,
    NotEqual,
    Less,
    LessOrEqual,
    Greater,
    GreaterOrEqual,
};

struct LeafNode {
    const ExpressionValue col1;
    const Operator op;
    const ExpressionValue col2;
};

enum class ExpressionType {
    Value,
    And,
    Or
};

struct BooleanExpression {
    BooleanExpression(const LeafNode& comparison);
    BooleanExpression(ExpressionType type, std::unique_ptr<BooleanExpression> left, std::unique_ptr<BooleanExpression> right);

    const ExpressionType type;
    const std::optional<LeafNode> comparison;
    const std::unique_ptr<BooleanExpression> left;
    const std::unique_ptr<BooleanExpression> right;

    bool eval(std::function<bool(const json&, Operator, const json&)> func) const;
};

std::unique_ptr<BooleanExpression> operator&&(std::unique_ptr<BooleanExpression> left, std::unique_ptr<BooleanExpression> right);
std::unique_ptr<BooleanExpression> operator||(std::unique_ptr<BooleanExpression> left, std::unique_ptr<BooleanExpression> right);
std::unique_ptr<BooleanExpression> operator==(const ExpressionValue& col1, const ExpressionValue& col2);
std::unique_ptr<BooleanExpression> operator!=(const ExpressionValue& col1, const ExpressionValue& col2);
std::unique_ptr<BooleanExpression> operator<(const ExpressionValue& col1, const ExpressionValue& col2);
std::unique_ptr<BooleanExpression> operator<=(const ExpressionValue& col1, const ExpressionValue& col2);
std::unique_ptr<BooleanExpression> operator>(const ExpressionValue& col1, const ExpressionValue& col2);
std::unique_ptr<BooleanExpression> operator>=(const ExpressionValue& col1, const ExpressionValue& col2);

}
