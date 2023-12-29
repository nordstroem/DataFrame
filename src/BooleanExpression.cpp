#include "BooleanExpression.hpp"

namespace jdf {

ExpressionValue operator""_c(const char* str, std::size_t)
{
    return ExpressionValue(str);
}

BooleanExpression::BooleanExpression(const LeafNode& comparison)
    : type(ExpressionType::Value)
    , comparison(comparison)
{
}

BooleanExpression::BooleanExpression(ExpressionType type, std::unique_ptr<BooleanExpression> left, std::unique_ptr<BooleanExpression> right)
    : type(type)
    , left(std::move(left))
    , right(std::move(right))
{
}

bool BooleanExpression::eval(std::function<bool(const json&, Operator, const json&)> func) const
{
    switch (type) {
    case ExpressionType::Value:
        return func(comparison->col1.value, comparison->op, comparison->col2.value);
    case ExpressionType::And:
        return left->eval(func) && right->eval(func);
    case ExpressionType::Or:
        return left->eval(func) || right->eval(func);
    }
    return false;
}

std::unique_ptr<BooleanExpression> operator&&(std::unique_ptr<BooleanExpression> left, std::unique_ptr<BooleanExpression> right)
{
    return std::make_unique<BooleanExpression>(ExpressionType::And, std::move(left), std::move(right));
}

std::unique_ptr<BooleanExpression> operator||(std::unique_ptr<BooleanExpression> left, std::unique_ptr<BooleanExpression> right)
{
    return std::make_unique<BooleanExpression>(ExpressionType::Or, std::move(left), std::move(right));
}

std::unique_ptr<BooleanExpression> operator==(const ExpressionValue& col1, const ExpressionValue& col2)
{
    return std::make_unique<BooleanExpression>(LeafNode { col1, Operator::Equal, col2 });
}

std::unique_ptr<BooleanExpression> operator!=(const ExpressionValue& col1, const ExpressionValue& col2)
{
    return std::make_unique<BooleanExpression>(LeafNode { col1, Operator::NotEqual, col2 });
}

std::unique_ptr<BooleanExpression> operator<(const ExpressionValue& col1, const ExpressionValue& col2)
{
    return std::make_unique<BooleanExpression>(LeafNode { col1, Operator::Less, col2 });
}

std::unique_ptr<BooleanExpression> operator<=(const ExpressionValue& col1, const ExpressionValue& col2)
{
    return std::make_unique<BooleanExpression>(LeafNode { col1, Operator::LessOrEqual, col2 });
}

std::unique_ptr<BooleanExpression> operator>(const ExpressionValue& col1, const ExpressionValue& col2)
{
    return std::make_unique<BooleanExpression>(LeafNode { col1, Operator::Greater, col2 });
}

std::unique_ptr<BooleanExpression> operator>=(const ExpressionValue& col1, const ExpressionValue& col2)
{
    return std::make_unique<BooleanExpression>(LeafNode { col1, Operator::GreaterOrEqual, col2 });
}

}
