#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <optional>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

     Node() = default;

     template<typename T>
     Node(T value)
         : value_(std::move(value)) {
     }

    const Value& GetValue() const { return value_; }
    int GetTypeIndex() const;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;


private:
    Value value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);
void PrintNode(const Node& node, std::ostream& out);

bool operator == (const Node& lhs, const Node& rhs);
bool operator != (const Node& lhs, const Node& rhs);

std::ostream& operator<<(std::ostream& out, const Array nodes);
std::ostream& operator<<(std::ostream& out, const Dict nodes);

bool operator == (const Document& lhs, const Document& rhs) ;
bool operator != (const Document& lhs, const Document& rhs) ;
}  // namespace json