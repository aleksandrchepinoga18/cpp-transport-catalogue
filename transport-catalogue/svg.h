#pragma once

#include <any>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>
#include<map>

namespace svg {

/* ---------------- TYPES ---------------- */

struct Point {
    Point() = default;
    Point(double x, double y) : x(x), y(y) {}

    double x{0.};
    double y{0.};
};

/*
 *Структура поддержки для вывода SVG
 */
struct RenderContext {
public:  // Constructors
    explicit RenderContext(std::ostream& out) : out(out) {}

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out), indent_step(indent_step), indent(indent) {}

public:  // Methods
    [[nodiscard]] RenderContext Indented() const;
    void RenderIndent() const;

public:  // Fields
    std::ostream& out;
    int indent_step{0};
    int indent{0};
};

/* ---------------- COLOR TYPES ---------------- */

struct Rgb {
    uint8_t red{0};
    uint8_t green{0};
    uint8_t blue{0};

    Rgb() = default;
    Rgb(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {}
};

struct Rgba : public Rgb {
    double opacity{1.};

    Rgba() = default;
    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) : Rgb(red, green, blue), opacity(opacity) {}
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

inline const Color NoneColor{"none"};

/*
 * Используется для вывода цвета в std::ostream
 */
struct ColorPrinter {
    std::ostream& os;

    void operator()(std::monostate) const;
    void operator()(const std::string& color) const;
    void operator()(Rgb color) const;
    void operator()(Rgba color) const;
};

std::ostream& operator<<(std::ostream& os, const Color& color);

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

std::ostream& operator<<(std::ostream& os, const StrokeLineCap& value);

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& value);

/* ---------------- PRIMITIVES ---------------- */

/*
* Абстрактный родительский класс Object используется для универсального подхода к хранению SVG-документов
 * Реализовано с помощью шаблона программирования "Шаблон".
 */
class Object {
public:  // Methods
    void Render(const RenderContext& context) const;
    void Render(std::ostream& os) const;

public:  // Destructor
    virtual ~Object() = default;

private:  // Methods
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
* Класс для хранения дополнительных атрибутов для объектов.
 * Отдельный класс, потому что не все объекты имеют эти атрибуты (<изображение>).
 * Подход: Любопытно повторяющийся шаблон шаблона
 */
template <class ObjectType>
class PathProps {
public:  // Methods
    ObjectType& SetFillColor(Color color) {
        color_ = color;
        return AsObjectType();
    }
    ObjectType& SetStrokeColor(Color stroke_color) {
        stroke_color_ = stroke_color;
        return AsObjectType();
    }
    ObjectType& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsObjectType();
    }
    ObjectType& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = line_cap;
        return AsObjectType();
    }
    ObjectType& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = line_join;
        return AsObjectType();
    }

protected:  // Destructor
    ~PathProps() = default;

protected:  // Methods
    void RenderAttrs(std::ostream& os) const {
        using namespace std::literals;

        //По умолчанию мы предполагаем, что объявление примитивов имеет пробел в конце

        PrintProperty(os, "fill"sv, color_);
        PrintProperty(os, "stroke"sv, stroke_color_);
        PrintProperty(os, "stroke-width"sv, stroke_width_);
        PrintProperty(os, "stroke-linecap"sv, line_cap_);
        PrintProperty(os, "stroke-linejoin"sv, line_join_);
    }

    void RenderAttrs(const RenderContext& context) const {
        RenderContext(context.out);
    }

protected:  // Fields
    std::optional<Color> color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;

private:  // Methods
    ObjectType& AsObjectType() {
        return static_cast<ObjectType&>(*this);
    }

    template <class PropertyType>
    void PrintProperty(std::ostream& os, std::string_view tag_name,
                       const std::optional<PropertyType>& tag_value) const {
        if (tag_value)
            os << " " << tag_name << "=\"" << *tag_value << "\"";
    }
};

