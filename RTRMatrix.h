#ifndef RTRMATRIX_H
#define RTRMATRIX_H

#include <iostream>
using namespace std;

class QColor;

class RTRMatrix
{
protected:
	int rows;
	int columns;
	int size;
	double data[9];
public:
	RTRMatrix();
	RTRMatrix(int _rows, int _columns);
	RTRMatrix(double x, double y, double z);
	RTRMatrix(const RTRMatrix& other);
	~RTRMatrix();
	int getRows() const;
	int getColumns() const;
	int getSize() const;
	double& elementAt(int i, int j);
	const double& elementAt(int i, int j) const;
	double& elementAt(int n);
	const double& elementAt(int n) const;
	double& operator() (int i,int j);
	const double& operator() (int i,int j) const;
	double& operator() (int n);
	const double& operator() (int n) const;
	void fill(double value = 0);
	const RTRMatrix& operator= (const RTRMatrix& other);
	bool operator== (const RTRMatrix& other) const;
	RTRMatrix operator* (const RTRMatrix& other) const;
	RTRMatrix operator* (const double val) const;
	RTRMatrix& operator*= (const double val);
	RTRMatrix operator/ (const double val) const;
	RTRMatrix& operator/= (const double val);
	RTRMatrix operator+ (const RTRMatrix& other) const;
	RTRMatrix operator- (const RTRMatrix& other) const;

	RTRMatrix(int size);
	bool isColumnVector() const;
	bool isRowVector() const;
	bool isVector() const;
	bool isNumber() const;
	void vectorTranspose();
	bool vectorNormalize();
	double vectorLength() const;
	double vectorLengthSquared() const;
	double dotProduct(const RTRMatrix& other) const;
	void printDebugInfo() const
	{
		cout << "[" <<endl;
		for(int i=0;i<rows;i++)
		{
			for(int j=0;j<columns;j++)
			{
				cout << (*this)(i,j) << ",";
			}
			cout << endl;
		}
		cout << "]" <<endl;
	}
};

typedef RTRMatrix RTRVector;

#endif // RTRMATRIX_H
