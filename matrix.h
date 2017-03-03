#ifndef MATRIX_H
#define MATRIX_H


#include "lib.h"


class matrix : public std::vector<std::valarray<double>>
{
public:
    matrix();
    ~matrix();
    matrix(int dim);
    matrix(int rows, int cols);
    matrix(int rows, int cols, double value);

    matrix(const matrix & other);
	matrix(const std::vector<std::valarray<double>> & other);

    matrix(matrix && other)
        :data{other.data}
    {
		other.data = std::vector<std::valarray<double>>();
    }

    matrix(std::initializer_list<std::valarray<double>> lst);

	matrix(std::initializer_list<double> lst); // diagonal


    matrix(const std::valarray<double> & vect1, const std::valarray<double> & vect2);
    matrix(const std::valarray<double> & vect, bool orientH);
	matrix(const std::valarray<double> & vect, char orient);
	matrix(const std::valarray<double> & vect, int rows);


    void resizeRows(int rows);
    void resizeCols(int newCols);
    void fill(double value);
    void print(int rows = 0, int cols = 0) const;
    int cols() const;
    int rows() const;
    double maxVal() const;
    double minVal() const;
    double sum() const;
	std::vector<std::valarray<double>>::iterator begin();
	std::vector<std::valarray<double>>::iterator end();
	std::vector<std::valarray<double>>::const_iterator begin() const;
	std::vector<std::valarray<double>>::const_iterator end() const;

    std::valarray<double> toVectorByRows() const;
	std::valarray<double> toVectorByCols() const;

    std::valarray<double> getCol(int i, int numCols = -1) const;
    std::valarray<double> averageRow() const;
    std::valarray<double> averageCol() const;
    void pop_back();
    void push_back(const std::valarray<double> &in);
    void push_back(const std::vector<double> &in);

    // for compability with vector < vector<Type> >
    void clear() {this->data.clear();}
    int size() const {return data.size();}

    void resize(int rows, int cols, double val);
    void resize(int rows, int cols);
    void resize(int i) {data.resize(i);}


    std::valarray<double> & operator [](int i)
    {
        return data[i];

    }
    const std::valarray<double> & operator [](int i) const
    {
        return data[i];
    }

    matrix operator = (const matrix & other);
	matrix operator = (const std::vector<std::valarray<double>> & other);

    matrix operator += (const matrix & other);
    matrix operator += (const double & val);

    matrix operator -= (const matrix & other);
    matrix operator -= (const double & val);

    matrix operator *= (const matrix & other);
    matrix operator *= (const double & val);

    matrix operator /= (const double & other);

    //"static"
    static matrix transpose(const matrix & input);

    // "private"
    void transpose();
    void invert();
    void swapCols(int i, int j);
    void swapRows(int i, int j);
    void zero();
    void one();
    void eraseRow(int i);
	void eraseRows(const vector<int> & indices);

public:
	std::vector<std::valarray<double>> data = std::vector<std::valarray<double>>();

};

matrix operator + (const matrix & lhs, const matrix & rhs);
matrix operator + (const matrix & lhs, const double & val);
matrix operator / (const matrix & lhs, const double & val);
matrix operator * (const matrix & lhs, const matrix & rhs);
matrix operator * (const matrix & lhs, const double & val);
matrix operator - (const matrix & lhs, const matrix & rhs);
matrix operator - (const matrix & lhs, const double & val);

#endif // MATRIX_H
