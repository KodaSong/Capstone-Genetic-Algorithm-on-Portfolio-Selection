/*
 * @Author: Koda Song
 * @Date: 2020-06-23 18:55:56
 * @LastEditors: Koda Song
 * @LastEditTime: 2020-08-05 10:18:03
 * @Description: GeneticAlgo.h
 */ 
#pragma once
#ifndef GeneticAlgo
#define GeneticAlgo

#include <iostream>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <math.h>
#include <vector>
#include <algorithm>

#include "Stock.h"

#define CROSSOVER_RATE            0.5
#define MUTATION_RATE             0.03
#define POP_SIZE                  50           //must be an even number
#define PORT_SIZE				  11		// Nums of Stocks in a portfolio
#define MAX_ALLOWABLE_GENERATIONS   100
#define KEEP_SIZE			  30	// number of individuals need to be replaced
#define RANDOM_NUM      ((float)rand()/RAND_MAX)	// 0 <= num <= 1

using namespace std;

//------------------------------------Mutate---------------------------------------
//
//  Mutates a portfolio's stocks dependent on the MUTATION_RATE
//  Replace the stock with another one in same industry
//-------------------------------------------------------------------------------------
void Mutate(sqlite3* db, vector<string>& sectors, vector<string>& symbols)
{
	char* error = NULL;
	char** results = NULL;
	int rows, columns;

	for (int i = 0; i < sectors.size(); i++)
	{
		if (RANDOM_NUM > MUTATION_RATE)
			continue;
		string tmp = "SELECT symbol FROM SP500Clean WHERE sector = '" + sectors[i] + "';";
		const char* sql_select = tmp.c_str();
		sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);

		int num = ((float)rand() / (RAND_MAX + 1)) * rows + 1;	// Skip the header, 1 <= num <= rows
		symbols[i] = results[num];	// Mutation happens

		sqlite3_free_table(results);
	}
}


//--------------------------------Roulette-------------------------------------------
//
//  selects a portfolio from the population via roulette wheel selection
//------------------------------------------------------------------------------------
Portfolio Roulette(Population& Pop)
{
	//generate a random number between [0, POP_SIZE - 1]
	int index = ((float)rand() / (RAND_MAX + 1)) * KEEP_SIZE;

	return Pop.GetPopulation()[index];
}

//---------------------------------- Crossover ---------------------------------------
//
//  Recombine two portfolios based on certain point
//  After recombination, mutate
//  Replace portfolios with low fitness
//------------------------------------------------------------------------------------
void Crossover(Population &Pop)
{
	Population tmp = Pop;
	string period = tmp.GetPeriod();
	sqlite3* db = tmp.GetDB();
	double Rf = tmp.GetRf();
	vector<string> sectors = tmp.GetSectors();
	// Randomly Select 2 parents from biggest 30 fitness portfolios
	// Replace smallest 20 fitness portfolios
	for (int i = 0; i < 20; i+=2)
	{
		// Select Parents
		// TO DO! Try to avoid selecting 2 same portfolios!
		Portfolio port1 = Roulette(tmp);
		Portfolio port2 = Roulette(tmp);

		// Swap portfolios
		vector<string> tmp1 = port1.GetSymbols();
		vector<string> tmp2 = port2.GetSymbols();
		vector<string> tmp11(tmp1.begin(), tmp1.begin() + 5);	// First half of Portfolio 1
		vector<string> tmp12(tmp1.begin() + 5, tmp1.end());		// Second half of Portfolio 1
		vector<string> tmp21(tmp2.begin(), tmp2.begin() + 5);
		vector<string> tmp22(tmp2.begin() + 5, tmp2.end());
		tmp11.insert(tmp11.end(), tmp22.begin(), tmp22.end());
		tmp21.insert(tmp21.end(), tmp12.begin(), tmp12.end());
		
		// Mutate
		Mutate(db, sectors, tmp11);
		Mutate(db, sectors, tmp21);

		// Create new portfolio objects
		//cout << "11111111111111111" << endl;
		Portfolio child1(period, db, tmp11, Rf, Pop.GetSP500Obj());
		//cout << "22222222222222222" << endl;
		Portfolio child2(period, db, tmp21, Rf, Pop.GetSP500Obj());

		// Replace portfolios with low fitness
		tmp.GetPopulation()[i + 30] = child1;
		tmp.GetPopulation()[i + 31] = child2;
	}
	Pop = tmp;
	/*for (int j = 0; j < 50; j++)
		cout << Pop.get_population()[j].get_fit() << endl;*/
}

#endif // !GeneticAlgo
