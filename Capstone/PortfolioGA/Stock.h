/*
 * @Author: Koda Song
 * @Date: 2020-08-02 11:38:20
 * @LastEditors: Koda Song
 * @LastEditTime: 2020-08-05 10:17:17
 * @Description: Stock.h
 */

#pragma once
#ifndef Stock_h
#define Stock_h

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <thread>
#include "Operator.h"
#include "database.h"
using namespace std;

#define CROSSOVER_RATE            0.5
#define MUTATION_RATE             0.03
#define POP_SIZE                  50           //must be an even number
#define PORT_SIZE				  11		// Nums of Stocks in a portfolio
#define MAX_ALLOWABLE_GENERATIONS   100
#define KEEP_SIZE			  30		// number of individuals need to be replaced
#define RANDOM_NUM      ((float)rand()/RAND_MAX)	// 0 <= num <= 1

// Calculate any each two stocks' covariance which are in different sectors
// Update data into Covariance Table
void ProcessCovariance(sqlite3* db, string dateBT, string datePB);

// Calculate each stock's average return before periods of backtest or probation
// Update to SP500Clean Table
void ProcessAvgRet(sqlite3* db);

//// Calculate Max Drawdown
//double Calculate_MDD(vector<double>& a);

// Add a member "dailyret"
class Trade
{
private:
	string date;
	float open;
	float high;
	float low;
	float close;
	float adjusted_close;
	int64_t volume;
	float daily_ret;
public:
	Trade() : date(""), open(0.0), high(0.0), low(0.0), close(0.0), adjusted_close(0.0), volume(0), daily_ret(0.0) {}
	Trade(string date_, float open_, float high_, float low_, float close_, float adjusted_close_, int64_t volume_, float dailyret_) :
		date(date_), open(open_), high(high_), low(low_), close(close_), adjusted_close(adjusted_close_), volume(volume_), daily_ret(dailyret_)
	{}
	~Trade() {}
	friend ostream& operator << (ostream& out, const Trade& t)
	{
		out << "Date: " << t.date << " Open: " << t.open 
			<< " High: " << t.high << " Low: " << t.low 
			<< " Close: " << t.close << " Adjusted_Close: " << t.adjusted_close 
			<< " Volume: " << t.volume << " DailyRet: " << t.daily_ret << endl;
		return out;
	}

	string GetDate() { return date; }
	float GetOpen() { return open; }
	float GetHigh() { return high; }
	float GetLow() { return low; }
	float GetClose() { return close; }
	float GetAdjclose() { return adjusted_close; }
	int64_t GetVolume() { return volume; }
	float GetRet() { return daily_ret; }
};

class Stock
{
private:
	string period;	// "BT" or "PB"	-> backtest/probation
	string symbol;
	vector<Trade> trades;	// each Trade include dailyret

