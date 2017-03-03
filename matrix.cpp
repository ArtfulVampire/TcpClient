#include "matrix.h"

using namespace std;

matrix::matrix()
{

}

matrix::~matrix()
{
}

matrix::matrix(int dim)
{
    this->resize(dim, dim);
    this->fill(0.);
}

matrix::matrix(int rows, int cols)
{
    this->resize(rows, cols);
    this->fill(0.);
}

matrix::matrix(const matrix & other) : matrix()
{
    this->data = other.data;
}


matrix::matrix(const std::vector<std::valarray<double>> & other)
{
    this->data = other;
}
matrix::matrix(const std::valarray<double> & vect, bool orientH)
{
    if(orientH)
    {
        this->resize(1, vect.size());
        this->data[0] = vect;
    }
    else
    {
        this->resize(vect.size(), 1);
        for(int i = 0; i < vect.size(); ++i)
        {
            this->data[i][0] = vect[i];
        }
    }
}
matrix::matrix(const std::valarray<double> & vect, char orient)
{
    if(orient == 'h' || orient == 'H')
    {
        this->resize(1, vect.size());
        this->data[0] = vect;
    }
    else if(orient == 'v' || orient == 'V')
    {
        this->resize(vect.size(), 1);
        for(int i = 0; i < vect.size(); ++i)
        {
            this->data[i][0] = vect[i];
        }
    }
    else
    {
        matrix();
    }
}

matrix::matrix(const std::valarray<double> & vect, int inRows)
{
    if(vect.size() % inRows != 0)
    {
        cout << "not appropriate size" << endl;
        return;
    }
    int newCols = vect.size() / inRows;

    this->resize(inRows, newCols);
    for(int i = 0; i < inRows; ++i)
    {
        std::copy(std::begin(vect) + i * newCols,
                  std::begin(vect) + (i + 1) * newCols,
                  std::begin(this->data[i]));
    }
}

matrix::matrix(const std::valarray<double> & vect1, const std::valarray<double> & vect2)
{
    this->data.clear();
    for(int i = 0; i < vect1.size(); ++i)
    {
        this->data.push_back(vect1[i] * vect2);
    }
}

matrix::matrix(std::initializer_list<std::valarray<double>> lst)
{
    this->resize(0, 0);
    std::for_each(lst.begin(),
                  lst.end(),
                  [this](std::valarray<double> in)
    {
        this->data.push_back(in);
    });
}

matrix::matrix(std::initializer_list<double> lst) // diagonal
{
    this->resize(lst.size(), lst.size());
    this->fill(0.);
    int count = 0;
    for(int item : lst)
    {
        this->data[count][count] = item;
        ++count;
    }
}

matrix matrix::operator = (const matrix & other)
{
    this->data = other.data;

    return *this;
}
matrix matrix::operator = (const std::vector<std::valarray<double>> & other)
{
    this->data = other;

    return *this;
}

matrix operator + (const matrix & lhs, const matrix & rhs)
{
    if(lhs.rows() != rhs.rows()
       || lhs.cols() != rhs.cols())
    {
        cout << "matrix sum failed, dimensions" << endl;
        return lhs;
    }
    matrix result(lhs.rows(), lhs.cols());
    for(int i = 0; i < lhs.rows(); ++i)
    {
        result[i] = lhs[i] + rhs[i];
    }
    return result;
}

matrix operator + (const matrix & lhs, const double & val)
{
    matrix result;
    for(int i = 0; i < lhs.rows(); ++i)
    {
        result.push_back(lhs[i] + val);
    }
    return result;
}

matrix matrix::operator += (const matrix & other)
{
    if(this->rows() != other.rows()
       || this->cols() != other.cols())
    {
        cout << "matrix sum failed" << endl;
        return *this;
    }
    for(int i = 0; i < this->rows(); ++i)
    {
        (*this)[i] += other[i];
    }
    return *this;
}

matrix matrix::operator += (const double & val)
{
    for(int i = 0; i < this->rows(); ++i)
    {
        (*this)[i] += val;
    }
    return *this;
}

matrix operator - (const matrix & lhs, const matrix & rhs)
{
    if(lhs.rows() != rhs.rows()
       || lhs.cols() != rhs.cols())
    {
        cout << "matrix sum failed, dimensions" << endl;
        return lhs;
    }
    matrix result(lhs.rows(), lhs.cols());
    for(int i = 0; i < lhs.rows(); ++i)
    {
        result[i] = lhs[i] - rhs[i];
    }
    return result;
}

matrix operator - (const matrix & lhs, const double & val)
{
    matrix result;
    for(int i = 0; i < lhs.rows(); ++i)
    {
        result.push_back(lhs[i] - val);
    }
    return result;
}

