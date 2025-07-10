//#include <vector>
//#include <cassert>
//#include <cmath>
//#include <iostream>
//#include "geometry.h"
//
//#define VEC_ONE 1
//
//template <> template <> Vec3<int>::Vec3(const Vec3<float>& v) : x(int(v.x + .5)), y(int(v.y + .5)), z(int(v.z + .5)) {
//}
//
//template <> template <> Vec3<float>::Vec3(const Vec3<int>& v) : x(v.x), y(v.y), z(v.z) {
//}
//
//
//
//Matrix::Matrix(int rows, int cols)
//{
//	m_rows = rows;
//	m_cols = cols;
//	for (int i = 0; i < rows; i++)
//	{
//		std::vector<float> tempVec;
//		for (int j = 0; j < cols; j++)
//		{
//			tempVec.push_back(0.0f);
//		}
//		m.push_back(tempVec);
//	}
//}
//
//Matrix::Matrix(Vec3f v) : m(std::vector<std::vector<float> >(4, std::vector<float>(1, 1.f))), m_rows(4), m_cols(1) {
//	m[0][0] = v.x;
//	m[1][0] = v.y;
//	m[2][0] = v.z;
//}
//
//Matrix Matrix::Identity(int dimension)
//{
//	Matrix identityMat(dimension, dimension);
//	for (int i = 0; i < dimension; i++)
//	{
//		for (int j = 0; j < dimension; j++)
//		{
//			if (i == j) identityMat[i][j] = 1.0f;
//		}
//		
//	}
//	return identityMat;
//}
//
//Matrix Matrix::transpose()
//{
//	Matrix transposedMat(m_cols, m_rows);
//	for (int i = 0; i < m_rows; i++)
//	{
//		for (int j = 0; j < m_cols; j++)
//		{
//			transposedMat[j][i] = m[i][j];
//		}
//	}
//	return transposedMat;
//}
//
//Matrix Matrix::operator*(const Matrix& a) const
//{
//	assert(m_cols == a.GetRows());
//	Matrix result(m_rows, a.GetCols());
//	for (int i = 0; i < m_rows; i++) {
//		for (int j = 0; j < a.GetCols(); j++) {
//			result.m[i][j] = 0.f;
//			for (int k = 0; k < GetCols(); k++) {
//				result.m[i][j] += m[i][k] * a.m[k][j];
//			}
//		}
//	}
//	return result;
//}
//
//std::vector<float> Matrix::operator*(const std::vector<float>& vec)
//{
//	assert(m_cols == vec.size());
//	std::vector<float> result(m_rows, 0.0f);
//	for (int i = 0; i < m_rows; i++) 
//	{
//		for (int j = 0; j < vec.size(); j++) 
//		{
//			result[i] += m[i][j] * vec[j];
//		}
//	}
//	return result;
//}
//
////Vec3f Matrix::operator*(const Vec3f& vec3)
////{
////	assert(m_cols == 4 && m_rows == 4);
////	std::vector<float> tempResult;
////	const std::vector<float> vec{ vec3[0],vec3[1],vec3[2], 1.0f };
////	tempResult = *this * vec;
////	Vec3f result{ tempResult[0]/ tempResult[3],tempResult[1] / tempResult[3],tempResult[2] / tempResult[3] };
////	return result;
////}
//
//
//
//


#include <vector>
#include <cassert>
#include <cmath>
#include <iostream>
#include "geometry.h"

template <> Vec3<float>::Vec3(Matrix m) : x(m[0][0] / m[3][0]), y(m[1][0] / m[3][0]), z(m[2][0] / m[3][0]) {}
template <> template <> Vec3<int>::Vec3(const Vec3<float>& v) : x(int(v.x + .5)), y(int(v.y + .5)), z(int(v.z + .5)) {}
template <> template <> Vec3<float>::Vec3(const Vec3<int>& v) : x(v.x), y(v.y), z(v.z) {}

Matrix::Matrix(Vec3f v) : m(std::vector<std::vector<float> >(4, std::vector<float>(1, 1.f))), rows(4), cols(1) {
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
}


Matrix::Matrix(int r, int c) : m(std::vector<std::vector<float> >(r, std::vector<float>(c, 0.f))), rows(r), cols(c) {}

int Matrix::nrows() {
    return rows;
}

int Matrix::ncols() {
    return cols;
}

Matrix Matrix::identity(int dimensions) {
    Matrix E(dimensions, dimensions);
    for (int i = 0; i < dimensions; i++) {
        for (int j = 0; j < dimensions; j++) {
            E[i][j] = (i == j ? 1.f : 0.f);
        }
    }
    return E;
}

std::vector<float>& Matrix::operator[](const int i) {
    assert(i >= 0 && i < rows);
    return m[i];
}

Matrix Matrix::operator*(const Matrix& a) {
    assert(cols == a.rows);
    Matrix result(rows, a.cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < a.cols; j++) {
            result.m[i][j] = 0.f;
            for (int k = 0; k < cols; k++) {
                result.m[i][j] += m[i][k] * a.m[k][j];
            }
        }
    }
    return result;
}

Matrix Matrix::transpose() {
    Matrix result(cols, rows);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            result[j][i] = m[i][j];
    return result;
}

Matrix Matrix::inverse() {
    assert(rows == cols);
    // augmenting the square matrix with the identity matrix of the same dimensions a => [ai]
    Matrix result(rows, cols * 2);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            result[i][j] = m[i][j];
    for (int i = 0; i < rows; i++)
        result[i][i + cols] = 1;
    // first pass
    for (int i = 0; i < rows - 1; i++) {
        // normalize the first row
        for (int j = result.cols - 1; j >= 0; j--)
            result[i][j] /= result[i][i];
        for (int k = i + 1; k < rows; k++) {
            float coeff = result[k][i];
            for (int j = 0; j < result.cols; j++) {
                result[k][j] -= result[i][j] * coeff;
            }
        }
    }
    // normalize the last row
    for (int j = result.cols - 1; j >= rows - 1; j--)
        result[rows - 1][j] /= result[rows - 1][rows - 1];
    // second pass
    for (int i = rows - 1; i > 0; i--) {
        for (int k = i - 1; k >= 0; k--) {
            float coeff = result[k][i];
            for (int j = 0; j < result.cols; j++) {
                result[k][j] -= result[i][j] * coeff;
            }
        }
    }
    // cut the identity matrix back
    Matrix truncate(rows, cols);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            truncate[i][j] = result[i][j + cols];
    return truncate;
}

std::ostream& operator<<(std::ostream& s, Matrix& m) {
    for (int i = 0; i < m.nrows(); i++) {
        for (int j = 0; j < m.ncols(); j++) {
            s << m[i][j];
            if (j < m.ncols() - 1) s << "\t";
        }
        s << "\n";
    }
    return s;
}