	double mktcap;
	double div_yield;
	double beta;
	double high52;
	double low52;
	double MA50;
	double MA200;
	double EPS;
	double short_ratio;	// Shorted Shares / Daily Trading Volume
	//double Rf;	//	Risk Free Rate at the first of testing period	-> Add to Portfolio/Population

public:
	Stock() : period(""), symbol(""), mktcap(0.0), div_yield(0.0), beta(0.0), 
		high52(0.0), low52(0.0), MA50(0.0), MA200(0.0), EPS(0.0), short_ratio(0.0)
	{}
	// Directly Extract Data From dB
	Stock(string period_, string symbol_, sqlite3 *db) : period(period_), symbol(symbol_)
	{
		char* error = NULL;
		char** results = NULL;
		int rows, columns;
		char sql_select[512];

		//// Extract Fundamental Data
		//sprintf(sql_select, "SELECT * FROM Fundamental WHERE symbol = \"%s\"", symbol.c_str());
		//sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
		//div_yield = stod(results[columns + 1]);
		//beta = stod(results[columns + 2]);
		//High52 = stod(results[columns + 3]);
		//Low52 = stod(results[columns + 4]);
		//MA50 = stod(results[columns + 5]);
		//MA200 = stod(results[columns + 6]);
		//mktcap = stod(results[columns + 7]);
		//EPS = stod(results[columns + 8]);
		//short_ratio = stod(results[columns + 9]);
		//sqlite3_free_table(results);

		//// Extract Prices Data
		//if (period == "BT")
		//{
		//	string period1 = "2019-01-01", period2 = "2020-01-01";
		//	sprintf(sql_select, "SELECT * FROM Prices WHERE symbol = \"%s\"\
		//						 AND date >= \"%s\" AND date < \"%s\";", symbol.c_str(), period1.c_str(), period2.c_str());
		//}
		//else if (period == "PB")
		//{
		//	string period1 = "2020-01-01";
		//	sprintf(sql_select, "SELECT * FROM Prices WHERE symbol = \"%s\"\
		//						 AND date >= \"%s\";", symbol.c_str(), period1.c_str());
		//}
		//sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
		//for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
		//{
		//	int cellPosition = rowCtr * columns;
		//	string date = results[cellPosition + 1];	// Skip column "symbol"
		//	double open = stod(results[cellPosition + 2]);
		//	double high = stod(results[cellPosition + 3]);
		//	double low = stod(results[cellPosition + 4]);
		//	double close = stod(results[cellPosition + 5]);
		//	double adjclose = stod(results[cellPosition + 6]);
		//	int64_t volume = stoi(results[cellPosition + 7]);
		//	double dailyret = stod(results[cellPosition + 8]);
		//	Trade trade_obj(date, open, high, low, close, adjclose, volume, dailyret);
		//	trades.push_back(trade_obj);
		//}
		//sqlite3_free_table(results);
	}
	~Stock() {}
	friend ostream& operator << (ostream& out, const Stock& s)
	{
		out << "Symbol: " << s.symbol << endl;
		for (vector<Trade>::const_iterator itr = s.trades.begin(); itr != s.trades.end(); itr++)
			out << *itr;
		return out;
	}

	string GetSymbol() { return symbol; }
	int64_t GetMktcap() { return mktcap; }
	double GetDivYield() { return div_yield; }
	double GetBeta() { return beta; }
	double GetHigh52() { return high52; }
	double GetLow52() { return low52; }
	double GetMA50() { return MA50; }
	double GetMA200() { return MA200; }
	double GetEPS() { return EPS; }
	double GetShortRatio() { return short_ratio; }
	vector<Trade> GetTrades() { return trades; }
	/*Stock(string symbol_, int64_t mktcap_, double div_yield_, double beta_,
		double High52_, double Low52_, double MA50_,
		double MA200_, double EPS_, double short_ratio_) :symbol(symbol_), mktcap(mktcap_),
		div_yield(div_yield_), beta(beta_), High52(High52_), Low52(Low52_),
		MA50(MA50_), MA200(MA200_), EPS(EPS_), short_ratio(short_ratio_)
	{}*/
	/*void ExtractData(sqlite3* db, string Period)
	{
		char* error = NULL;
		char** results = NULL;
		int rows, columns;
		char sql_select[512];
		// Extract Fundamental Data
		sprintf(sql_select, "SELECT * FROM Fundamental WHERE symbol = \"%s\"", symbol.c_str());
		sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
		div_yield = stod(results[columns + 1]);
		beta = stod(results[columns + 2]);
		High52 = stod(results[columns + 3]);
		Low52 = stod(results[columns + 4]);
		MA50 = stod(results[columns + 5]);
		MA200 = stod(results[columns + 6]);
		mktcap = stoi(results[columns + 7]);
		EPS = stod(results[columns + 8]);
		short_ratio = stod(results[columns + 9]);
		sqlite3_free_table(results);

		// Extract Prices Data
		if (Period == "BT")
		{
			string period1 = "2019-01-01", period2 = "2020-01-01";
			sprintf(sql_select, "SELECT * FROM Prices WHERE symbol = \"%s\"\
								 AND date >= \"%s\" AND date < \"%s\";", symbol.c_str(), period1.c_str(), period2.c_str());
		}
		if (Period == "PB")
		{
			string period1 = "2020-01-01";
			sprintf(sql_select, "SELECT * FROM Prices WHERE symbol = \"%s\"\
								 AND date >= \"%s\";", symbol.c_str(), period1.c_str());
		}
		sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
		for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
		{
			int cellPosition = rowCtr * columns;
			string date = results[cellPosition + 1];	// Skip column "symbol"
			double open = stod(results[cellPosition + 2]);	
			double high = stod(results[cellPosition + 3]);	
			double low = stod(results[cellPosition + 4]);	
			double close = stod(results[cellPosition + 5]);	
			double adjclose = stod(results[cellPosition + 6]);	
			int64_t volume = stoi(results[cellPosition + 7]);
			double dailyret = stod(results[cellPosition + 8]);
			Trade trade_obj(date, open, high, low, close, adjclose, volume, dailyret);
			trades.push_back(trade_obj);
		}
		sqlite3_free_table(results);
	}*/
	/*void addTrade(Trade aTrade)
	{
		trades.push_back(aTrade);
	}*/
};

