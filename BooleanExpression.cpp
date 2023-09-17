#include "BooleanExpression.hpp"
#include "doctest.h"

namespace df {

ColumnName::ColumnName(const char* value)
    : name(value)
{
}

ColumnName operator""_c(const char* str, std::size_t)
{
    return ColumnName(str);
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

bool BooleanExpression::eval(std::function<bool(const std::string& col1, Operator op, const std::string& col2)> func) const
{
    switch (type) {
    case ExpressionType::Value:
        return func(comparison->col1.name, comparison->op, comparison->col2.name);
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

std::unique_ptr<BooleanExpression> operator==(const ColumnName& col1, const ColumnName& col2)
{
    return std::make_unique<BooleanExpression>(LeafNode { col1, Operator::Equal, col2 });
}

std::unique_ptr<BooleanExpression> operator!=(const ColumnName& col1, const ColumnName& col2)
{
    return std::make_unique<BooleanExpression>(LeafNode { col1, Operator::NotEqual, col2 });
}

std::unique_ptr<BooleanExpression> operator<(const ColumnName& col1, const ColumnName& col2)
{
    return std::make_unique<BooleanExpression>(LeafNode { col1, Operator::Less, col2 });
}

std::unique_ptr<BooleanExpression> operator<=(const ColumnName& col1, const ColumnName& col2)
{
    return std::make_unique<BooleanExpression>(LeafNode { col1, Operator::LessOrEqual, col2 });
}

std::unique_ptr<BooleanExpression> operator>(const ColumnName& col1, const ColumnName& col2)
{
    return std::make_unique<BooleanExpression>(LeafNode { col1, Operator::Greater, col2 });
}

std::unique_ptr<BooleanExpression> operator>=(const ColumnName& col1, const ColumnName& col2)
{
    return std::make_unique<BooleanExpression>(LeafNode { col1, Operator::GreaterOrEqual, col2 });
}

TEST_CASE("Boolean expressions")
{
    auto const standardVisitor = [](auto const& col1, Operator op, auto const& col2) {
        switch (op) {
        case Operator::Equal:
            return col1 == col2;
        case Operator::NotEqual:
            return col1 != col2;
        }
        return false;
    };
    SUBCASE("equality")
    {
        const auto expression = "x"_c == "x";
        CHECK(expression->eval(standardVisitor));
    }
    SUBCASE("inequality")
    {
        const auto expression = "x"_c != "x";
        CHECK_FALSE(expression->eval(standardVisitor));
    }
    SUBCASE("and operator - true")
    {
        const auto expression = "x"_c == "x" && "y"_c == "y";
        CHECK(expression->eval(standardVisitor));
    }
    SUBCASE("and operator - false")
    {
        const auto expression = "x"_c == "x" && "y"_c == "z";
        CHECK_FALSE(expression->eval(standardVisitor));
    }
    SUBCASE("or operator - true")
    {
        const auto expression = "x"_c == "z" || "y"_c == "y";
        CHECK(expression->eval(standardVisitor));
    }
    SUBCASE("or operator - false")
    {
        const auto expression = "x"_c == "z" || "y"_c == "z";
        CHECK_FALSE(expression->eval(standardVisitor));
    }
    SUBCASE("two or operator")
    {
        const auto expression = "x"_c == "z" || ("y"_c == "z" || "z"_c == "z");
        CHECK(expression->eval(standardVisitor));
    }
    SUBCASE("and + or operator")
    {
        const auto expression = "x"_c == "x" && ("y"_c == "z" || "z"_c != "z");
        CHECK_FALSE(expression->eval(standardVisitor));
    }
}

}
