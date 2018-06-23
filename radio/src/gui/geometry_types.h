/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef GEOMETRY_TYPES_H
#define GEOMETRY_TYPES_H

#include "otx_math.h"

#include <cstdint>
#include <cmath>
#include <type_traits>

#define GEOTYPE_TEMPL                   template<typename U = T>
#define GEOTYPE_TEMPL2                  template<typename U1 = T, typename U2 = T>
#define GEOTYPE_TEMPL_IFNUM             template<typename U = T, typename _EIF = typename std::enable_if<std::is_arithmetic<U>::value, bool>::type>  // "typename _EIF = " makes MSVC15 not crash
#define GEOTYPE_TEMPL_INL               GEOTYPE_TEMPL inline
#define GEOTYPE_TEMPL2_INL              GEOTYPE_TEMPL2 inline
#define GEOTYPE_TEMPL_IFNUM_INL         GEOTYPE_TEMPL_IFNUM inline
#define GEOTYPE_TEMPL_CST_INL           GEOTYPE_TEMPL constexpr inline
#define GEOTYPE_TEMPL_IFNUM_CST_INL     GEOTYPE_TEMPL_IFNUM constexpr inline
#define GEOTYPE_TEMPL_CST_FR_INL        GEOTYPE_TEMPL constexpr friend inline
#define GEOTYPE_TEMPL_IFNUM_CST_FR_INL  GEOTYPE_TEMPL_IFNUM constexpr friend inline

/*!
  \brief Template class which defines a simple 2-dimentional vector (or vertex) with origin at {0, 0}.
         NOTE: Some functions are limited to \e float precision, may lose accuracy if used with \e T=double.
*/
template<typename T = int>
class Vector2T
{
  public:
    static_assert(std::is_arithmetic<T>::value, "Type must be numeric.");
    typedef T value_type;
    typedef Vector2T<T> this_t;

    // constructors

    Vector2T()         : x(T(0)), y(T(0)) {}
    Vector2T(T x, T y) : x(x), y(y) {}
    //! Create from any other Vector2T type
    GEOTYPE_TEMPL explicit Vector2T(const Vector2T<U> & v) : Vector2T(T(v.x), T(v.y)) {}
    //! Move constructor
    GEOTYPE_TEMPL Vector2T(Vector2T<U> && v) noexcept { x = v.x; y = v.y; }

    // accessors

    T x;
    T y;

    inline T getX() const { return x; }
    inline T getY() const { return y; }

    // setters

    inline this_t & setX(T x)     { this->x = x; return *this; }
    inline this_t & setY(T y)     { this->y = y; return *this; }
    inline this_t & set(T x, T y) { return setX(x).setY(y); }
    //! set from any other Vector2T type
    GEOTYPE_TEMPL_INL this_t & set(const Vector2T<U> & v) { return  this->set(T(v.x), T(v.y)); }

    //! Swap x and y values and return reference to self
    inline this_t & swap()                { T tY = y; y = x; x = tY; return *this; }
    //! Rotate 90 degrees CW or CCW based on value of \a cw and return reference to self
    inline this_t & rot90(bool cw = true) { if (cw) swap().y *= -1; else swap().x *= -1; return *this; }
    //! return normalized instance of self
    inline this_t & normalize() { return (*this = normalized()); }
    //! Shorten to \a length and return reference to self
    GEOTYPE_TEMPL this_t & truncate(U length) { return (*this = truncated(length)); }
    //! Rotate \a deg in any direction and return reference to self
    GEOTYPE_TEMPL this_t & rotate(U deg) { return (*this = rotated(deg)); }

    // calculations

    //! returns normalized length (always returns float so we don't lose accuracy in dependent calculations)
    inline float len()    const { return sqrtf((float)lenSquared()); }
    //! returns "quick" normalized length
    inline T lenSquared() const { return T(x * x + y * y); }
    //! returns new vector orthagonal this vector
    inline this_t ortho() const { return this_t(y, -x); }
    //! returns new vector with \e(abs(x), abs(y)) of this vector
    inline this_t abs()   const { return this_t(T(fabsf(x)), T(fabsf(y))); }

    //! Returns new vector with normalized version of this vector
    inline this_t normalized() const
    {
      if (!lenSquared())
        return this_t();
      return (*this * (1.0f / len()));
    }

