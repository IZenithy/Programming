#ifndef UTIL_H
#define UTIL_H

extern "C"
{
    #include <X11/Xlib.h>
}
#include <ostream>
#include <string>

// Represents a 2D size.
template <typename T>
class Size
{
public: // Public
    T m_width;
    T m_height;

    Size() = default;

    Size(T w, T h)
      : m_width{w}, m_height{h}
    {

    }

    std::string ToString() const;
};


// Outputs a Size<T> as a string to a std::ostream.
template <typename T>
std::ostream& operator << (std::ostream& out, const Size<T>& size);


// Represents a 2D position.
template <typename T>
class Position
{
public: // Public
    T m_x;
    T m_y;

    Position() = default;

    Position(T x, T y)
    : m_x{x}, m_y{y}
    {

    }

    std::string ToString() const;
};


// Represents a 2D vector.
template <typename T>
class Vector2D
{
public: // Public
    T m_x;
    T m_y;

    Vector2D() = default;

    Vector2D(T x, T y)
    : m_x{x}, m_y{y}
    {

    }

    std::string ToString() const;
};

// Outputs a Size<T> as a string to a std::ostream.
template <typename T>
std::ostream& operator << (std::ostream& out, const Position<T>& pos);

// Position operators.
template <typename T>
Vector2D<T> operator - (const Position<T>& a, const Position<T>& b);
template <typename T>
Position<T> operator + (const Position<T>& a, const Vector2D<T> &v);
template <typename T>
Position<T> operator + (const Vector2D<T> &v, const Position<T>& a);
template <typename T>
Position<T> operator - (const Position<T>& a, const Vector2D<T> &v);

// Size operators.
template <typename T>
Vector2D<T> operator - (const Size<T>& a, const Size<T>& b);
template <typename T>
Size<T> operator + (const Size<T>& a, const Vector2D<T> &v);
template <typename T>
Size<T> operator + (const Vector2D<T> &v, const Size<T>& a);
template <typename T>
Size<T> operator - (const Size<T>& a, const Vector2D<T> &v);

// Joins a container of elements into a single string, with elements separated
// by a delimiter. Any element can be used as long as an operator << on ostream
// is defined.
template <typename Container>
std::string Join(const Container& container, const std::string& delimiter);

// Joins a container of elements into a single string, with elements separated
// by a delimiter. The elements are converted to string using a converter
// function.
template <typename Container, typename Converter>
std::string Join(
    const Container& container,
    const ::std::string& delimiter,
    Converter converter);

// Returns a string representation of a built-in type that we already have
// ostream support for.
template <typename T>
std::string ToString(const T& x);

// Returns a string describing an X event for debugging purposes.
std::string ToString(const XEvent& e);

// Returns a string describing an X window configuration value mask.
std::string XConfigureWindowValueMaskToString(unsigned long value_mask);

// Returns the name of an X request code.
std::string XRequestCodeToString(unsigned char request_code);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               IMPLEMENTATION                              *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <algorithm>
#include <vector>
#include <sstream>

template <typename T>
std::string Size<T>::ToString() const
{
    std::ostringstream out;
    out << m_width << 'x' << m_height;
    return out.str();
}

template <typename T>
std::ostream& operator << (std::ostream& out, const Size<T>& size)
{
    return out << size.ToString();
}

template <typename T>
std::string Position<T>::ToString() const
{
    std::ostringstream out;
    out << "(" << m_x << ", " << m_y << ")";
    return out.str();
}

template <typename T>
std::ostream& operator << (std::ostream& out, const Position<T>& size)
{
    return out << size.ToString();
}

template <typename T>
std::string Vector2D<T>::ToString() const
{
    std::ostringstream out;
    out << "(" << m_x << ", " << m_y << ")";
    return out.str();
}

template <typename T>
std::ostream& operator << (std::ostream& out, const Vector2D<T>& size)
{
    return out << size.ToString();
}

template <typename T>
Vector2D<T> operator - (const Position<T>& a, const Position<T>& b)
{
    return Vector2D<T>(a.m_x - b.m_x, a.m_y - b.m_y);
}

template <typename T>
Position<T> operator + (const Position<T>& a, const Vector2D<T> &v)
{
    return Position<T>(a.m_x + v.m_x, a.m_y + v.m_y);
}

template <typename T>
Position<T> operator + (const Vector2D<T> &v, const Position<T>& a)
{
    return Position<T>(a.m_x + v.m_x, a.m_y + v.m_y);
}

template <typename T>
Position<T> operator - (const Position<T>& a, const Vector2D<T> &v)
{
    return Position<T>(a.m_x - v.m_x, a.m_y - v.m_y);
}

template <typename T>
Vector2D<T> operator - (const Size<T>& a, const Size<T>& b)
{
    return Vector2D<T>(a.width - b.m_width, a.m_height - b.m_height);
}

template <typename T>
Size<T> operator + (const Size<T>& a, const Vector2D<T> &v)
{
    return Size<T>(a.m_width + v.m_x, a.m_height + v.m_y);
}

template <typename T>
Size<T> operator + (const Vector2D<T> &v, const Size<T>& a)
{
    return Size<T>(a.m_width + v.m_x, a.m_height + v.m_y);
}

template <typename T>
Size<T> operator - (const Size<T>& a, const Vector2D<T> &v)
{
    return Size<T>(a.m_width - v.m_x, a.m_height - v.m_y);
}

template <typename Container>
std::string Join(const Container& container, const ::std::string& delimiter)
{
    ::std::ostringstream out;
    for (auto i = container.cbegin(); i != container.cend(); ++i)
    {
        if (i != container.cbegin())
        {
            out << delimiter;
        }
        out << *i;
    }
    return out.str();
}

template <typename Container, typename Converter>
std::string Join(const Container& container,const ::std::string& delimiter,Converter converter)
{
    std::vector<::std::string> converted_container(container.size());
    std::transform(container.cbegin(), container.cend(), converted_container.begin(), converter);
    return Join(converted_container, delimiter);
}

template <typename T>
std::string ToString(const T& x)
{
    std::ostringstream out;
    out << x;
    return out.str();
}

#endif