class SPY : public Stock
{
private:
	string period;
	string symbol = "SPY";
	vector<Trade> trades;
public:
	SPY() : period("") {}
	SPY(string period_, sqlite3 *db)
	{
		char* error = NULL;
		char** results = NULL;
		int rows, columns;
		char sql_select[512];
		// Extract Prices Data
		if (period == "BT")
		{
			/*string period1 = "2019-01-01", period2 = "2020-01-01";*/
			string period1 = "2020-01-01", period2 = "2020-07-01";
			sprintf(sql_select, "SELECT * FROM Prices WHERE symbol = \"%s\"\
								 AND date >= \"%s\" AND date < \"%s\";", symbol.c_str(), period1.c_str(), period2.c_str());
		}
		else if (period == "PB")
		{
			/*string period1 = "2020-01-01";*/
			string period1 = "2020-07-01";
			sprintf(sql_select, "SELECT * FROM Prices WHERE symbol = \"%s\"\
								 AND date >= \"%s\";", symbol.c_str(), period1.c_str());
		}
		sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
		for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
		{
			int cellPosition = rowCtr * columns;
			string date = results[cellPosition + 1];	// Skip column "symbol"
			double open = stod(results[cellPosition + 2]);
			double high = stod(results[cellPosition + 3]);
			double low = stod(results[cellPosition + 4]);
			double close = stod(results[cellPosition + 5]);
			double adjclose = stod(results[cellPosition + 6]);
			int64_t volume = stoi(results[cellPosition + 7]);
			double dailyret = stod(results[cellPosition + 8]);
			Trade trade_obj(date, open, high, low, close, adjclose, volume, dailyret);
			trades.push_back(trade_obj);
		}
		sqlite3_free_table(results);
	}
	~SPY() {}
};

// Store data from online and insert into Fundamental Prices
class Fundamental
{
private:
	string symbol;
	int64_t mktcap;
	//double PE;
	double div_yield;
	double beta;
	double high52;
	double low52;
	double MA50;
	double MA200;
	double EPS;
	double short_ratio;	// Shorted Shares / Daily Trading Volume
public:
	Fundamental() : symbol(""), div_yield(0.0), beta(0.0), high52(0.0), low52(0.0),
		MA50(0.0), MA200(0.0), mktcap(0), EPS(0.0), short_ratio(0.0) {}
	Fundamental(string symbol_, double div_yield_, double beta_, double High52_,
		double Low52_, double MA50_, double MA200_, int64_t mktcap_, double EPS_, double short_ratio_) :
		symbol(symbol_), div_yield(div_yield_), beta(beta_), high52(High52_), low52(Low52_),
		MA50(MA50_), MA200(MA200_), mktcap(mktcap_), EPS(EPS_), short_ratio(short_ratio_) {}
	~Fundamental() {}

	string GetSymbol() { return symbol; }
	int64_t GetMktcap() { return mktcap; }
	double GetDivYield() { return div_yield; }
	double GetBeta() { return beta; }
	double GetHigh52() { return high52; }
	double GetLow52() { return low52; }
	double GetMA50() { return MA50; }
	double GetMA200() { return MA200; }
	double GetEPS() { return EPS; }
	double GetShortRatio() { return short_ratio; }

