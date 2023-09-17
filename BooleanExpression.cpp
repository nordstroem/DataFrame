#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

class ColumnName
{
public:
    template <typename T>
    ColumnName(T value)
        : name(std::to_string(value))
    {
    }
    ColumnName(const char *value)
        : name(value)
    {
    }
    const std::string name;
};

ColumnName operator""_c(const char *str, std::size_t)
{
    return ColumnName(str);
}

enum class Operator
{
    Equal,
    NotEqual,
    Less,
    LessOrEqual,
    Greater,
    GreaterOrEqual,
};

struct LeafNode
{
    LeafNode(const ColumnName &col1, Operator op, const ColumnName &col2)
        : col1(col1), op(op), col2(col2)
    {
    }

    const ColumnName col1;
    const Operator op;
    const ColumnName col2;
};

enum class ExpressionType
{
    Value,
    And,
    Or
};

struct BooleanExpression
{
    BooleanExpression(LeafNode comparison)
        : type(ExpressionType::Value), comparison(comparison)
    {
    }
    BooleanExpression(ExpressionType type, std::unique_ptr<BooleanExpression> left, std::unique_ptr<BooleanExpression> right)
        : type(type), left(std::move(left)), right(std::move(right))
    {
    }
    const ExpressionType type;
    const std::optional<LeafNode> comparison;
    const std::unique_ptr<BooleanExpression> left;
    const std::unique_ptr<BooleanExpression> right;

    bool eval(std::function<bool(const std::string_view col1, Operator op, std::string_view col2)> func) const
    {
        switch (type)
        {
        case ExpressionType::Value:
            return func(comparison->col1.name, comparison->op, comparison->col2.name);
        case ExpressionType::And:
            return left->eval(func) && right->eval(func);
        case ExpressionType::Or:
            return left->eval(func) || right->eval(func);
        }
        return false;
    }
};

inline std::unique_ptr<BooleanExpression> operator&&(std::unique_ptr<BooleanExpression> left, std::unique_ptr<BooleanExpression> right)
{
    return std::make_unique<BooleanExpression>(ExpressionType::And, std::move(left), std::move(right));
}

inline std::unique_ptr<BooleanExpression> operator||(std::unique_ptr<BooleanExpression> left, std::unique_ptr<BooleanExpression> right)
{
    return std::make_unique<BooleanExpression>(ExpressionType::Or, std::move(left), std::move(right));
}

inline std::unique_ptr<BooleanExpression> operator==(const ColumnName &col1, const ColumnName &col2)
{
    return std::make_unique<BooleanExpression>(LeafNode(col1, Operator::Equal, col2));
}

inline std::unique_ptr<BooleanExpression> operator!=(const ColumnName &col1, const ColumnName &col2)
{
    return std::make_unique<BooleanExpression>(LeafNode(col1, Operator::NotEqual, col2));
}

inline std::unique_ptr<BooleanExpression> operator<(const ColumnName &col1, const ColumnName &col2)
{
    return std::make_unique<BooleanExpression>(LeafNode(col1, Operator::Less, col2));
}

inline std::unique_ptr<BooleanExpression> operator<=(const ColumnName &col1, const ColumnName &col2)
{
    return std::make_unique<BooleanExpression>(LeafNode(col1, Operator::LessOrEqual, col2));
}

inline std::unique_ptr<BooleanExpression> operator>(const ColumnName &col1, const ColumnName &col2)
{
    return std::make_unique<BooleanExpression>(LeafNode(col1, Operator::Greater, col2));
}

inline std::unique_ptr<BooleanExpression> operator>=(const ColumnName &col1, const ColumnName &col2)
{
    return std::make_unique<BooleanExpression>(LeafNode(col1, Operator::GreaterOrEqual, col2));
}

int main()
{
    auto const visitor = [](auto const &col1, Operator op, auto const &col2)
    {
        switch (op)
        {
        case Operator::Equal:
            return col1 == col2;
        case Operator::NotEqual:
            return col1 != col2;
        }
        return false;
    };

    const auto x = "3"_c == 3 || "a"_c != "a";

    std::cout << x->eval(visitor) << "\n";
}