matrix matrix::operator -= (const matrix & other)
{
    if(this->rows() != other.rows()
       || this->cols() != other.cols())
    {
        cout << "matrix sum failed" << endl;
        return *this;
    }
    for(int i = 0; i < this->rows(); ++i)
    {
        (*this)[i] -= other[i];
    }
    return *this;
}

matrix matrix::operator -= (const double & val)
{
    for(int i = 0; i < this->rows(); ++i)
    {
        (*this)[i] -= val;
    }
    return *this;
}

matrix operator * (const matrix & lhs, const matrix & rhs)
{
    if(lhs.cols() != rhs.rows())
    {
        cout << "matrixProduct (operator *): input matrices are not productable" << endl;
        return lhs;
    }

    const int dim1 = lhs.rows();
    const int dim2 = rhs.cols();

    matrix result(dim1, dim2);
#if 1

    for(int j = 0; j < dim2; ++j)
    {
        std::valarray<double> currCol = rhs.getCol(j);
        for(int i = 0; i < dim1; ++i)
		{
            result[i][j] = prod(lhs[i], currCol);
        }
    }
#else
    const matrix temp = matrix::transpose(rhs);

    for(int i = 0; i < dim1; ++i)
    {
        for(int j = 0; j < dim2; ++j)
        {
            result[i][j] = prod(lhs[i], temp[j]);
        }
    }
#endif
    return result;
}

matrix operator * (const matrix & lhs, const double & val)
{
    matrix result;
    for(int i = 0; i < lhs.rows(); ++i)
    {
        result.push_back(lhs[i] * val);
    }
    return result;
}

matrix matrix::operator *= (const double & other)
{
    for(int i = 0; i < this->rows(); ++i)
    {
        this->data[i] *= other;
    }
    return *this;
}

matrix matrix::operator *= (const matrix & other)
{
    /// OMG
    (*this) = (*this) * other;
    return (*this);
}

matrix operator / (const matrix & lhs, const double & val)
{
    matrix result(lhs.rows(), lhs.cols());
    for(int i = 0; i < lhs.rows(); ++i)
    {
        result[i] = lhs[i] / val;
    }
    return result;
}

matrix matrix::operator /= (const double & other)
{
    for(int i = 0; i < this->rows(); ++i)
    {
        this->data[i] /= other;

    }
    return *this;
}

matrix::matrix(int rows, int cols, double value)
{
    this->resize(rows, cols);
    this->fill(value);
}


void matrix::fill(double value)
{
    for(std::vector<std::valarray<double> >::iterator it = data.begin(); it < data.end(); ++it)
    {
        (*it) = value;
    }
}

void matrix::resize(int rows, int cols, double val)
{
    this->resize(rows, cols);
    this->fill(val);
}

void matrix::resize(int newRows, int newCols)
{
    this->data.resize(newRows);
    std::for_each(data.begin(),
                  data.end(),
                  [newCols](std::valarray<double> & in)
    {
        std::valarray<double> temp = in;
        in.resize(newCols);
        std::copy(std::begin(temp),
                  std::begin(temp) + min(newCols, int(temp.size())),
                  std::begin(in));
	});

}


void matrix::resizeRows(int newRows)
{
    int cols = this->cols();
    int oldRows = this->rows();
	data.resize(newRows);
    if(oldRows < newRows) /// why?
    {
        std::for_each(data.begin() + oldRows,
                      data.end(),
                      [cols](std::valarray<double> & in)
        {
            std::valarray<double> temp = in;
            in.resize(cols);
            std::copy(std::begin(temp),
                      std::begin(temp) + min(cols, int(temp.size())),
                      std::begin(in));
        });
	}
}


void matrix::resizeCols(int newCols)
{
    std::for_each(data.begin(),
                  data.end(),
                  [newCols](std::valarray<double> & in)
    {
        /// not resizeValar from library
        std::valarray<double> temp = in;
        in.resize(newCols);
        std::copy(std::begin(temp),
                  std::begin(temp) + min(newCols, int(temp.size())),
                  std::begin(in));
    });

}

int matrix::rows() const
{
   return data.size();
}


double matrix::maxVal() const
{
	double res = 0.;
    std::for_each(this->data.begin(),
                  this->data.end(),
                  [&res](const std::valarray<double> & in)
    {
        res = max(res, in.max());
    });

    return res;
}
double matrix::minVal() const
{
	double res = data[0][0];
    std::for_each(this->data.begin(),
                  this->data.end(),
                  [&res](const std::valarray<double> & in)
    {
        res = min(res, in.max());
    });

    return res;
}
double matrix::sum() const
{
	double res = 0.;
    std::for_each(this->data.begin(),
                  this->data.end(),
                  [&res](const std::valarray<double> & in)
    {
       res += in.sum();
    });

    return res;
}


std::vector<std::valarray<double>>::iterator matrix::begin()
{
    return this->data.begin();
}

std::vector<std::valarray<double>>::iterator matrix::end()
{
    return this->data.end();
}