	friend ostream& operator << (ostream& out, const Fundamental& t)
	{
		out << "Symbol: " << t.symbol << " Dividend Yield: " << t.div_yield
			<< " Beta: " << t.beta << " High52: " << t.high52 << " Low52: " << t.low52
			<< " MA50: " << t.MA50 << " MA200: " << t.MA200 << " EPS: " << t.EPS
			<< " Short Ratio: " << t.short_ratio << endl;
		return out;
	}
};

class SP500Data
{
public:
	vector<string> symbols;
	vector<double> avgretbt, avgretpb;
	vector<int> shortratiorank;

	SP500Data() {}
	SP500Data(sqlite3* db)
	{
		char* error = NULL;
		char** results = NULL;
		int rows, columns;
		string tmp = "SELECT SP500Clean.symbol, SP500Clean.AvgRetBT, SP500Clean.AvgRetPB, Fundamental.ShortRatioRank\
					FROM SP500Clean, Fundamental\
					WHERE SP500Clean.symbol = Fundamental.symbol;";
		sqlite3_get_table(db, tmp.c_str(), &results, &rows, &columns, &error);
		for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
		{
			// Determine Cell Position
			int cellposition = rowCtr * columns;
			symbols.push_back(results[cellposition]);
			avgretbt.push_back(stod(results[cellposition + 1]));
			avgretpb.push_back(stod(results[cellposition + 2]));
			shortratiorank.push_back(stoi(results[cellposition + 3]));
		}
	}
	~SP500Data() {}
};


class SummaryStats
{
private:
	int dayscount;
	double alpha;
	double beta;
	double vol;		// Daily Volatility
	double VaR;
	double ES;	// Expected Shortfall
	double TotalMktRet;
	double TotalRet;
	double MaxDrawDown;
	double fitness;	// Portfolio's SharpeRatio
	double InformationRatio;
public:
	SummaryStats() {}
	/*SummaryStats(double alpha_, double beta_, double vol_, double IR, double VaR_, double MDD, double SR) :
		alpha(alpha_), beta(beta_), vol(vol_), InformationRatio(IR), VaR(VaR_), MaxDrawDown(MDD), SharpeRatio(SR) {}*/
	~SummaryStats() {}

	friend ostream& operator << (ostream& out, const SummaryStats& t)
	{
		out << setiosflags(ios::fixed) << setprecision(2);
		out << "Days: " << t.dayscount << endl
			<< "fitness(Sharpe Ratio): " << t.fitness << endl
			<< "Total Return: " << t.TotalRet * 100 << "%" << endl
			<< "Total Benchmark Return: " << t.TotalMktRet * 100 << "%" << endl
			<< "Beta: " << t.beta << endl
			<< "Yearly Alpha: " << t.alpha << endl
			<< "Yearly Volatility: " << t.vol * pow(252, 0.5) * 100 << "%" << endl
			<< "Max Drawdown: " << t.MaxDrawDown * 100 << "%" << endl
			<< "Information Ratio: " << t.InformationRatio << endl
			<< "95% Daily VaR: " << t.VaR * 100 << "%" << endl
			<< "95% Expected Shortfall: " << t.ES * 100 << "%" << endl;
		return out;
	}

	void SetDaysCount(int dayscount_) { dayscount = dayscount_; }
	void SetAlpha(double alpha_) { alpha = alpha_; }
	void SetBeta(double beta_) { beta = beta_; }
	void SetVol(double vol_) { vol = vol_; }
	void SetVaR(double VaR_) { VaR = VaR_; }
	void SetES(double ES_) { ES = ES_; }
	void SetTotalMarketRet(double MarketRet_) { TotalMktRet = MarketRet_; }
	void SetTotalRet(double TotalRet_) { TotalRet = TotalRet_; }
	void SetMDD(double MDD) { MaxDrawDown = MDD; }
	void SetFitness(double SharpeRatio_) { fitness = SharpeRatio_; }
	void SetIR(double InformationRatio_) { InformationRatio = InformationRatio_; }

	int GetDaysCount() { return dayscount; }
	double GetAlpha() { return alpha; }
	double GetBeta() { return beta; }
	double GetVol() { return vol; }
	double GetVaR() { return VaR; }
	double GetES() { return ES; }
	double GetTotalMktRet() { return TotalMktRet; }
	double GetTotalRet() { return TotalRet; }
	double GetMDD() { return MaxDrawDown; }
	double GetFitness() { return fitness; }
	double GetIR() { return InformationRatio; }
};