/*
 * Circle represent <circle> tag
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:  // Methods
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:  // Methods
    void RenderObject(const RenderContext& context) const override;

private:  // Fields
    Point center_;
    double radius_ = 1.0;
};

/*
 * Polyline represent <polyline> tag
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:  // Methods
    Polyline& AddPoint(Point point);

private:  // Methods
    void RenderObject(const RenderContext& context) const override;

private:  // Fields
    std::vector<Point> vertexes_;
};

/*
 * Text represent <text> tag
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:  // Types
    struct EscapeCharacter {
        char character;
        std::string replacement;
    };

public:  // Methods
    Text& SetPosition(Point position = Point());

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset = Point());

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:  // Methods
    void RenderObject(const RenderContext& context) const override;

    //! Заменяет все escape-символы SVG для входной строки
    [[nodiscard]] std::string PreprocessTest(const std::string& input_text) const;

private:  // Constants
    //! Последовательность символов должна начинаться с «&», иначе
    //! все следующие символы (&quot, ...) будут с заменой '&'
    inline static const std::vector<EscapeCharacter> kEscapeCharacters{
        {'&', "&amp;"}, {'"', "&quot;"}, {'\'', "&apos;"}, {'<', "&lt;"}, {'>', "&gt;"}};

private:  // Fields
    Point position_;
    Point offset_;

    uint32_t font_size_{1};
    std::string font_family_;
    std::string font_weight_;

    std::string text_;
};

/* ---------------- OBJECT CONTAINERS ---------------- */

class ObjectContainer {
public:  // Destructor
    virtual ~ObjectContainer() = default;

public:  // Methods
    virtual void AddPtr(std::unique_ptr<Object>&& /* objects */) = 0;

    template <typename ObjectType>
    void Add(ObjectType obj) {
        AddPtr(std::make_unique<ObjectType>(std::move(obj)));
    }

protected:  // Fields
    std::vector<std::unique_ptr<Object>> storage_;
};

class Document final : public ObjectContainer {
public:  // Methods
    void AddPtr(std::unique_ptr<Object>&& object) override;
    void Render(std::ostream& out) const;
};

/* ---------------- FIGURES ---------------- */

class Drawable {
public:  // Destructor
    virtual ~Drawable() = default;

public:  // Methods
    virtual void Draw(svg::ObjectContainer& /* container */) const = 0;
};

}  // namespace svg

namespace shapes {

//============================================= shapes==========   
  struct Size {
    int width = 0; 
    int height = 0; 
}; 

struct Point { 
    int x = 0;
    int y = 0;
};

// Изображение. Пиксели это символы.
// Первый индекс (по std::vector) - строки изображения, координата y
// Второй индекс (по std::string) - столбцы изображения, координата x
// Предполагается, что длина всех строк одинакова
using Image = std::vector<std::string>;

inline Size GetImageSize(const Image& image) {
    const int width = image.empty() ? 0 : static_cast<int>(image[0].size());
    const int height = static_cast<int>(image.size());
    return {width, height};
}

// Проверяет, содержится ли заданная точка в эллипсе заданного размера
// Считается, что эллипс вписан в прямоугольник с вершинами в точках (0, 0) и
// (size.width, size.height)
inline bool IsPointInEllipse(Point p, Size size) {
    // Нормируем координаты точки в диапазон (-1, 1)
    // Сдвиг на 0.5 нужен, чтобы считать расстояние до центра пикселя, так
    // получается более красивая форма
    double x = (p.x + 0.5) / (size.width / 2.0) - 1;
    double y = (p.y + 0.5) / (size.height / 2.0) - 1;
    // Проверяем, лежит ли точка в единичном круге
    return x * x + y * y <= 1;
} 
//==========================================================================
class Texture {
public:
    // Constructor
    Texture(Size size) : size_(size) {}

  char GetPixelColor(Point position) const {
    const std::string colors = ".@Xo*";
    const int color_index = (position.x + position.y) % colors.size();
    return colors[color_index];
}
private:
    Size size_;
};

//============================================================================
// Поддерживаемые виды фигур: прямоугольник и эллипс
enum class ShapeType { RECTANGLE, ELLIPSE };

class Shape {
public:  // Constructor
    explicit Shape(ShapeType type) : type_(type) {}

public:  // Methods
    void SetPosition(Point pos) {
        position_ = pos;
    }
    void SetSize(Size size) {
        size_ = size;
    }
    void SetTexture(std::shared_ptr<Texture> texture) {
        texture_ = std::move(texture);
    }
    /*
    * Метод рисует фигуру на указанном изображении
     * В зависимости от типа фигуры следует рисовать либо эллипс, либо прямоугольник
     * Пиксели формы, выходящие за пределы текстуры, а также в корпус
     * если текстура не указана, должна отображаться с точкой '.'
     * Части фигуры за пределами объекта изображения должны быть отброшены.
     */
    void Draw(Image& image) const;

private:  // Fields
    ShapeType type_;
    Point position_;
    Size size_;
    std::shared_ptr<Texture> texture_;
};

// ======================   
class Canvas {
public:  // Using
    using ShapeId = size_t;

public:  // Constructor
    explicit Canvas(Size size) : size_(size) {}

public:  // Methods
    void SetSize(Size size) {
        size_ = size;
    }

