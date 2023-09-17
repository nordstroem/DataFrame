#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace df {

class ColumnName {
public:
    template <typename T>
    ColumnName(T value)
        : name(std::to_string(value))
    {
    }
    ColumnName(const char* value);
    const std::string name;
};

ColumnName operator""_c(const char* str, std::size_t);

enum class Operator {
    Equal,
    NotEqual,
    Less,
    LessOrEqual,
    Greater,
    GreaterOrEqual,
};

struct LeafNode {
    const ColumnName col1;
    const Operator op;
    const ColumnName col2;
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

    bool eval(std::function<bool(const std::string_view col1, Operator op, std::string_view col2)> func) const;
};

std::unique_ptr<BooleanExpression> operator&&(std::unique_ptr<BooleanExpression> left, std::unique_ptr<BooleanExpression> right);
std::unique_ptr<BooleanExpression> operator||(std::unique_ptr<BooleanExpression> left, std::unique_ptr<BooleanExpression> right);
std::unique_ptr<BooleanExpression> operator==(const ColumnName& col1, const ColumnName& col2);
std::unique_ptr<BooleanExpression> operator!=(const ColumnName& col1, const ColumnName& col2);
std::unique_ptr<BooleanExpression> operator<(const ColumnName& col1, const ColumnName& col2);
std::unique_ptr<BooleanExpression> operator<=(const ColumnName& col1, const ColumnName& col2);
std::unique_ptr<BooleanExpression> operator>(const ColumnName& col1, const ColumnName& col2);
std::unique_ptr<BooleanExpression> operator>=(const ColumnName& col1, const ColumnName& col2);

}
