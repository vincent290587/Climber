/*
 * sine_fitter.cpp
 *
 *  Created on: 27 mars 2019
 *      Author: Vincent
 */

#include "sine_fitter.h"
#include "math_wrapper.h"
#include "segger_wrapper.h"

static float _fitter_compute(float *datax, float *datay, float *dataz,
		uint16_t numOfData, float *res) {

	float p[3] = {0}; // product of fitting equation
	float XiYi(0);
	float XiZi(0);
	float YiZi(0);
	float XiXi(0);
	float YiYi(0);
	float Xi(0);
	float Yi(0);
	float Zi(0);

	for(int i=0; i < numOfData; i++)
	{
		XiYi += datax[i] * datay[i];
		XiZi += datax[i] * dataz[i];
		YiZi += datay[i] * dataz[i];
		XiXi += datax[i] * datax[i];
		YiYi += datay[i] * datay[i];
		Xi   += datax[i];
		Yi   += datay[i];
		Zi   += dataz[i];
	}

	float A[3][3];
	float B[3][3];
	float C[3][3];
	float X[3][3];
	int i;
	int j;
	float x = 0;
	float n = 0; //n is the determinant of A

	A[0][0] = XiXi;
	A[0][1] = XiYi;
	A[0][2] = Xi;
	A[1][0] = XiYi;
	A[1][1] = YiYi;
	A[1][2] = Yi;
	A[2][0] = Xi;
	A[2][1] = Yi;
	A[2][2] = numOfData;

	for( i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{
			B[i][j] = 0;
			C[i][j] = 0;
		}
	}

	for( i=0, j=0;j<3;j++)
	{
		if(j == 2)
		{
			n += A[i][j] * A[i+1][0] * A[i+2][1];
		}
		else if(j == 1)
		{
			n += A[i][j] * A[i+1][j+1] * A[i+2][0];
		}
		else
		{
			n += A[i][j] * A[i+1][j+1] * A[i+2][j+2];
		}
	}

	for( i=2, j=0;j<3;j++)
	{
		if(j == 2)
		{
			n -= A[i][j] * A[i-1][0] * A[i-2][1];
		}
		else if(j == 1)
		{
			n -= A[i][j] * A[i-1][j+1] * A[i-2][0];
		}
		else
		{
			n -= A[i][j]*A[i-1][j+1]*A[i-2][j+2];
		}
	}

	// Check determinant n of matrix A
	if (n)
	{
		x = 1.0/n;

		for(i=0;i<3;i++)
		{
			for(j=0;j<3;j++)
			{
				B[i][j] = A[j][i];
			}
		}

		C[0][0] =  (B[1][1] * B[2][2]) - (B[2][1] * B[1][2]);
		C[0][1] = ((B[1][0] * B[2][2]) - (B[2][0] * B[1][2])) * (-1);
		C[0][2] =  (B[1][0] * B[2][1]) - (B[2][0] * B[1][1]);

		C[1][0] = ((B[0][1] * B[2][2]) - (B[2][1] * B[0][2])) * (-1);
		C[1][1] =  (B[0][0] * B[2][2]) - (B[2][0] * B[0][2]);
		C[1][2] = ((B[0][0] * B[2][1]) - (B[2][0] * B[0][1])) * (-1);

		C[2][0] =  (B[0][1] * B[1][2]) - (B[1][1] * B[0][2]);
		C[2][1] = ((B[0][0] * B[1][2]) - (B[1][0] * B[0][2])) * (-1);
		C[2][2] =  (B[0][0] * B[1][1]) - (B[1][0] * B[0][1]);

		for( i=0;i<3;i++)
		{
			for( j=0;j<3;j++)
			{
				X[i][j] = C[i][j] * x;
			}
		}

		p[0] = XiZi;
		p[1] = YiZi;
		p[2] = Zi;

		res[0] = X[0][0] * p[0] + X[0][1] * p[1] + X[0][2] * p[2];
		res[1] = X[1][0] * p[0] + X[1][1] * p[1] + X[1][2] * p[2];
		res[2] = X[2][0] * p[0] + X[2][1] * p[1] + X[2][2] * p[2];

	}
	else  // determinant=0
	{
		res[0] = 1;
		res[1] = 1;
		res[2] = 0;
	}

	return n;
}

/**
 *
 * @param dataz Input data
 * @param omega In rad/s
 * @param sampling In s
 * @param numOfData Length of input data array
 * @param output Pointer to ouput struct
 */
void sine_fitter_compute(float *dataz, float omega, float sampling,
		uint16_t numOfData, sSineFitterOuput *output) {

	float res[3];
	float datax[numOfData];
	float datay[numOfData];

	for(int i=0;i < numOfData;i++)
	{
	  datax[i] = cosf(omega * sampling * i);
	  datay[i] = sinf(omega * sampling * i);
	}

	if (_fitter_compute(datax, datay, dataz,
			numOfData, res)) {

		output->alpha = res[2];
		output->beta  = my_sqrtf(res[1]*res[1]+res[0]*res[0]);
		output->phi   = atan2f(res[0], res[1]);

	} else {
		LOG_ERROR("NULL determinant");
	}
}
