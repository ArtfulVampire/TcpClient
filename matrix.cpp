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

matrix::matrix(const matrix & other)
{
    this->data = other.data;
}


matrix::matrix(const dataType & other)
{
    this->data = other;
}
matrix::matrix(const lineType & vect, bool orientH)
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
matrix::matrix(const lineType & vect, char orient)
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

#if CPP_11
matrix::matrix(const lineType & vect, int inRows)
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
#endif

matrix::matrix(const lineType & vect1, const lineType & vect2)
{
    this->data.clear();
    for(int i = 0; i < vect1.size(); ++i)
    {
        this->data.push_back(vect1[i] * vect2);
    }
}

#if CPP_11
matrix::matrix(std::initializer_list<lineType> lst)
{
    this->resize(0, 0);
    std::for_each(lst.begin(),
                  lst.end(),
                  [this](lineType in)
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
#endif





matrix matrix::operator = (const matrix & other)
{
    this->data = other.data;

    return *this;
}
matrix matrix::operator = (const dataType & other)
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
        lineType currCol = rhs.getCol(j);
        for(int i = 0; i < dim1; ++i)
        {
//            result[i][j] = std::inner_product(begin(lhs[i]),
//                                              end(lhs[i]),
//                                              begin(currCol),
//                                              0.);
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
#if CPP_11
    this->data.resize(newRows);
    std::for_each(data.begin(),
                  data.end(),
                  [newCols](lineType & in)
    {
        lineType temp = in;
        in.resize(newCols);
        std::copy(std::begin(temp),
                  std::begin(temp) + min(newCols, int(temp.size())),
                  std::begin(in));
    });
#else
    this->data.resize(newRows);
    if(newCols > this->cols()) // enlarge
    {
        for(std::vector<std::valarray<double> >::iterator it = data.begin(); it < data.end(); ++it)
        {
            lineType temp = (*it);
            (*it).resize(newCols);
            for(int i = 0; i < temp.size(); ++i)
            {
                (*it)[i] = temp[i];
            }
        }
    }
    else
    {
        for(std::vector<std::valarray<double> >::iterator it = data.begin(); it < data.end(); ++it)
        {
            (*it) = (*it)[std::slice(0, newCols, 1)];
        }
    }
#endif

}


void matrix::resizeRows(int newRows)
{
    int cols = this->cols();
    int oldRows = this->rows();
    data.resize(newRows);
#if CPP_11
    if(oldRows < newRows) /// why?
    {
        std::for_each(data.begin() + oldRows,
                      data.end(),
                      [cols](lineType & in)
        {
            lineType temp = in;
            in.resize(cols);
            std::copy(std::begin(temp),
                      std::begin(temp) + min(cols, int(temp.size())),
                      std::begin(in));
        });
    }
#else

#endif
}


void matrix::resizeCols(int newCols)
{
#if CPP_11
    std::for_each(data.begin(),
                  data.end(),
                  [newCols](lineType & in)
    {
        /// not resizeValar from library
        lineType temp = in;
        in.resize(newCols);
        std::copy(std::begin(temp),
                  std::begin(temp) + min(newCols, int(temp.size())),
                  std::begin(in));
    });
#else
    if(newCols > this->cols()) // enlarge
    {
        for(std::vector<std::valarray<double> >::iterator it = data.begin(); it < data.end(); ++it)
        {
            lineType temp = (*it);
            (*it).resize(newCols);
            for(int i = 0; i < temp.size(); ++i)
            {
                (*it)[i] = temp[i];
            }
        }
    }
    else
    {
        for(std::vector<std::valarray<double> >::iterator it = data.begin(); it < data.end(); ++it)
        {
            (*it) = (*it)[std::slice(0, newCols, 1)];
        }
    }
#endif
}

int matrix::rows() const
{
   return data.size();
}


double matrix::maxVal() const
{
    double res = 0.;
#if CPP_11
    std::for_each(this->data.begin(),
                  this->data.end(),
                  [&res](const lineType & in)
    {
        res = max(res, in.max());
    });
#else
    for(std::vector<std::valarray<double> >::const_iterator it = data.begin(); it != data.end(); ++it)
    {
        res = max(res, (*it).max());
    }
#endif
    return res;
}
double matrix::minVal() const
{
    double res = data[0][0];
#if CPP_11
    std::for_each(this->data.begin(),
                  this->data.end(),
                  [&res](const lineType & in)
    {
        res = min(res, in.max());
    });
#else
    for(std::vector<std::valarray<double> >::const_iterator it = data.begin(); it != data.end(); ++it)
    {
        res = min(res, (*it).min());
    }
#endif
    return res;
}
double matrix::sum() const
{
    double res = 0.;

#if CPP_11
    std::for_each(this->data.begin(),
                  this->data.end(),
                  [&res](const lineType & in)
    {
       res += in.sum();
    });
#else
    for(std::vector<std::valarray<double> >::const_iterator it = data.begin(); it != data.end(); ++it)
    {
        res += (*it).sum();
    }
#endif
    return res;
}


dataType::iterator matrix::begin()
{
    return this->data.begin();
}

dataType::iterator matrix::end()
{
    return this->data.end();
}

dataType::const_iterator matrix::begin() const
{
    return this->data.begin();
}

dataType::const_iterator matrix::end() const
{
    return this->data.end();
}

#if CPP_11
lineType matrix::toVectorByRows() const
{
    lineType res(this->rows() * this->cols());
    for(int i = 0; i < this->rows(); ++i)
    {
        std::copy(std::begin(this->data[i]),
                  std::end(this->data[i]),
                  std::begin(res) + this->cols() * i);
    }
    return res;
}

lineType matrix::toVectorByCols() const
{
    lineType res(this->rows() * this->cols());
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
#endif


lineType matrix::averageRow() const
{
    lineType res(0., this->cols());
    for(int i = 0; i < this->rows(); ++i)
    {
        res += this->data[i];
    }
    res /= this->rows();
    return res;
}

lineType matrix::averageCol() const
{
    lineType res(this->rows());
    for(int i = 0; i < this->rows(); ++i)
    {
        res[i] = this->data[i].sum() / this->data[i].size();
    }
    return res;
}

lineType matrix::getCol(int i, int numCols) const
{
    if(numCols < 0) numCols = this->rows();
    lineType res(numCols);
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

void matrix::push_back(const lineType & in)
{
    this->data.push_back(in);
}

void matrix::push_back(const vectType & in)
{
    lineType temp(in.data(), in.size());
    this->data.push_back(temp);
}

int matrix::cols() const
{
    return data[0].size();
}

matrix matrix::transpose(const matrix &input)
{
#if 1
    matrix res(input.cols(), input.rows());
    for(int i = 0; i < input.rows(); ++i)
    {
        for(int j = 0; j < input.cols(); ++j)
        {
            res[j][i] = input[i][j];
        }
    }
    return res;
#else
    matrix res;
    for(int i = 0; i < input.cols(); ++i)
    {
        res.push_back(input.getCol(i));
    }
    return res;
#endif
}
void matrix::transpose()
{
#if 1
    (*this) = matrix::transpose(*this);
#else
    int oldCols = this->cols();
    int oldRows = this->rows();
    this->resize(max(oldRows, oldCols),
                 max(oldRows, oldCols)); // make square

    for(int i = 0; i < this->rows(); ++i)
    {
        for(int j = i + 1; j < this->cols(); ++j)
        {
            std::swap(this->data[j][i], this->data[i][j]);
        }
    }
    this->resize(oldCols,
                 oldRows);
#endif
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

#if 1
    eraseItems(this->data, indices);
    return;
#else
    std::set<int, std::less<int> > excludeSet;
    for(auto item : indices)
    {
        excludeSet.emplace(item);
    }
    vector<int> excludeVector;
    for(auto a : excludeSet)
    {
        excludeVector.push_back(a);
    }
    excludeVector.push_back(this->data.size());
    for(int i = 0; i < excludeVector.size() - 1; ++i)
    {
        for(int j = excludeVector[i] - i; j < excludeVector[i + 1] - i - 1; ++j)
        {
            this->data[j] = this->data[j + 1 + i];
        }
    }
    this->data.resize(this->data.size() - excludeSet.size());
#endif
}

//template <typename matType1, typename matType2>
void matrixProduct(const matrix & in1,
                   const matrix & in2,
                   matrix & result,
                   int dim,
                   int rows1,
                   int cols2)
{
    int dim1 = 0;
    int dim2 = 0;
    int Size = 0;

    if(rows1 != -1)
    {
        dim1 = rows1;
    }
    else
    {
        dim1 = in1.rows();
    }

    if(cols2 != -1)
    {
        dim2 = cols2;
    }
    else
    {
        dim2 = in2.cols();
    }

    if(dim != -1)
    {
        Size = dim;
    }
    else if(in1.cols() != in2.rows())
    {
        cout << "matrixProduct: input matrices are not productable" << endl;
        result = matrix();
        return;
    }
    else
    {
        Size = in1.cols();
    }

//    result.resize(max(dim1, result.rows()),
//                  max(dim2, result.cols()));
    result.resize(dim1, dim2);


    lineType temp;
    for(int j = 0; j < dim2; ++j)
    {
        temp = in2.getCol(j, Size); // size for prod()
        for(int i = 0; i < dim1; ++i)
        {
            result[i][j] = prod(temp, in1[i]);
        }
    }
}

