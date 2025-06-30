#include "geometry.h"
#include <cassert>
#define VEC_ONE 1


template <> template <> Vec3<int>::Vec3(const Vec3<float>& v) : x(int(v.x + .5)), y(int(v.y + .5)), z(int(v.z + .5)) {
}

template <> template <> Vec3<float>::Vec3(const Vec3<int>& v) : x(v.x), y(v.y), z(v.z) {
}


Matrix::Matrix(int rows, int cols)
{
	m_rows = rows;
	m_cols = cols;
	for (int i = 0; i < rows; i++)
	{
		std::vector<float> tempVec;
		for (int j = 0; j < cols; j++)
		{
			tempVec.push_back(0.0f);
		}
		m.push_back(tempVec);
	}
}
Matrix Matrix::Identity(int dimension)
{
	Matrix identityMat(dimension, dimension);
	for (int i = 0; i < dimension; i++)
	{
		for (int j = 0; j < dimension; j++)
		{
			if (i == j) identityMat[i][j] = 1.0f;
		}
		
	}
	return identityMat;
}

Matrix Matrix::transpose()
{
	Matrix transposedMat(m_cols, m_rows);
	for (int i = 0; i < m_rows; i++)
	{
		for (int j = 0; j < m_cols; j++)
		{
			transposedMat[j][i] = m[i][j];
		}
	}
	return transposedMat;
}

Matrix Matrix::operator*(const Matrix& a) const
{
	assert(m_cols == a.GetRows());
	Matrix result(m_rows, a.GetCols());
	for (int i = 0; i < m_rows; i++) {
		for (int j = 0; j < a.GetCols(); j++) {
			result.m[i][j] = 0.f;
			for (int k = 0; k < GetCols(); k++) {
				result.m[i][j] += m[i][k] * a.m[k][j];
			}
		}
	}
	return result;
}

std::vector<float> Matrix::operator*(const std::vector<float>& vec)
{
	assert(m_cols == vec.size());
	std::vector<float> result(m_rows, 0.0f);
	for (int i = 0; i < m_rows; i++) 
	{
		for (int j = 0; j < vec.size(); j++) 
		{
			result[i] += m[i][j] * vec[j];
		}
	}
	return result;
}



