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

struct TreeNode
{
    TreeNode(const ColumnName &col1, Operator op, const ColumnName &col2)
        : col1(col1), op(op), col2(col2)
    {
    }

    const ColumnName col1;
    const Operator op;
    const ColumnName col2;
};

enum class TreeType
{
    Value,
    And,
    Or
};

struct Tree
{
    Tree(TreeNode comparison)
        : type(TreeType::Value), comparison(comparison)
    {
    }
    Tree(TreeType type, std::unique_ptr<Tree> left, std::unique_ptr<Tree> right)
        : type(type), left(std::move(left)), right(std::move(right))
    {
    }
    const TreeType type;
    const std::optional<TreeNode> comparison;
    const std::unique_ptr<Tree> left;
    const std::unique_ptr<Tree> right;

    bool eval(std::function<bool(const std::string_view col1, Operator op, std::string_view col2)> func) const
    {
        switch (type)
        {
        case TreeType::Value:
            return func(comparison->col1.name, comparison->op, comparison->col2.name);
        case TreeType::And:
            return left->eval(func) && right->eval(func);
        case TreeType::Or:
            return left->eval(func) || right->eval(func);
        }
        return false;
    }
};

inline std::unique_ptr<Tree> operator&&(std::unique_ptr<Tree> left, std::unique_ptr<Tree> right)
{
    return std::make_unique<Tree>(TreeType::And, std::move(left), std::move(right));
}

inline std::unique_ptr<Tree> operator||(std::unique_ptr<Tree> left, std::unique_ptr<Tree> right)
{
    return std::make_unique<Tree>(TreeType::Or, std::move(left), std::move(right));
}

inline std::unique_ptr<Tree> operator==(const ColumnName &col1, const ColumnName &col2)
{
    return std::make_unique<Tree>(TreeNode(col1, Operator::Equal, col2));
}

inline std::unique_ptr<Tree> operator!=(const ColumnName &col1, const ColumnName &col2)
{
    return std::make_unique<Tree>(TreeNode(col1, Operator::NotEqual, col2));
}

inline std::unique_ptr<Tree> operator<(const ColumnName &col1, const ColumnName &col2)
{
    return std::make_unique<Tree>(TreeNode(col1, Operator::Less, col2));
}

inline std::unique_ptr<Tree> operator<=(const ColumnName &col1, const ColumnName &col2)
{
    return std::make_unique<Tree>(TreeNode(col1, Operator::LessOrEqual, col2));
}

inline std::unique_ptr<Tree> operator>(const ColumnName &col1, const ColumnName &col2)
{
    return std::make_unique<Tree>(TreeNode(col1, Operator::Greater, col2));
}

inline std::unique_ptr<Tree> operator>=(const ColumnName &col1, const ColumnName &col2)
{
    return std::make_unique<Tree>(TreeNode(col1, Operator::GreaterOrEqual, col2));
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
