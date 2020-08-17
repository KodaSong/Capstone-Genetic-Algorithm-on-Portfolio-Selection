/*
 * @Author: Koda Song
 * @Date: 2020-08-02 11:38:20
 * @LastEditors: Koda Song
 * @LastEditTime: 2020-08-05 10:18:39
 * @Description: gnuplot.h
 */

#pragma once
#ifndef gnuplot_h
#define gnuplot_h

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void plotResults(double* xData, double* yData, double* yData2, int dataSize) {
	FILE* gnuplotPipe, * tempDataFile;
	const char* tempDataFileName1 = "Portfolio Return.txt";
	const char* tempDataFileName2 = "SPY Return.txt";
	double x1, y1, x2, y2;
	int i;
	gnuplotPipe = _popen("D:\\gnuplot\\bin\\gnuplot", "w");
	if (gnuplotPipe) {
		fprintf(gnuplotPipe, "plot \"%s\" using 1:2 title 'Portfolio' with lines linetype 4 smooth csplines, \"%s\" using 1:2 title 'SPY' with lines linetype 6 smooth csplines\n", tempDataFileName1, tempDataFileName2);
		fflush(gnuplotPipe);
		/*tempDataFile = fopen(tempDataFileName1, "w");*/
		tempDataFile = fopen("Portfolio Return.txt", "w");
		for (i = 0; i <= dataSize; i++) {
			x1 = xData[i];
			y1 = yData[i];
			fprintf(tempDataFile, "%lf %lf\n", x1, y1);
		}
		fclose(tempDataFile);

		//tempDataFile = fopen(tempDataFileName2, "w");
		tempDataFile = fopen("SPY Return.txt", "w");
		for (i = 0; i <= dataSize; i++) {
			x2 = xData[i];
			y2 = yData2[i];
			fprintf(tempDataFile, "%lf %lf\n", x2, y2);
		}
		fclose(tempDataFile);
		fprintf(gnuplotPipe, "exit \n");
	}
	else {
		printf("gnuplot not found...");
	}
}

#endif // !gnuplot_h