    ShapeId AddShape(ShapeType shape_type, Point position, Size size, std::shared_ptr<Texture> texture) {
        auto shape = std::make_unique<Shape>(shape_type);
        shape->SetPosition(position);
        shape->SetSize(size);
        shape->SetTexture(std::move(texture));
        return InsertShape(std::move(shape));
    }

    ShapeId DuplicateShape(ShapeId source_id, Point target_position) {
        auto shape = std::make_unique<Shape>(*GetShapeNodeById(source_id)->second);
        shape->SetPosition(target_position);
        return InsertShape(std::move(shape));
    }

    void RemoveShape(ShapeId id) {
        shapes_.erase(GetShapeNodeById(id));
    }

    void MoveShape(ShapeId id, Point position) {
        GetShapeNodeById(id)->second->SetPosition(position);
    }

    void ResizeShape(ShapeId id, Size size) {
        GetShapeNodeById(id)->second->SetSize(size);
    }

    int GetShapesCount() const {
        return static_cast<int>(shapes_.size());
    }

    void Print(std::ostream& output) const {
        using namespace std::literals;

        Image image(size_.height, std::string(size_.width, ' '));

        for (const auto& [id, shape] : shapes_) {
            shape->Draw(image);
        }

        output << '#' << std::string(size_.width, '#') << "#\n"sv;
        for (const auto& line : image) {
            output << '#' << line << "#\n"sv;
        }
        output << '#' << std::string(size_.width, '#') << "#\n"sv;
    }

private:
    using Shapes = std::map<ShapeId, std::unique_ptr<Shape>>;

    Shapes::iterator GetShapeNodeById(ShapeId id) {
        auto it = shapes_.find(id);
        if (it == shapes_.end()) {
            throw std::out_of_range("No shape with given ID");
        }
        return it;
    }
    ShapeId InsertShape(std::unique_ptr<Shape> shape) {
        shapes_[current_id_] = std::move(shape);
        return current_id_++;
    }

    Size size_ = {};
    ShapeId current_id_ = 0;
    Shapes shapes_;
};  
   //============================================= shapes=========================================
 //=======================================================   
class Star : public svg::Drawable {
public:  // Constructor
    Star(svg::Point center, double outer_radius, double inner_radius, int rays_count)
        : center_(center), outer_radius_(outer_radius), inner_radius_(inner_radius), rays_count_(rays_count) {}

public:  // Methods
    void Draw(svg::ObjectContainer& container) const override;

private:  // Fields
    svg::Point center_;
    double outer_radius_{0.};
    double inner_radius_{0.};
    int rays_count_{0};

    svg::Color fill_color_{"red"};
    svg::Color stroke_color_{"black"};
};

class Snowman : public svg::Drawable {
public:  // Constructors
    Snowman(svg::Point head_center, double head_radius) : head_center_(head_center), head_radius_(head_radius) {}

public:  // Methods
    void Draw(svg::ObjectContainer& container) const override;

private:  // Fields
    svg::Point head_center_;
    double head_radius_;

    svg::Color fill_color_{"rgb(240,240,240)"};
    svg::Color stroke_color_{"black"};
};

class Triangle : public svg::Drawable {
public:  // Constructor
    Triangle(svg::Point first, svg::Point second, svg::Point third) : first_(first), second_(second), third_(third) {}

public:  // Methods
    void Draw(svg::ObjectContainer& container) const override;

private:  // Fields
    svg::Point first_;
    svg::Point second_;
    svg::Point third_;
};

}  // namespace shapes 