class Portfolio
{
private:
	double fitness;
	double exp_ret;		// equals to weight.T * stocks_avg_ret
	double rf;

	string period;
	sqlite3* db;
	vector<string> symbols;	// 11 stocks in different sectors
	vector<string> sectors;	// 11 sectors
	vector<Stock> stocks;
	vector<double> optimal_weight;
	vector<double> stock_avg_ret;	// Store each stocks' average return in historical data

	vector<double> daily_ret;	// Store portfolio's daily return during period
	vector<double> cumulative_ret;	// Store portfolio's daily cumulative return during period

	vector<double> spy_dailyret;
	vector<double> spy_cumulative_ret;

	vector<int> buy_or_sell;	// < 8 means buy; >= 8 means sell

public:
	SP500Data sp500obj;
	SummaryStats summary;

	Portfolio() : fitness(0.0), exp_ret(0.0), rf(0.0), db(NULL) {}
	/*Portfolio(double fitness_, double Expected_Return_, double Vol_, double Rf_, double SR_) :
		fitness(fitness_), Expected_Return(Expected_Return_), Vol(Vol_), Rf(Rf_), SR(SR_)
	{}*/
	Portfolio(string period_, sqlite3* db_, vector<string> symbols_, double Rf_, SP500Data SP_Obj_) : period(period_), fitness(0.0), exp_ret(0.0),
		rf(Rf_), db(db_), symbols(symbols_), sp500obj(SP_Obj_)
	{
		stock_avg_ret.resize(11);
		buy_or_sell.resize(11);
		for (int i = 0; i < symbols.size(); i++)
		{
			Stock mystock(period, symbols[i], db);
			stocks.push_back(mystock);

			char* error = NULL;
			char** results = NULL;
			int rows, columns;
			char sql_select[512];
			/*sprintf(sql_select, "SELECT ShortRatioRank FROM Fundamental WHERE symbol = \"%s\";", symbols[i].c_str());
			sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
			buy_or_sell.push_back(stoi(results[1]));
			sqlite3_free_table(results);*/

			// Extract Expected Return & ShortRatioRank
			vector<string>::iterator position = find(sp500obj.symbols.begin(), sp500obj.symbols.end(), symbols[i]);
			int dist = distance(std::begin(sp500obj.symbols), position);
			//cout << symbols[i] << " " << dist << endl;
			if (period == "BT")
				stock_avg_ret[i] = sp500obj.avgretbt[dist];
			else
				stock_avg_ret[i] = sp500obj.avgretpb[dist];
			buy_or_sell[i] = sp500obj.shortratiorank[dist];
		}
	}
	~Portfolio() {}

	friend ostream& operator << (ostream& out, const Portfolio& t)
	{
		out << setiosflags(ios::fixed) << setprecision(3);
		out << setw(4) << left << "No."
			<< setw(8) << left << "Symbol"
			<< setw(8) << left << "Weight"
			<< setw(30) << left << "Sector" << endl;
		for (int i = 0; i < 11; i++)
		{
			out << setw(4) << left << i + 1
				<< setw(8) << left << t.symbols[i]
				<< setw(8) << left << t.optimal_weight[i]
				<< setw(30) << left << t.sectors[i] << endl;
		}
		return out;
	}

	void SetRf(double Rf_) { rf = Rf_; }
	void SetSectors(vector<string> sectors_) { sectors = sectors_; }
	void SetSPY();	// Extract SPY Daily Return from dB 

	double GetFitness() { return fitness; }
	double GetExpectedRet() { return exp_ret; }
	sqlite3* GetDB() { return db; }
	vector<string> GetSymbols() { return symbols; }
	vector<Stock> GetStocks() { return stocks; }
	vector<double> GetWeight() { return optimal_weight; }
	vector<double> GetStockAvgRet() { return stock_avg_ret; }

