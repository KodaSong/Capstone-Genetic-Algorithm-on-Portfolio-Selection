/*
Main.cpp
This file is used to test whole program
For build, debug
*/

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <stdio.h>

#include "Table.h"
#include "GeneticAlgo.h"
#include "gnuplot.h"

using namespace std;


int main(void)
{
	srand(time(NULL));
	// We import "SP500.csv" & "RiskFree.csv" into dB by DB Browser
	const char* db_name = "test.db";
	sqlite3* db = NULL;
	if (OpenDatabase(db_name, db) == -1)
		return -1;

	system("pause");

	// First generation
	SP500Data sp500obj(db);

	Population Pop("BT", db, sp500obj);
	Pop.ConstructPopulation();

	MultiCalculateFitness(Pop, 0, POP_SIZE);
	Pop.SortFitness("D");

	// MAX_ALLOWABLE_GENERATIONS
	for (int i = 1; i < 100; i++)
	{
		cout << i << endl;
		Crossover(Pop);
		MultiCalculateFitness(Pop, KEEP_SIZE, POP_SIZE);
		Pop.SortFitness("D");
	}
	cout << endl;
	
	Portfolio Best = Pop.GetPopulation()[0];
	Best.SetSPY();	// Extract SPY Data from DB
	Best.SetSectors(Pop.GetSectors());
	Best.CalculateDailyRet();
	Best.CalculateCumuRetAndMdd();
	Best.CalculateSummaryStats();
	cout << Best << endl;	// Show stocks and weight
	cout << Best.summary << endl;	// Show portfolio performance

	//int nIntervals = Pop.GetSPYCumu().size() - 1;
	int nIntervals = Best.GetSPYCumu().size() - 1;
	double* xData = (double*)malloc((nIntervals + 1) * sizeof(double));
	double* yData1 = (double*)malloc((nIntervals + 1) * sizeof(double));
	double* yData2 = (double*)malloc((nIntervals + 1) * sizeof(double));

	xData[0] = 0.0;
	for (int i = 0; i <= nIntervals - 1; i++) {
		xData[i + 1] = xData[i] + 1.0;
	}

	for (int i = 0; i <= nIntervals; i++) {
		yData1[i] = Best.GetCumuRet()[i];	// Percentage
		yData2[i] = Best.GetSPYCumu()[i];
	}
	plotResults(xData, yData1, yData2, nIntervals);

 	CloseDatabase(db);
	return 0;
}
