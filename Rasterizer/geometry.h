#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <iostream>
#include <vector>
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class t> struct Vec2 {
    t x, y;
    Vec2<t>() : x(t()), y(t()) {}
    Vec2<t>(t _x, t _y) : x(_x), y(_y) {}
    Vec2<t>(const Vec2<t>& v) : x(t()), y(t()) { *this = v; }
    Vec2<t>& operator =(const Vec2<t>& v) {
        if (this != &v) {
            x = v.x;
            y = v.y;
        }
        return *this;
    }
    Vec2<t> operator +(const Vec2<t>& V) const { return Vec2<t>(x + V.x, y + V.y); }
    Vec2<t> operator -(const Vec2<t>& V) const { return Vec2<t>(x - V.x, y - V.y); }
    Vec2<t> operator *(float f)          const { return Vec2<t>(x * f, y * f); }
    t& operator[](const int i) { if (i <= 0) return x; else return y; }
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

template <class t> struct Vec3 {
    t x, y, z;
    Vec3<t>() : x(t()), y(t()), z(t()) {}
    Vec3<t>(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
    template <class u> Vec3<t>(const Vec3<u>& v);
    Vec3<t>(const Vec3<t>& v) : x(t()), y(t()), z(t()) { *this = v; }
    Vec3<t>(const std::vector<t>& otherVec):Vec3(otherVec[0], otherVec[1], otherVec[2]){}
    Vec3<t>& operator =(const Vec3<t>& v) {
        if (this != &v) {
            x = v.x;
            y = v.y;
            z = v.z;
        }
        return *this;
    }
    Vec3<t> operator ^(const Vec3<t>& v) const { return Vec3<t>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
    Vec3<t> operator +(const Vec3<t>& v) const { return Vec3<t>(x + v.x, y + v.y, z + v.z); }
    Vec3<t> operator -(const Vec3<t>& v) const { return Vec3<t>(x - v.x, y - v.y, z - v.z); }
    Vec3<t> operator *(float f)          const { return Vec3<t>(x * f, y * f, z * f); }
    t       operator *(const Vec3<t>& v) const { return x * v.x + y * v.y + z * v.z; }
    float norm() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3<t>& normalize(t l = 1) { *this = (*this) * (l / norm()); return *this; }
    t& operator[](const int i) { if (i <= 0) return x; else if (i == 1) return y; else return z; }
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};

typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;

template <> template <> Vec3<int>::Vec3(const Vec3<float>& v);
template <> template <> Vec3<float>::Vec3(const Vec3<int>& v);

template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
	s << "(" << v.x << ", " << v.y << ")\n";
	return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
	s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
	return s;
}


class Matrix
{
public:
	Matrix(int rows, int cols);
	inline int GetRows() const { return m_rows; }
	inline int GetCols() const { return m_cols; }

	static Matrix Identity(int dimension);
	Matrix transpose();
	Matrix operator*(const Matrix& otherMat) const;
	std::vector<float> operator*(const std::vector<float>& vec);
	std::vector<float>& operator[](const int i) { return m[i]; }
	std::vector<float>operator[](const int i) const { return m[i]; }


private:
	std::vector<std::vector<float>> m;
	int m_rows, m_cols;
};




#endif //__GEOMETRY_H__