    //! Returns new vector of this vector shortened to \a length
    GEOTYPE_TEMPL this_t truncated(U length) const
    {
      float angle = 0;
      if (x || y)
        angle = MathUtil::atan2f(y, x);
      return this_t(T(length * MathUtil::cosf(angle)), T(length * MathUtil::sinf(angle)));
    }

    //! Returns a new vector with \a deg rotation applied to this vector
    GEOTYPE_TEMPL this_t rotated(U deg) const
    {
      const float t = deg * DEG_TO_RADf;
      const float c = MathUtil::cosf(t);
      const float s = MathUtil::sinf(t);
      return this_t(x * c - y * s, x * s + y * c);
    }

    //! returns distance between this vector and another in \e value_type units
    GEOTYPE_TEMPL_INL T dist(const Vector2T<U> & v) const
    {
      return (v - *this).len();
    }

    //! returns "quick" distance between this vector and another in \e value_type units (no sqrt() use used)
    GEOTYPE_TEMPL_INL T distSquared(const Vector2T<U> & v) const
    {
      return (v - *this).lenSquared();
    }

    //! returns distance between this vector and another as a new vector
    GEOTYPE_TEMPL_INL this_t distVect(const Vector2T<U> & v) const
		{
      return (v - *this).abs();
    }

    //! Returns dot product between this vector and another in \e value_type units
    GEOTYPE_TEMPL_INL T dot(const Vector2T<U> & v) const
    {
      return x * v.x + y * v.y;
    }

    //! Returns cross product between this vector and another in \e value_type units
    GEOTYPE_TEMPL_INL T cross(const Vector2T<U> & v) const
    {
      return x * v.y - y * v.x;
    }

    //! Returns center point between this vector and another in \e value_type units
    GEOTYPE_TEMPL_INL T center(const Vector2T<U> & v) const
    {
      return (*this + v) / 2;
    }

    //! Returns angle in radians between this vector and another
    GEOTYPE_TEMPL_INL float angleRad(const Vector2T<U> & v) const
    {
      return MathUtil::atan2f(float(v.y - y), float(v.x - x));
    }

    /*! Return the normalized angle in whole degrees between 2 vectors \a v1 and \a v2.
        E=0, S=90, range 0 to 359. Return -1 if angle is invalid (vectors are equal). \sa angleRad() */
    GEOTYPE_TEMPL_INL int16_t angle(const Vector2T<U> & v) const
    {
      int16_t d = -1;
      if (*this != v) {
        d = MathUtil::normalizeCompassAngle(int16_t(angleRad(v) * RAD_TO_DEGf));
      }
      return d;
    }

    template<typename U = T>
    inline typename std::enable_if<std::is_integral<U>::value, bool>::type
    isNull() const { return (x == 0 && y == 0); }

    template<typename U = T>
    inline typename std::enable_if<!std::is_integral<U>::value && std::is_same<float, typename std::remove_cv<U>::type>::value, bool>::type
    isNull() const { return (MathUtil::fuzzyIsZero(float(x)) && MathUtil::fuzzyIsZero(float(y))); }

    template<typename U = T>
    inline typename std::enable_if<!std::is_integral<U>::value && std::is_same<double, typename std::remove_cv<U>::type>::value, bool>::type
    isNull() const { return (MathUtil::fuzzyIsZero(double(x)) && MathUtil::fuzzyIsZero(double(y))); }

    // operators

    inline T & operator[](int i)       { return *(&x + i); }
    inline T   operator[](int i) const { return *(&x + i); }

    //! Convert between Vector2T types
    GEOTYPE_TEMPL_INL explicit operator Vector2T<U>() const { return Vector2T<U>(U(x), U(y)); }

    GEOTYPE_TEMPL_INL this_t & operator =  (const Vector2T<U> & v)     { return set(T(v.x), T(v.y)); }  // assign
    GEOTYPE_TEMPL_INL this_t & operator =  (Vector2T<U> && v) noexcept { return set(T(v.x), T(v.y)); }  // move

    GEOTYPE_TEMPL_INL this_t & operator += (const Vector2T<U> & v) { return (*this = *this + v); }
    GEOTYPE_TEMPL_INL this_t & operator -= (const Vector2T<U> & v) { return (*this = *this - v); }
    GEOTYPE_TEMPL_INL this_t & operator *= (const Vector2T<U> & v) { return (*this = *this * v); }
    GEOTYPE_TEMPL_INL this_t & operator /= (const Vector2T<U> & v) { return (*this = *this / v); }

