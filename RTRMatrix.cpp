#include "RTRMatrix.h"
#include <QtCore>
#include <QColor>

RTRMatrix::RTRMatrix()
{
	size = 0;
	//data = NULL;
}

RTRMatrix::RTRMatrix(int _rows, int _columns)
{
	rows = _rows;
	columns = _columns;
	size = rows * columns;
	//data = new double[size];
}

RTRMatrix::RTRMatrix(const RTRMatrix &other)
{
	rows = other.rows;
	columns = other.columns;
	size = other.size;
	//data = new double[size];
	memcpy(data,other.data,sizeof(double)* size);
}

RTRMatrix::~RTRMatrix()
{
	//delete[] data;
}

double& RTRMatrix::elementAt(int i, int j)
{
	Q_ASSERT(data != NULL);
	Q_ASSERT(i>=0 && i<rows &&j>=0&&j<columns);
	return data[i*columns+j];
}

const double& RTRMatrix::elementAt(int i, int j) const
{
	Q_ASSERT(data != NULL);
	Q_ASSERT(i>=0 && i<rows &&j>=0&&j<columns);
	return data[i*columns+j];
}

double& RTRMatrix::elementAt(int n)
{
	Q_ASSERT(data != NULL);
	Q_ASSERT(n>=0&&n<size);
	return data[n];
}

const double& RTRMatrix::elementAt(int n) const
{
	Q_ASSERT(data != NULL);
	Q_ASSERT(n>=0&&n<size);
	return data[n];
}

int RTRMatrix::getRows() const
{
	return rows;
}

int RTRMatrix::getColumns() const
{
	return columns;
}

int RTRMatrix::getSize() const
{
	return size;
}

double& RTRMatrix::operator() (int i,int j)
{
	return elementAt(i,j);
}

const double& RTRMatrix::operator() (int i,int j) const
{
	return elementAt(i,j);
}

double& RTRMatrix::operator() (int n)
{
	return elementAt(n);
}

const double& RTRMatrix::operator() (int n) const
{
	return elementAt(n);
}



void RTRMatrix::fill(double value)
{
	Q_ASSERT(data != NULL);
	for(int i=0;i<size;i++)
	{
		data[i] = value;
	}
}

const RTRMatrix& RTRMatrix::operator= (const RTRMatrix& other)
{
	if(size == 0)
	{
		rows = other.rows;
		columns = other.columns;
		size = other.size;
		//data = new double[size];
		memcpy(data,other.data,sizeof(double)* size);
	}
	else
	{
		if(size != other.size)
		{
			size ++ ;
		}
		Q_ASSERT(size == other.size);
		for(int i = 0; i < size; i++)
		{
			data[i] = other.data[i];
		}
	}
	return *this;
}

bool RTRMatrix::operator== (const RTRMatrix& other) const
{
	Q_ASSERT(data != NULL && other.data!=NULL);
	if(columns!=other.columns || rows!=other.rows) return false;
	for(int i = 0; i < size; i++)
	{
		if(data[i] != other.data[i]) return false;
	}
	return true;
}

RTRMatrix RTRMatrix::operator* (const RTRMatrix& other) const
{
	Q_ASSERT(data != NULL && other.data!=NULL);
	Q_ASSERT(columns == other.rows);
	RTRMatrix ret(rows, other.columns);
	ret.fill(0);
	for(int i=0;i<ret.rows;i++)
	{
		for(int j=0;j<ret.columns;j++)
		{
			for(int k=0;k<columns;k++)
			{
				ret(i,j) += (*this)(i,k)*other(k,j);
			}
		}
	}
	return ret;
}

RTRMatrix RTRMatrix::operator+ (const RTRMatrix& other) const
{
	Q_ASSERT(data != NULL && other.data!=NULL);
	Q_ASSERT(size == other.size);
	RTRMatrix ret(rows, other.columns);
	ret.fill(0);
	for(int i=0;i<ret.rows;i++)
	{
		for(int j=0;j<ret.columns;j++)
		{
			ret(i,j) = (*this)(i,j)+other(i,j);
		}
	}
	return ret;
}

RTRMatrix RTRMatrix::operator- (const RTRMatrix& other) const
{
	Q_ASSERT(size == other.size);
	RTRMatrix ret(rows, other.columns);
	ret.fill(0);
	for(int i=0;i<ret.rows;i++)
	{
		for(int j=0;j<ret.columns;j++)
		{
			ret(i,j) = (*this)(i,j) - other(i,j);
		}
	}
	return ret;
}

