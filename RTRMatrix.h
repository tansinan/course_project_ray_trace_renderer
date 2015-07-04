#ifndef RTRMATRIX_H
#define RTRMATRIX_H

#include <iostream>
#include <QtCore>
using namespace std;

class QColor;

template<int R, int C>
class RTRMatrix
{
protected:
	double data[R*C];
public:
	int getRows() const { return R; }
	int getColumns() const { return C; }
	int getSize() const { return R*C; }
	double& elementAt(int i, int j)
	{
		Q_ASSERT(i >= 0 && i < R &&j >= 0 && j < C);
		return data[i*C + j];
	}

	const double& elementAt(int i, int j) const
	{
		Q_ASSERT(i >= 0 && i < R &&j >= 0 && j < C);
		return data[i*C + j];
	}

	double& elementAt(int n)
	{
		Q_ASSERT(n >= 0 && n < R*C);
		return data[n];
	}

	const double& elementAt(int n) const
	{
		Q_ASSERT(n >= 0 && n < R*C);
		return data[n];
	}

	double& operator() (int i, int j) { return elementAt(i, j); }
	const double& operator() (int i, int j) const  { return elementAt(i, j); }
	double& operator() (int n) { return elementAt(n); }
	const double& operator() (int n) const { return elementAt(n); }
	void fill(double value = 0)
	{
		for (int i = 0; i < R*C; i++)
		{
			data[i] = value;
		}
	}
	const RTRMatrix& operator= (const RTRMatrix& other)
	{
		for (int i = 0; i < R*C; i++)
		{
			data[i] = other.data[i];
		}
		return *this;
	}

	bool operator== (const RTRMatrix<R,C>& other) const
	{
		for (int i = 0; i < R*C; i++)
		{
			if (data[i] != other.data[i]) return false;
		}
		return true;
	}

	RTRMatrix operator* (const double val) const
	{
		RTRMatrix<R, C> ret;
		for (int i = 0; i < R*C; i++) ret.elementAt(i) = (*this)(i)*val;
		return ret;
	}

	RTRMatrix& operator*= (const double val)
	{
		for (int i = 0; i < R*C; i++) elementAt(i) *= val;
		return *this;
	}
	RTRMatrix operator/ (const double val) const { return operator*(1 / val); }
	RTRMatrix& operator/= (const double val) { return operator*=(1 / val); }
	RTRMatrix operator+ (const RTRMatrix& other) const
	{
		RTRMatrix<R, C> ret;
		for (int i = 0; i<R; i++)
		{
			for (int j = 0; j<C; j++) ret(i, j) = (*this)(i, j) + other(i, j);
		}
		return ret;
	}

	RTRMatrix operator- (const RTRMatrix& other) const
	{
		RTRMatrix<R, C> ret;
		for (int i = 0; i<R; i++)
		{
			for (int j = 0; j<C; j++) ret(i, j) = (*this)(i, j) - other(i, j);
		}
		return ret;
	}

	bool isVector() const{ return R == 1 || C == 1; }
	bool vectorNormalize()
	{
		Q_ASSERT(isVector());
		double ratio = 0;
		for (int i = 0; i<R*C; i++)
		{
			ratio += (elementAt(i)*elementAt(i));
		}
		ratio = sqrt(ratio);
		if (ratio == 0) return false;
		for (int i = 0; i<R*C; i++)
		{
			elementAt(i) /= ratio;
		}
		return true;
	}

	double vectorLength() const
	{
		return sqrt(vectorLengthSquared());
	}

	double vectorLengthSquared() const
	{
		Q_ASSERT(isVector());
		double ret = 0.0;
		for (int i = 0; i<R*C; i++)
		{
			ret += elementAt(i)*elementAt(i);
		}
		return ret;
	}

	double dotProduct(const RTRMatrix<R,C>& other) const
	{
		Q_ASSERT(isVector());
		double ret = 0.0;
		for (int i = 0; i<R*C; i++)
		{
			ret += (elementAt(i)*other(i));
		}
		return ret;
	}
	/*void printDebugInfo() const
	{
		cout << "[" << endl;
		for (int i = 0; i<rows; i++)
		{
			for (int j = 0; j<columns; j++)
			{
				cout << (*this)(i, j) << ",";
			}
			cout << endl;
		}
		cout << "]" << endl;
	}*/
	template<int M>
	RTRMatrix<R, M> operator* (const RTRMatrix<C, M>& other) const
	{
		RTRMatrix<R, M> ret;
		ret.fill(0);
		for (int i = 0; i < R; i++)
		{
			for (int j = 0; j < M; j++)
			{
				for (int k = 0; k < C; k++)
				{
					ret(i, j) += (*this)(i, k)*other(k, j);
				}
			}
		}
		return ret;
	}
};

typedef RTRMatrix<3, 3> RTRMatrix33;

#endif // RTRMATRIX_H
