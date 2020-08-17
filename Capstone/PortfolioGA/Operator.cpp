/*
 * @Author: Koda Song
 * @Date: 2020-06-23 18:55:56
 * @LastEditors: Koda Song
 * @LastEditTime: 2020-08-05 10:14:47
 * @Description: Operator.cpp
 */
#include "Operator.h"

using namespace std;

Vector_ operator+(const Vector_& V, const double v)
{
	Vector_ U(V.size());
	for (int i = 0; i < V.size(); i++)
		U[i] = V[i] + v;
	return U;
}

Vector_ operator-(const Vector_& V, const double v)
{
	Vector_ U(V.size());
	for (int i = 0; i < V.size(); i++)
		U[i] = V[i] - v;
	return U;
}

Vector_ operator-(const Vector_& V, const Vector_& W)
{
	int size = V.size();
	Vector_ U(size);
	for (int i = 0; i < size; i++)
	{
		U[i] = V[i] - W[i];
	}
	return U;
}

Vector_ operator/(const Vector_& V, const double n)
{
	int size = V.size();
	Vector_ U(size);
	for (int i = 0; i < size; i++)
	{
		U[i] = V[i] / n;
	}
	return U;
}

Vector_ sqrt(const Vector_& V)
{
	int d = V.size();
	Vector_ U(d);
	for (int j = 0; j < d; j++) 
		U[j] = sqrt(V[j]);
	return U;
}

Vector_ pow(const Vector_& V, const int N)
{
	int d = V.size();
	Vector_ U(d);
	for (int j = 0; j < d; j++)
		U[j] = pow(V[j], N);
	return U;
}

Vector_ operator+=(Vector_& V, const Vector_& W)
{
	int size = V.size();
	for (int i = 0; i < size; i++)
	{
		V[i] += W[i];
	}
	return V;
}

Vector_ operator/=(Vector_& V, const double n)
{
	int size = V.size();
	for (int i = 0; i < size; i++)
	{
		V[i] /= n;
	}
	return V;
}

Vector_ operator*(const Vector_& V, const Vector_& W)
{
	int size = V.size();
	Vector_ U(size);
	for (int i = 0; i < size; i++)
	{
		U[i] = V[i] * W[i];
	}
	return U;
}

// Return Value
// Vector (1 * N)	*	Vector (N * 1)
double dot(const Vector_& V, const Vector_& W)
{
	int size = V.size();
	double res = 0.0;
	for (int i = 0; i < size; i++)
	{
		res += V[i] * W[i];
	}
	return res;
}

// Return Vector (1 * M)
// Vector (1 * N)	*	Matrix (N * M)
Vector_ dot(const Vector_& V, const Matrix_& M)
{
	int rows = V.size();
	int cols = M[0].size();
	Vector_ U(cols, 0.0);

	for (int i = 0; i < cols; i++)
	{
		for (int j = 0; j < rows; j++)
			U[i] += V[j] * M[j][i];
	}
	return U;
}

// Return Vector (M * 1)
// Matrix (M * N) * Vector (N * 1)
Vector_ dot(const Matrix_& M, const Vector_& V)
{
	int size = V.size();
	Vector_ U(size);

	for (int i = 0; i < size; i++)
	{
		U[i] = dot(M[i], V);
	}
	return U;
}

// Return a Value
// Vector (1 * N) *	Matrix (N * N) * Vector (N * 1)
double dot(const Vector_& V, const Matrix_& M, const Vector_& W)
{
	//return dot(dot(V, M), W);
	return dot(V, dot(M, W));
}

double Avg(const Vector_& V, const int start, const int end)
{
	int size = end - start;
	double sum = 0.0;
	for (int i = start; i < end; i++)
	{
		sum += V[i];
	}
	return sum / size;
}

double Cov(const Vector_& V, const Vector_& W)
{
	int N = V.size();	
	Vector_ U(N);
	U = V * W;
	double avg1 = Avg(U, 0, N);
	double avg2 = Avg(V, 0, N);
	double avg3 = Avg(W, 0, N);
	
	return avg1 - avg2 * avg3;
}