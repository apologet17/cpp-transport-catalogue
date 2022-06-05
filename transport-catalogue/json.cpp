#include "json.h"

using namespace std;

namespace json {

namespace {

bool CheckSpecialWord(istream& input, const std::string& word) {
    std::string str;
    char ch;
    for (size_t i = 0; i < word.size(); ++i) {
        ch = input.get();
        if (input.good()) {
            str.push_back(ch);
        }
        else {
            break;
        }
    }
    if (str.size() == word.size() && str == word) {
        return true;
    }
    else {
        return false;
    }
}

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    char c;

    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if ( c != ']') {
        throw ParsingError("");
    }
    else {
        return Node(move(result));
    }

}

Node LoadInt(istream& input) {

    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    }
    else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node{ std::stoi(parsed_num) };
            }
            catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node{ std::stod(parsed_num) };
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(istream& input) {

    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        }
        else if (ch == 92) {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
            case 'n':
                s.push_back('\n');
                break;
            case 't':
                s.push_back('\t');
                break;
            case 'r':
                s.push_back('\r');
                break;
            case '"':
                s.push_back('"');
                break;
            case '\\':
                s.push_back(92);
                break;
            default:
                // Встретили неизвестную escape-последовательность
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        }
        else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        }
        else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }
    return Node{ s };
}

Node LoadDict(istream& input) {
    Dict result;
    char c;
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    if (c != '}') {
        throw ParsingError("");
    }
    else {
        return Node(move(result));
    }
    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;

    while (input) {
        c = input.get();
        if ((c != ' ') && (c != '\t') && (c != '\n') && (c != '\r')) {

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'f') {
                input.putback(c);
                if (CheckSpecialWord(input, "false"s)) {
                    return Node{false};
                }
                else {
                    throw ParsingError("");
                }
            }
            else if (c == 't') {
                input.putback(c);
                if (CheckSpecialWord(input, "true"s)) {
                    return Node{ true };
                }
                else {
                    throw ParsingError("");
                }
            }
            else if (c == 'n') {
                input.putback(c);
                if (CheckSpecialWord(input, "null"s)) {
                    return Node{ nullptr };
                }
                else {
                    throw ParsingError("");
                }
            }
            else {
                input.putback(c);
                return LoadInt(input);
            }
        }
    }
    return Node();
}

}  // namespace

int Node::GetTypeIndex() const {
    return value_.index();
} 

bool operator == (const Node& lhs, const Node& rhs) {
    if ((lhs.GetTypeIndex() == rhs.GetTypeIndex()) && (lhs.GetValue() == rhs.GetValue())) {
        return true;
    }
    return false;
}

bool operator != (const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

bool Node::IsInt() const {
    if (value_.index() == 1) {
        return true;
    }
    return false;
}

bool  Node::IsDouble() const {
    if ((value_.index() == 2) || (value_.index() == 1)) {
        return true;
    }
    return false;
}

bool  Node::IsPureDouble() const {
    if (value_.index() == 2) {
        return true;
    }
    return false;
}

bool  Node::IsBool() const {
    if (value_.index() == 4) {
        return true;
    }
    return false;
}

bool  Node::IsString() const {
    if (value_.index() == 3) {
        return true;
    }
    return false;
}

bool  Node::IsNull() const {
    if (value_.index() == 0) {
        return true;
    }
    return false;
}

bool  Node::IsArray() const {
    if (value_.index() == 5) {
        return true;
    }
    return false;
}

bool  Node::IsMap() const {
    if (value_.index() == 6) {
        return true;
    }
    return false;
}

int Node::AsInt() const {
    if (IsInt()) {
        return std::get<int>(value_);
    }
    else {
        throw std::logic_error("");
    }
}

bool Node::AsBool() const {
    if (IsBool()) {
        return std::get<bool>(value_);
    }
    else {
        throw std::logic_error("");
    }
}

double Node::AsDouble() const {
    if (IsDouble()) {
        if (value_.index() == 1) {
            return static_cast<double>(std::get<int>(value_));
        }
        return std::get<double>(value_);
    }
    else {
        throw std::logic_error("");
    }
}

const string& Node::AsString() const {
    if (IsString()) {
        return std::get<std::string>(value_);
    }
    else {
        throw std::logic_error("");
    }
}

const Array& Node::AsArray() const {
    if (IsArray()) {
        return std::get<Array>(value_);
    }
    else {
        throw std::logic_error("");
    }
}

const Dict& Node::AsMap() const {
    if (IsMap()) {
        return std::get<Dict>(value_);
    }
    else {
        throw std::logic_error("");
    }
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool operator == (const Document& lhs, const Document& rhs) {
    if ((lhs.GetRoot() == rhs.GetRoot()) ) {
        return true;
    }
    return false;
}

bool operator != (const Document& lhs, const Document& rhs)  {
        return !(lhs == rhs);
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

// Шаблон, подходящий для вывода double и int  using Value = std::variant<, , , std::string, , Array, Dict>;
template <typename Value>
void PrintValue(const Value& value, std::ostream& out) {
    out << value;
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(const std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}

// Перегрузка функции PrintValue для вывода значений bool
void PrintValue(const bool value, std::ostream& out) {
    out << std::boolalpha << value;
}

// Перегрузка функции PrintValue для вывода значений string
void PrintValue(const std::string value, std::ostream& out) {
    out << "\"";
    for (const auto& ch : value) {
        if (ch == '\t' || (' ' <= ch && ch != '\\' && ch != '"') || ch<0) {
            out << ch;
        }
        else {
            out << '\\';
            switch (ch) {
            case '"':   out << '"';  break;
            case '\\':  out << '\\'; break;
            case '\r':  out << 'r';  break;
            case '\n':  out << 'n';  break;
            default: break;
            }
        }
    }
    out << "\"";
}

// Перегрузка функции PrintValue для вывода значений Array
void PrintValue(const Array value, std::ostream& out) {
    bool first = true;

    out << "["s<<std::endl;
    for (const auto& val : value) {
        if (first) {
            first = false;
        }
        else {
            out << ","s << std::endl;
        }
        PrintNode(val, out);
    }
    out<<std::endl << "]"s;
}

// Перегрузка функции PrintValue для вывода значений Dict
void PrintValue(const Dict value, std::ostream& out) {
    out << "{"s<<std::endl;
    bool first = true;
    for (const auto& [key, val] : value) {
        if (first) {
            first = false;
        }
        else {
            out << ","s<<std::endl;
        }
        out << "\""s<<key<<"\": "s;
        PrintNode(val, out);
    }
    out<<std::endl << "}"s;
}

void Print(const Document& doc, std::ostream& output) {
   // output << "json"s;
    PrintNode(doc.GetRoot(), output);
}

void PrintNode(const Node& node, std::ostream& out) {
    std::visit(
        [&out](const auto& value) { PrintValue(value, out); },
        node.GetValue());
}

std::ostream& operator<<(std::ostream& out, const Array nodes) {
    PrintValue(nodes, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const Dict nodes) {
    PrintValue(nodes, out);
    return out;
}

}  // namespace json