	vector<double> GetDailyRet() { return daily_ret; }
	vector<double> GetCumuRet() { return cumulative_ret; }
	vector<double> GetSPYDaily() { return spy_dailyret; }
	vector<double> GetSPYCumu() { return spy_cumulative_ret; }

	void CalculateDailyRet();
	void CalculateCumuRetAndMdd();
	void EfficientFrontier();
	void CalculateSummaryStats();
};



// Only 1 Rf will used for all portfolios
class Population
{
private:
	string period;
	double rf;
	vector<Portfolio> element_ports;	// 50 portfolios
	vector<string> sectors;
	sqlite3* db;
	vector< vector<string> > symbols;	// To store symbols in each Portfolio
	/*vector<double> spy_dailyret;
	vector<double> spy_cumulative_ret;*/
	SP500Data sp500obj;

public:
	Population() : period(""), rf(0.0), db(NULL) {}
	Population(string period_, sqlite3 *db_, SP500Data SP_Obj_) : period(period_), db(db_), sp500obj(SP_Obj_)
	{
		char* error = NULL;
		char** results = NULL;
		int rows, columns;
		char sql_select[512];
		// Extract Rf
		string period;
		if (period == "BT")
		{
			// period = "2019-01-01";
			period = "2020-01-01";
		}
		else if (period == "PB")
		{
			// period = "2020-01-01";
			period = "2020-07-01";
		}
			
		sprintf(sql_select, "SELECT AdjClose FROM RiskFree WHERE date >= \"%s\";", period.c_str());
		sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
		rf = stod(results[1]) / 100.0;
		sqlite3_free_table(results);

		string tmp = "SELECT DISTINCT sector FROM SP500;";
		sqlite3_get_table(db, tmp.c_str(), &results, &rows, &columns, &error);
		for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
		{
			// Determine Cell Position
			sectors.push_back(results[rowCtr]);
		}
		sqlite3_free_table(results);
	}
	~Population() {}

	double GetRf() { return rf; }
	string GetPeriod() { return period; }
	sqlite3* GetDB() { return db; }
	/*vector<double> GetSPYDaily() { return spy_dailyret; }
	vector<double> GetSPYCumu() { return spy_cumulative_ret; }*/
	vector<string> GetSectors() { return sectors; }
	vector<Portfolio> GetPopulation() { return element_ports; }
	SP500Data GetSP500Obj() { return sp500obj; }

	void SetElementPorts(Portfolio& port_, int index) { element_ports[index] = port_; }

	void ConstructPopulation();	// Construct 50 portfolios
	void SortFitness(string order);	// Sort portfolios by fitness
};




void CalculateFitness(Population& Pop, int start_index, int end_index);

void MultiCalculateFitness(Population& Pop, int begin, int over);

// Add a data member "symbol"
// Used for store data into Prices Table
class Trade_New
{
private:
	string symbol;
	string date;
	float open;
	float high;
	float low;
	float close;
	float adjusted_close;
	long volume;
public:
	Trade_New() : symbol(""), date(""), open(0.0), high(0.0), low(0.0), close(0.0), adjusted_close(0.0), volume(0) {}
	Trade_New(string symbol_, string date_, float open_, float high_, float low_, float close_, float adjusted_close_, long volume_) :
		symbol(symbol_), date(date_), open(open_), high(high_), low(low_), close(close_), adjusted_close(adjusted_close_), volume(volume_)
	{}

	string get_symbol() { return symbol; }
	string get_date() { return date; }
	float get_open() { return open; }
	float get_high() { return high; }
	float get_low() { return low; }
	float get_close() { return close; }
	float get_adjclose() { return adjusted_close; }
	long get_volume() { return volume; }

	~Trade_New() {}
	friend ostream& operator << (ostream& out, const Trade_New& t)
	{
		out << "Symbol: " << t.symbol << " Date: " << t.date << " Open: " << t.open << " High: " << t.high << " Low: " << t.low << " Close: " << t.close << " Adjusted_Close: " << t.adjusted_close << " Volume: " << t.volume << endl;
		return out;
	}
};

//---------------------------------- No use anymore ---------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------------------------------------
void BacktestCovariance(sqlite3* db);
void ProbationCovariance(sqlite3* db);


#endif // !Stock_h
