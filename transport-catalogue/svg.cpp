#include "svg.h"

#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& stroke_line_cap) {
        switch (stroke_line_cap) {
        case StrokeLineCap::BUTT:
            os << "butt"s;
            break;
        case StrokeLineCap::ROUND:
            os << "round"s;
            break;
        case StrokeLineCap::SQUARE:
            os << "square"s;
            break;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& stroke_line_join) {
        switch (stroke_line_join) {
        case StrokeLineJoin::ARCS:
            os << "arcs"s;
            break;
        case StrokeLineJoin::BEVEL:
            os << "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            os << "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            os << "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            os << "round"s;
            break;
        }
        return os;
    }

    std::ostream& ColorPrinter::operator()(std::monostate) const {
        os << "none"s;
        return os;
    }

    std::ostream& ColorPrinter::operator()(std::string str) const {
        os << str;
        return os;
    }

    std::ostream& ColorPrinter::operator()(svg::Rgb rgb) const {
        os << "rgb("s << std::to_string(rgb.red) << ","s
            << std::to_string(rgb.green) << ","s
            << std::to_string(rgb.blue) << ")"s;
        return os;
    }

    std::ostream& ColorPrinter::operator()(svg::Rgba rgba) const {
        os << "rgba("s << std::to_string(rgba.red) << ","s
            << std::to_string(rgba.green) << ","s
            << std::to_string(rgba.blue) << ","s
            << rgba.opacity << ")"s;
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Color color) {
        return std::visit(ColorPrinter{ os }, color);
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\""sv;
        out << " r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        poliline_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        bool first = true;
        out << "<polyline points=\""sv;
        for (const auto& point : poliline_) {
            if (first) {
                out << point.x << ","sv << point.y;
                first = false;
            }
            else {
                out << " "sv << point.x << ","sv << point.y;
            }
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_.clear();
        for (const auto& ch : data) {
            if (ch == '"') {
                data_ += "&quot;";
            }
            else if (ch == 39) {
                data_ += "&apos;";
            }
            else if (ch == '<') {
                data_ += "&lt;";
            }
            else if (ch == '>') {
                data_ += "&gt;";
            }
            else if (ch == '&') {
                data_ += "&amp;";
            }
            else {
                data_.push_back(ch);
            }
        }
        return *this;
    }
/*
    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\" "sv;
        if (!font_family_.empty()) {
            out << "font-family=\""sv << font_family_ << "\" "sv;
        }
        if (!font_weight_.empty()) {
            out << "font-weight=\""sv << font_weight_ << "\""sv;
        }
        RenderAttrs(out);
        out << ">" << data_;
        out << "</text>"sv;
    }
*/
    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"s;
        RenderAttrs(out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">" << data_;
        out << "</text>"sv;
    }
    // ---------- Document ------------------

    // Добавляет в svg-документ объект-наследник svg::Object
    void  Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    // Выводит в ostream svg-представление документа
    void  Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << "\n";
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << "\n";
        for (const auto& object : objects_) {
            out << "  ";
            object->Render(out);
        }
        out << "</svg>";
    }

}  // namespace svg