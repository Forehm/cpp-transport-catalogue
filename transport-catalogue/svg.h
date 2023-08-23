#pragma once
#define _USE_MATH_DEFINES
#include <optional>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <ostream>




namespace svg {

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };


    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };


    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap_);


    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join_);


    class Rgb
    {
    public:
        Rgb() = default;

        Rgb(const uint8_t red, const uint8_t green, const uint8_t blue)
            : red(red), green(green), blue(blue)
        {
        }

        void operator = (Rgb other);

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };
     
    class Rgba : public Rgb
    {
    public:
        Rgba() = default;
        Rgba(const uint8_t red, const uint8_t green, const uint8_t blue, const double opacity)
            : Rgb(red, green, blue), opacity(opacity)
        {
        }

        void operator = (Rgba other);

        double opacity = 1;
    };

    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
    inline const Color NoneColor{ "none" };


    struct ColorPrinter
    {
        std::ostream& out;

        void operator()(std::monostate);
        void operator()(std::string str);
        void operator()(svg::Rgb rgb);
        void operator()(svg::Rgba rgba);
    };



    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = std::move(width);
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = std::move(line_cap);
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const;

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };


    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };


    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const;

        void RenderIndent() const;

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };


    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;

    protected:
        Object() = default;
    };


    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);

        Circle& SetRadius(double radius);


    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };


    class Polyline : public Object, public PathProps<Polyline> {
    public:

        Polyline& AddPoint(Point point);



    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };


    class Text : public Object, public PathProps<Text> {
    public:

        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

    private:

        void RenderObject(const RenderContext& context) const override;

        Point position_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };


    class ObjectContainer {
    public:
        template <typename Obj>
        void Add(Obj obj) {
            objects_.push_back(std::make_unique<Obj>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    protected:
        std::vector<std::unique_ptr<Object>> objects_;

        ~ObjectContainer() = default;
    };


    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;

        virtual ~Drawable() = default;
    };


    class Document : public ObjectContainer {
    public:
        // Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
        // Пример использования:
        // Document doc;
        // doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));

        Document() = default;

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

    };

    template<typename Owner>
    inline void PathProps<Owner>::RenderAttrs(std::ostream& out) const
    {
        using namespace std::literals;

        if (fill_color_)
        {
            out << " fill=\""sv;
            std::visit(ColorPrinter{ out }, *fill_color_);
            out << "\""sv;
        }
        if (stroke_color_)
        {
            out << " stroke=\""sv;
            std::visit(ColorPrinter{ out }, *stroke_color_);
            out << "\""sv;
        }
        if (stroke_width_)
        {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (line_cap_)
        {
            out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
        }
        if (line_join_)
        {
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    }

    

}  // namespace svg




std::ostream& operator << (std::ostream& out, svg::Color color);



namespace shapes
{
    class Triangle : public svg::Drawable {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3);

        // Реализует метод Draw интерфейса svg::Drawable
        void Draw(svg::ObjectContainer& container) const override;

    private:
        svg::Point p1_, p2_, p3_;
    };


    class Star : public svg::Drawable {
    public:
        Star(svg::Point center, double outer_rad, double inner_rad, int num_rays);

        void Draw(svg::ObjectContainer& container) const override;

    private:
        svg::Polyline polyline_;

        svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays);
    };


    class Snowman : public svg::Drawable {
    public:
        Snowman(svg::Point head_center, double head_radius);

        void Draw(svg::ObjectContainer& container) const override;

    private:
        svg::Point head_center_;
        double head_radius_;
    };
} // namespace shapes 