std::vector<std::valarray<double>>::const_iterator matrix::begin() const
{
    return this->data.begin();
}

std::vector<std::valarray<double>>::const_iterator matrix::end() const
{
    return this->data.end();
}

std::valarray<double> matrix::toVectorByRows() const
{
    std::valarray<double> res(this->rows() * this->cols());
    for(int i = 0; i < this->rows(); ++i)
    {
        std::copy(std::begin(this->data[i]),
                  std::end(this->data[i]),
                  std::begin(res) + this->cols() * i);
    }
    return res;
}

std::valarray<double> matrix::toVectorByCols() const
{
    std::valarray<double> res(this->rows() * this->cols());
    int count = 0;
    for(int i = 0; i < this->cols(); ++i)
    {
        for(int j = 0; j < this->rows(); ++j)
        {
            res[count++] = this->data[j][i];
        }
    }
    return res;
}

std::valarray<double> matrix::averageRow() const
{
    std::valarray<double> res(0., this->cols());
    for(int i = 0; i < this->rows(); ++i)
    {
        res += this->data[i];
    }
    res /= this->rows();
    return res;
}

std::valarray<double> matrix::averageCol() const
{
    std::valarray<double> res(this->rows());
    for(int i = 0; i < this->rows(); ++i)
    {
        res[i] = this->data[i].sum() / this->data[i].size();
    }
    return res;
}

std::valarray<double> matrix::getCol(int i, int numCols) const
{
    if(numCols < 0) numCols = this->rows();
    std::valarray<double> res(numCols);
    for(int j = 0; j < numCols; ++j)
    {
        res[j] = this->data[j][i];
    }
    return res;
}

void matrix::pop_back()
{
    this->data.pop_back();
}

void matrix::print(int rows, int cols) const
{
    if(rows == 0) rows = this->rows();
    if(cols == 0) cols = this->cols();

    for(int i = 0; i < rows; ++i)
    {
        for(int j = 0; j < cols; ++j)
        {
            cout << doubleRound(data[i][j], 3) << "\t";
        }
        cout << endl;
    }
    cout << endl;
}

void matrix::push_back(const std::valarray<double> & in)
{
    this->data.push_back(in);
}

void matrix::push_back(const std::vector<double> & in)
{
    std::valarray<double> temp(in.data(), in.size());
    this->data.push_back(temp);
}

int matrix::cols() const
{
    return data[0].size();
}

matrix matrix::transpose(const matrix &input)
{
    matrix res(input.cols(), input.rows());
    for(int i = 0; i < input.rows(); ++i)
    {
        for(int j = 0; j < input.cols(); ++j)
        {
            res[j][i] = input[i][j];
        }
    }
	return res;
}
void matrix::transpose()
{
    (*this) = matrix::transpose(*this);
}

void matrix::invert()
{
    if(this->rows() != this->cols())
    {
        cout << "matrix::invert: matrix is not square" << endl;
        return;
    }

    int size = this->rows();
    matrix initMat(size, size);
    initMat = (*this);

    matrix tempMat(size, size, 0.);
    for(int i = 0; i < size; ++i)
    {
        for(int j = 0; j < size; ++j)
        {
            tempMat[i][j] = (j==i);
        }
    }
    double coeff;

    //1) make higher-triangular
    for(int i = 0; i < size - 1; ++i) //which line to substract
    {
        for(int j = i + 1; j < size; ++j) //FROM which line to substract
        {
            coeff = initMat[j][i] / initMat[i][i]; // coefficient

            //row[j] -= coeff * row[i] for both (temp & init) matrices
            initMat[j] -= initMat[i] * coeff;
            tempMat[j] -= tempMat[i] * coeff;
        }
    }

    //2) make diagonal
    for(int i = size - 1; i > 0; --i) //which line to substract (bottom -> up)
    {
        for(int j = i - 1; j >= 0; --j) //FROM which line to substract
        {
            coeff = initMat[j][i] / initMat[i][i];

            //row[j] -= coeff * row[i] for both matrices
            initMat[j] -= initMat[i] * coeff;
            tempMat[j] -= tempMat[i] * coeff;
        }
    }

    //3) divide on diagonal elements
    for(int i = 0; i < size; ++i) //which line to divide
    {
        tempMat[i] /= initMat[i][i];
    }

    (*this) = tempMat;
}

void matrix::swapCols(int i, int j)
{
    for(int k = 0; k < this->rows(); ++k)
    {
        std::swap(this->data[k][i], this->data[k][j]);
    }
}
void matrix::swapRows(int i, int j)
{
    std::swap(this->data[i], this->data[j]);
}

void matrix::eraseRow(int i)
{
    if(i >= this->rows()) return;
    this->data.erase(data.begin() + i);
}


/// looks like okay
void matrix::eraseRows(const vector<int> & indices)
{
	eraseItems(this->data, indices);
}

