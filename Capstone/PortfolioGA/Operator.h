/*
 * @Author: Koda Song
 * @Date: 2020-06-23 18:55:56
 * @LastEditors: Koda Song
 * @LastEditTime: 2020-08-05 10:13:32
 * @Description: Operator.h
 */ 
#pragma once
#ifndef Operator_h
#define Operator_h

#include <iostream>
#include <vector>
using namespace std;

// Since Vector and Matrix are already used in "ExcelDriver"
// We use "Vector_" and "Matrix_" to rename vector<double> and vector< vector<double> >
typedef vector<double> Vector_;
typedef vector<Vector_> Matrix_;

Vector_ operator+(const Vector_& V, const double v);

Vector_ operator-(const Vector_& V, const double v);

Vector_ operator-(const Vector_& V, const Vector_& W);

Vector_ operator/(const Vector_& V, const double n);

Vector_ operator+=(Vector_& V, const Vector_& W);

Vector_ operator/=(Vector_& V, const double n);

Vector_ sqrt(const Vector_& V);  //calculate square root of a vector

Vector_ pow(const Vector_& V, const int N);  //calculate square of a vector

Vector_ operator*(const Vector_& V, const Vector_& W);

double dot(const Vector_& V, const Vector_& W);

Vector_ dot(const Vector_& V, const Matrix_& M);

Vector_ dot(const Matrix_& M, const Vector_& V);

double dot(const Vector_& V, const Matrix_& M, const Vector_& W);

double Avg(const Vector_& V, const int start, const int end);

// Calculate two stocks' covariance
double Cov(const Vector_& V, const Vector_& W);


#endif // !Operator_h

