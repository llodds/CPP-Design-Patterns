#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

struct Shape {
    virtual string str() const = 0;
};

struct Circle : Shape {
    float radius;
    
    Circle() {}
    
    explicit Circle(const float radius) : radius{radius} {}
    
    void resize(float factor) { radius *= factor; }
    
    string str() const override {
        ostringstream oss;
        oss << "A circle of radius " << radius;
        return oss.str();
    }
};

struct Square : Shape {
    float side;
    
    Square() {}
    
    explicit Square(const float side) : side{side} {}
    
    string str() const override {
        ostringstream oss;
        oss << "A square of with side " << side;
        return oss.str();
    }
};

// Mixin Inheritance: now we can use resize() inherited from Circle
// decorating via inheritance
template <typename T> struct ColoredShape : T {
    // We are decorating T which must be a Shape by inheritance.
    static_assert(is_base_of<Shape, T>::value,
                  "Template argument must be a Shape");
    
    string color;
    
    // need this
    ColoredShape() {}
    
    // in order to being able to call
    // TransparentShape2<ColoredShape2<square>> s{10, "red", 44}
    template < typename... Args>
    ColoredShape(const string& color, Args... args)
    : T(std::forward<Args>(args)...), color{color} {}
    
    string str() const override {
        ostringstream oss;
        oss << T::str() << " has the color " << color;
        return oss.str();
    }
};

template <typename T> struct TransparentShape : T {
    static_assert(is_base_of<Shape, T>::value,
                  "Template argument must be a Shape");
    
    uint8_t transparency;
    
    TransparentShape(){}
    
    template <typename... Args>
    explicit TransparentShape(const uint8_t transparency, Args... args)
    : T(std::forward<Args>(args)...), transparency{transparency} {}
    
    string str() const override {
        ostringstream oss;
        oss << T::str() << " has "
        << transparency / 255.f * 100.f << "% transparency";
        return oss.str();
    }
};

int main() {
    // Won't work without a default constructors. Here for Circle.
    ColoredShape<Circle> green_circle{"green"};
    green_circle.radius = 5;
    cout << green_circle.str() << endl;
    
    TransparentShape<ColoredShape<Square>> square{51, "blue", 10};
    cout << square.str() << endl;
    
    return 0;
}