RTRMatrix&  RTRMatrix::operator*= (const double val)
{
	for(int i=0;i<this->size;i++)
	{
		elementAt(i) *= val;
	}
	return *this;
}
RTRMatrix RTRMatrix::operator* (const double val) const
{
	RTRMatrix ret(rows, columns);
	for(int i=0;i<size;i++)
	{
		ret.elementAt(i) = (*this)(i)*val;
	}
	return ret;
}


RTRMatrix RTRMatrix::operator/ (const double val) const
{
	return operator*(1/val);
}

RTRMatrix& RTRMatrix::operator/= (const double val)
{
	return operator*=(1/val);
}

RTRMatrix::RTRMatrix(int n)
{
	rows = n;
	columns = 1;
	size = rows * columns;
	//data = new double[rows*columns];
}

bool RTRMatrix::isColumnVector() const
{
	return columns == 1;
}

bool RTRMatrix::isRowVector() const
{
	return rows == 1;
}

bool RTRMatrix::isVector() const
{
	return isRowVector()||isColumnVector();
}

bool RTRMatrix::isNumber() const
{
	return rows==1 && columns ==1;
}

void RTRMatrix::vectorTranspose()
{
	Q_ASSERT(isVector());
	qSwap(rows,columns);
}

bool RTRMatrix::vectorNormalize()
{
	Q_ASSERT(isVector());
	double ratio = 0;
	for(int i=0;i<size;i++)
	{
		ratio+=(elementAt(i)*elementAt(i));
	}
	ratio = sqrt(ratio);
	if(ratio == 0) return false;
	for(int i=0;i<size;i++)
	{
		elementAt(i) /= ratio;
	}
	return true;
}

double RTRMatrix::vectorLength() const
{
	return sqrt(vectorLengthSquared());
}

double RTRMatrix::vectorLengthSquared() const
{
	Q_ASSERT(isVector());
	double ret = 0.0;
	for(int i=0;i<size;i++)
	{
		ret+=elementAt(i)*elementAt(i);
	}
	return ret;
}

double& RTRMatrix::x()
{
	Q_ASSERT(isVector()&&size>=1);
	return data[0];
}

double RTRMatrix::x() const
{
	Q_ASSERT(isVector()&&size>=1);
	return data[0];
}

double& RTRMatrix::y()
{
	Q_ASSERT(isVector()&&size>=2);
	return data[1];
}

double RTRMatrix::y() const
{
	Q_ASSERT(isVector()&&size>=2);
	return data[1];
}

double& RTRMatrix::z()
{
	Q_ASSERT(isVector()&&size>=3);
	return data[2];
}

double RTRMatrix::z() const
{
	Q_ASSERT(isVector()&&size>=3);
	return data[2];
}

double& RTRMatrix::r()
{
	return x();
}

double RTRMatrix::r() const
{
	return x();
}

double& RTRMatrix::g()
{
	return y();
}

double RTRMatrix::g() const
{
	return y();
}

double& RTRMatrix::b()
{
	return z();
}

double RTRMatrix::b() const
{
	return z();
}

double& RTRMatrix::a()
{
	Q_ASSERT(isVector()&&size>=4);
	return data[3];
}

double RTRMatrix::a() const
{
	Q_ASSERT(isVector()&&size>=4);
	return data[3];
}

RTRMatrix RTRVector::crossProduct(const RTRMatrix& other) const
{
	Q_ASSERT(isVector()&&size==3&&other.isVector()&&other.size==3);
	RTRVector ret(3);
	ret.x() = y()*other.z()-z()*other.y();
	ret.y() = z()*other.x()-x()*other.z();
	ret.z() = x()*other.y()-y()*other.x();
	return ret;
}

double RTRVector::dotProduct(const RTRMatrix &other) const
{
	Q_ASSERT(isVector()&&other.isVector()&&size == other.size);
	double ret = 0.0;
	for(int i=0;i<size;i++)
	{
		ret += (elementAt(i)*other(i));
	}
	return ret;
}

RTRMatrix::RTRMatrix(double x, double y, double z)
{
	rows = 3;
	columns = 1;
	size = rows * columns;
	//data = new double[rows*columns];
	data[0] = x;
	data[1] = y;
	data[2] = z;
}

QColor RTRMatrix::toQtColor() const
{
	Q_ASSERT(isVector()&&size==3||size==4);
	QColor ret;
	if(r()<0) ret.setRed(0);
	else if(r()>1) ret.setRedF(1.0);
	else ret.setRedF(r());

	if(g()<0) ret.setGreen(0);
	else if(g()>1) ret.setGreenF(1.0);
	else ret.setGreenF(r());

	if(b()<0) ret.setBlue(0);
	else if(b()>1) ret.setBlueF(1.0);
	else ret.setBlueF(b());
	return ret;
}