    GEOTYPE_TEMPL_IFNUM_INL this_t & operator += (U s) { return *this += Vector2T<U>(s, s); }
    GEOTYPE_TEMPL_IFNUM_INL this_t & operator -= (U s) { return *this -= Vector2T<U>(s, s); }
    GEOTYPE_TEMPL_IFNUM_INL this_t & operator *= (U s) { return *this *= Vector2T<U>(s, s); }
    GEOTYPE_TEMPL_IFNUM_INL this_t & operator /= (U s) { return *this /= Vector2T<U>(s, s); }

    GEOTYPE_TEMPL_CST_FR_INL this_t operator + (const Vector2T & a, const Vector2T<U> & b) { return this_t(T(a.x + b.x), T(a.y + b.y)); }
    GEOTYPE_TEMPL_CST_FR_INL this_t operator - (const Vector2T & a, const Vector2T<U> & b) { return this_t(T(a.x - b.x), T(a.y - b.y)); }
    GEOTYPE_TEMPL_CST_FR_INL this_t operator * (const Vector2T & a, const Vector2T<U> & b) { return this_t(T(a.x * b.x), T(a.y * b.y)); }
    GEOTYPE_TEMPL_CST_FR_INL this_t operator / (const Vector2T & a, const Vector2T<U> & b) { return this_t(T(a.x / b.x), T(a.y / b.y)); }

    GEOTYPE_TEMPL_IFNUM_CST_FR_INL this_t operator + (const Vector2T & a, U s) { return a + Vector2T<U>(s, s); }
    GEOTYPE_TEMPL_IFNUM_CST_FR_INL this_t operator - (const Vector2T & a, U s) { return a - Vector2T<U>(s, s); }
    GEOTYPE_TEMPL_IFNUM_CST_FR_INL this_t operator * (const Vector2T & a, U s) { return a * Vector2T<U>(s, s); }
    GEOTYPE_TEMPL_IFNUM_CST_FR_INL this_t operator / (const Vector2T & a, U s) { return a / Vector2T<U>(s, s); }

    GEOTYPE_TEMPL_IFNUM_CST_FR_INL this_t operator + (U s, const Vector2T & a) { return a + s; }
    GEOTYPE_TEMPL_IFNUM_CST_FR_INL this_t operator * (U s, const Vector2T & a) { return a * s; }

    GEOTYPE_TEMPL_CST_FR_INL bool operator == (const Vector2T & a, const Vector2T<U> & b) { return (a.x == b.x && a.y == b.y); }
    GEOTYPE_TEMPL_CST_FR_INL bool operator != (const Vector2T & a, const Vector2T<U> & b) { return !(a == b); }
};

typedef Vector2T<int16_t> Vector2;
typedef Vector2T<float>   Vector2F;


/*!
  \brief Template class which defines the size of a 2-dimentional object. It is "cheating" a bit and under
         the hood using a PointT class as base. So x = width, y = height, accessible via named functions.
*/
template<typename T = int>
class Size2T : public Vector2T<T>
{
  public:
    typedef T value_type;
    typedef Size2T<T> this_t;
    using Vector2T<T>::x;
    using Vector2T<T>::y;

    Size2T()                     : Vector2T<T>() {}
    Size2T(T w, T h)             : Vector2T<T>(w, h) {}

    //! Create from other Size2T
    GEOTYPE_TEMPL explicit Size2T(const Size2T<U> & other) : Size2T(T(other.width()), T(other.height())) {}

    //! Create from other Vector2T
    GEOTYPE_TEMPL explicit Size2T(const Vector2T<U> & p) : Size2T(T(p.x), T(p.y)) {}

    T & w() { return x; }               //! non-const accessor
    T & h() { return y; }               //! non-const accessor
    T width() const  { return x; }      //! const accessor
    T height() const { return y; }      //! const accessor

    this_t & setWidth(T w) { x = w; return *this; }
    this_t & setHeight(T h) { y = h; return *this; }

    bool isEmpty() { return (width() <= 0 || height() <= 0); }
};

typedef Size2T<int16_t> Size;
typedef Size2T<float>   SizeF;


/*!
  \brief Template class which defines a positioned rectangle object with \a x and \a y defining the top left corner, and \a w and \a h defining the dimensions.
         Could use some more operators and functions... add here as needed.
*/
template<typename T = int>
class RectT
{
  public:
    static_assert(std::is_arithmetic<T>::value, "Type must be numeric.");
    typedef T value_type;
    typedef Vector2T<T> vect_type;
    typedef Size2T<T> size_type;
    typedef RectT<T> this_t;

    // constructors

    RectT()                   : x(0), y(0), w(0), h(0) {}
    RectT(T x, T y, T w, T h) : x(x), y(y), w(w), h(h) {}
    RectT(T w, T h)           : RectT(0, 0, w, h) {}

    //! Create from other RectT
    GEOTYPE_TEMPL explicit RectT(const RectT<U> & other) : RectT(T(other.x), T(other.y), T(other.w), T(other.h)) {}
    //! Create from a Vector2T \a pos and \a w & \a h
    GEOTYPE_TEMPL explicit RectT(const Vector2T<U> & topLeft, T w, T h) : RectT(T(topLeft.x), T(topLeft.y), w, h) {}
    //! Create from two Vector2T types representing the top left and bottom right corners
    GEOTYPE_TEMPL2 explicit RectT(const Vector2T<U1> & topLeft, const Vector2T<U2> & botRight) : RectT(topLeft, T(botRight.x - topLeft.x), T(botRight.y - topLeft.y)) {}
    //! Create from a Vector2T \a pos and a Size2T \a size
    GEOTYPE_TEMPL2 explicit RectT(const Vector2T<U1> & topLeft, const Size2T<U2> & size) : RectT(topLeft, T(size.width()), T(size.height())) {}
    //! Move constructor
    GEOTYPE_TEMPL RectT(RectT<U> && other) noexcept { set(other.x, other.y, other.w, other.h); }

    // accessors

    T x;
    T y;
    T w;
    T h;

    virtual inline T left()             const { return x; }
    virtual inline T top()              const { return y; }
    virtual inline T right()            const { return x + w; }
    virtual inline T bottom()           const { return y + h; }
    virtual inline vect_type topLeft()  const { return vect_type(x, y); }
    virtual inline vect_type topRight() const { return vect_type(right(), y); }
    virtual inline vect_type botLeft()  const { return vect_type(x, bottom()); }
    virtual inline vect_type botRight() const { return vect_type(right(), bottom()); }
    virtual inline vect_type center()   const { return vect_type(right() - roundf(w / 2), bottom() - roundf(h / 2)); }
    virtual inline size_type size()     const { return size_type(w, h); }

    // setters

    inline this_t & setPosition(T x, T y)   { this->x = x; this->y = y; return *this; }
    inline this_t & setSize(T w, T h)       { this->w = w; this->h = h; return *this; }
    inline this_t & set(T x, T y, T w, T h) { return setPosition(x, y).setSize(w, h); }

    GEOTYPE_TEMPL_INL  this_t & setSize(const Size2T<U> & size)                        { return this->setSize(T(size.width()), T(size.height())); }
    GEOTYPE_TEMPL_INL  this_t & setPosition(const Vector2T<U> & pos)                   { return this->setPosition(T(pos.x), T(pos.y)); }
    GEOTYPE_TEMPL2_INL this_t & set(const Vector2T<U1> & pos, const Size2T<U2> & size) { return setPosition(pos).setSize(size); }

    virtual inline this_t & setLeft(T x)
    {
      T dlta = x - this->x;
      this->x += dlta;
      this->w -= dlta;
      return *this;
    }

    virtual inline this_t & setTop(T y)
    {
      T dlta = y - this->y;
      this->y += dlta;
      this->h -= dlta;
      return *this;
    }

    virtual inline this_t & setRight(T newX)  { w += right() - newX; return *this; }
    virtual inline this_t & setBottom(T newY) { h += bottom() - newY; return *this; }

    GEOTYPE_TEMPL_INL this_t & setTopLeft(const Vector2T<U> & pos)     { return setLeft(T(pos.x)).setTop(T(pos.y)); }
    GEOTYPE_TEMPL_INL this_t & setTopRight(const Vector2T<U> & pos)    { return setRight(T(pos.x)).setTop(T(pos.y)); }
    GEOTYPE_TEMPL_INL this_t & setBottomLeft(const Vector2T<U> & pos)  { return setLeft(T(pos.x)).setBottom(T(pos.y)); }
    GEOTYPE_TEMPL_INL this_t & setBottomRight(const Vector2T<U> & pos) { return setRight(T(pos.x)).setBottom(T(pos.y)); }

    // calculations

    GEOTYPE_TEMPL_INL constexpr bool contains(const Vector2T<U> & point, int8_t margin = 0) const
    {
      return (point.x >= x + margin && point.x <= right() - margin && point.y >= y + margin && point.y <= bottom() - margin);
    }

    GEOTYPE_TEMPL_IFNUM_CST_INL bool contains(U x, U y, int8_t margin = 0) const { return contains(vect_type(x, y), margin); }

    inline bool isEmpty() const { return (x >= right() || y >= bottom()); }
    inline bool isNull()  const { return (x == right() && y == bottom()); }

    // operators

    //! Convert between RectT types
    GEOTYPE_TEMPL_INL operator RectT<U>() const { return RectT<U>(U(x), U(y), U(w), U(h)); }

    GEOTYPE_TEMPL_INL this_t & operator = (const RectT<U> & other)     { return set(other.topLeft(), other.botRight()); }  // assign
    GEOTYPE_TEMPL_INL this_t & operator = (RectT<U> && other) noexcept { return set(other.topLeft(), other.botRight()); }  // move

    GEOTYPE_TEMPL_INL this_t & operator += (const RectT<U> & v) { return (*this = *this + v); }
		GEOTYPE_TEMPL_INL this_t & operator -= (const RectT<U> & v) { return (*this = *this - v); }

		GEOTYPE_TEMPL_CST_FR_INL this_t operator + (const RectT & a, const RectT<U> & b) { return this_t(a.topLeft() - b.topLeft(), a.botRight() + b.botRight()); }
    GEOTYPE_TEMPL_CST_FR_INL this_t operator - (const RectT & a, const RectT<U> & b) { return this_t(a.topLeft() + b.topLeft(), a.botRight() - b.botRight()); }

    GEOTYPE_TEMPL_CST_FR_INL bool operator == (const RectT & a, const RectT<U> & b) { return (a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h); }
    GEOTYPE_TEMPL_CST_FR_INL bool operator != (const RectT & a, const RectT<U> & b) { return !(a == b); }
};

typedef RectT<int16_t> Rect;
typedef RectT<float>   RectF;


/*!
  \brief Template class which defines spacing (margins/padding/etc) around an object. It is "cheating" a bit and under
         the hood using a RectT class as base. So x = left margin, y = top, w = right, and h = bottom, accessible via named functions.
*/
template<typename T = int>
class MarginsT : public RectT<T>
{
  public:
    typedef T value_type;
    typedef MarginsT<T> this_t;
    using RectT<T>::x;
    using RectT<T>::y;
    using RectT<T>::w;
    using RectT<T>::h;

    MarginsT() : RectT<T>() {}
    explicit MarginsT(T left, T top, T right, T bottom) : RectT<T>(left, top, right, bottom) {}
    GEOTYPE_TEMPL explicit MarginsT(const MarginsT<U> & other) : MarginsT(T(other.x), T(other.y), T(other.w), T(other.h)) {}

    T & l() { return x; }               //! non-const accessor
    T & t() { return y; }               //! non-const accessor
    T & r() { return w; }               //! non-const accessor
    T & b() { return h; }               //! non-const accessor

    T left()   const { return x; }      //! const accessor
    T top()    const { return y; }      //! const accessor
    T right()  const { return w; }      //! const accessor
    T bottom() const { return h; }      //! const accessor

    T horizontal() const { return left() + right(); }
    T vertical()   const { return top() + bottom(); }

    this_t & setLeft(T left)     { l() = left; return *this; }
    this_t & setTop(T top)       { t() = top; return *this; }
    this_t & setRight(T right)   { r() = right; return *this; }
    this_t & setBottom(T bottom) { b() = bottom; return *this; }

    //! conveniences to add/subtract margins to/from rectangle
    GEOTYPE_TEMPL_CST_FR_INL RectT<U> operator + (const RectT<U> & a, const MarginsT & b)  {  return RectT<U>(a) += b;  }
    GEOTYPE_TEMPL_CST_FR_INL RectT<U> operator - (const RectT<U> & a, const MarginsT & b)  {  return RectT<U>(a) -= b;  }
};

typedef MarginsT<int16_t> Margins;
typedef MarginsT<float>   MarginsF;


#endif // GEOMETRY_TYPES_H
