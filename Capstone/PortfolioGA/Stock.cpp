/*
 * @Author: Koda Song
 * @Date: 2020-08-02 11:38:20
 * @LastEditors: Koda Song
 * @LastEditTime: 2020-08-05 10:17:41
 * @Description: Stock.cpp
 */

#include "Stock.h"

bool descending(Portfolio& a, Portfolio& b)
{
	return a.GetFitness() > b.GetFitness();
}

bool ascending(Portfolio& a, Portfolio& b)
{
	return a.GetFitness() > b.GetFitness();
}

void ProcessCovariance(sqlite3* db, string dateBT, string datePB)
{
	char* error = NULL;
	char** results = NULL;
	int rows, columns;

	vector<string> symbol_vec;
	vector<string> sector_vec;
	vector<double> varbt_vec, varpb_vec;
	vector<double> avgretbt_vec, avgretpb_vec;
	string tmp = "SELECT symbol, sector, VarBT, VarPB, AvgRetBT, AvgRetPB FROM SP500Clean;";
	sqlite3_get_table(db, tmp.c_str(), &results, &rows, &columns, &error);

	for (int rowCtr = 1; rowCtr <= rows; rowCtr++)
	{
		symbol_vec.push_back(results[rowCtr * columns]);
		sector_vec.push_back(results[rowCtr * columns + 1]);
		varbt_vec.push_back(stod(results[rowCtr * columns + 2]));
		varpb_vec.push_back(stod(results[rowCtr * columns + 3]));
		avgretbt_vec.push_back(stod(results[rowCtr * columns + 4]));
		avgretpb_vec.push_back(stod(results[rowCtr * columns + 5]));
		cout << rowCtr << "  " << results[rowCtr * columns] << endl;
	}
	sqlite3_free_table(results);

	// extract data before backtest period
	char sql_select[512];
	sprintf_s(sql_select, "SELECT SP500Clean.symbol, Prices.DailyRet\
		FROM Prices, SP500Clean\
		WHERE Prices.date < \"%s\"\
		AND SP500Clean.symbol = Prices.symbol;", dateBT.c_str());
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
	/*tmp = "SELECT SP500Clean.symbol, Prices.DailyRet\
		FROM Prices, SP500Clean\
		WHERE Prices.date < \"2019-01-01\"\
		AND SP500Clean.symbol = Prices.symbol;";
	sqlite3_get_table(db, tmp.c_str(), &results, &rows, &columns, &error);*/
	// vector<string> symbol_vec;
	int N = symbol_vec.size();
	vector< vector<double> > retbt_vec(N);
	for (int i = 0; i < N; i++)
	{
		for (int rowCtr = 1; rowCtr <= rows; rowCtr++)
		{
			if (results[rowCtr * columns] == symbol_vec[i])
				retbt_vec[i].push_back(stod(results[rowCtr * columns + 1]));
		}
	}
	sqlite3_free_table(results);

	// extract data before probation period
	sprintf_s(sql_select, "SELECT SP500Clean.symbol, Prices.DailyRet\
		FROM Prices, SP500Clean\
		WHERE Prices.date < \"%s\"\
		AND SP500Clean.symbol = Prices.symbol;", datePB.c_str());
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
	/*tmp = "SELECT SP500Clean.symbol, Prices.DailyRet\
		FROM Prices, SP500Clean\
		WHERE Prices.date < \"2020-01-01\"\
		AND SP500Clean.symbol = Prices.symbol;";
	sqlite3_get_table(db, tmp.c_str(), &results, &rows, &columns, &error);*/

	vector< vector<double> > retpb_vec(N);
	for (int i = 0; i < N; i++)
	{
		for (int rowCtr = 1; rowCtr <= rows; rowCtr++)
		{
			if (results[rowCtr * columns] == symbol_vec[i])
				retpb_vec[i].push_back(stod(results[rowCtr * columns + 1]));
		}
	}
	sqlite3_free_table(results);


	// Insert
	/*tmp = "DROP TABLE IF EXISTS Covariance;\
			CREATE TABLE Covariance(symbol1 CHAR(20) NOT NULL, symbol2 CHAR(20) NOT NULL,\
			Var1BT REAL NOT NULL, Var2BT REAL NOT NULL, CovBT REAL NOT NULL,\
			Var1PB REAL NOT NULL, Var2PB REAL NOT NULL, CovPB REAL NOT NULL,\
			PRIMARY KEY(symbol1, symbol2));";*/
	tmp = "DROP TABLE IF EXISTS Covariance;\
			CREATE TABLE Covariance(symbol1 CHAR(20) NOT NULL, symbol2 CHAR(20) NOT NULL,\
			AvgRet1BT REAL NOT NULL, Var1BT REAL NOT NULL, AvgRet2BT REAL NOT NULL, Var2BT REAL NOT NULL, CovBT REAL NOT NULL,\
			AvgRet1PB REAL NOT NULL, Var1PB REAL NOT NULL, AvgRet2PB REAL NOT NULL, Var2PB REAL NOT NULL, CovPB REAL NOT NULL,\
			PRIMARY KEY(symbol1, symbol2));";
	sqlite3_exec(db, tmp.c_str(), NULL, NULL, &error);

	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	const char* sql = "INSERT INTO Covariance VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
	for (int i = 0; i < N; i++)
	{
		string symbol1 = symbol_vec[i];
		string sector1 = sector_vec[i];
		//int num1 = nums_list[i];
		cout << "------------------------------------------------------------------------------------------------" << endl;
		for (int j = 0; j < N; j++)
		{
			if (i == j)
				continue;
			string symbol2 = symbol_vec[j];
			string sector2 = sector_vec[j];
			//int num2 = nums_list[j];
			cout << setw(5) << left << i
				<< setw(5) << left << symbol1
				<< setw(8) << left << "||"
				<< setw(5) << left << j
				<< setw(5) << left << symbol2 << endl;
			//if (symbol1 != symbol2 && sector1 != sector2)
			if (sector1 != sector2)
			{
				double avg_ret1bt = avgretbt_vec[i];
				double var1bt = varbt_vec[i];
				double avg_ret2bt = avgretbt_vec[j];
				double var2bt = varbt_vec[j];
				double covbt = Cov(retbt_vec[i], retbt_vec[j]);
				double avg_ret1pb = avgretpb_vec[i];
				double var1pb = varpb_vec[i];
				double avg_ret2pb = avgretpb_vec[j];
				double var2pb = varpb_vec[j];
				double covpb = Cov(retpb_vec[i], retpb_vec[j]);

				sqlite3_bind_text(stmt, 1, symbol1.c_str(), -1, SQLITE_STATIC);
				sqlite3_bind_text(stmt, 2, symbol2.c_str(), -1, SQLITE_STATIC);
				sqlite3_bind_double(stmt, 3, avg_ret1bt);
				sqlite3_bind_double(stmt, 4, var1bt);
				sqlite3_bind_double(stmt, 5, avg_ret2bt);
				sqlite3_bind_double(stmt, 6, var2bt);
				sqlite3_bind_double(stmt, 7, covbt);
				sqlite3_bind_double(stmt, 8, avg_ret1pb);
				sqlite3_bind_double(stmt, 9, var1pb);
				sqlite3_bind_double(stmt, 10, avg_ret2pb);
				sqlite3_bind_double(stmt, 11, var2pb);
				sqlite3_bind_double(stmt, 12, covbt);
				sqlite3_step(stmt);
				sqlite3_reset(stmt);
			}
		}
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);
}

void ProcessAvgRet(sqlite3* db)
{
	char* error = NULL;
	char** results = NULL;
	int rows, columns;

	string tmp = "CREATE VIEW TP1 AS\
				SELECT DISTINCT symbol, Avg(DailyRet) AS AvgRetBT FROM Prices\
				WHERE date < \"2019-01-01\"\
				GROUP BY symbol;\
				ALTER TABLE SP500Clean ADD COLUMN AvgRetBT REAL;\
				UPDATE SP500Clean SET AvgRetBT = \
				(SELECT AvgRetBT FROM TP1\
				WHERE SP500Clean.symbol = TP1.symbol);\
				DROP VIEW TP1;";
	sqlite3_exec(db, tmp.c_str(), NULL, NULL, &error);

	tmp = "CREATE VIEW TP2 AS\
				SELECT DISTINCT symbol, Avg(DailyRet) AS AvgRetPB FROM Prices\
				WHERE date < \"2020-01-01\"\
				GROUP BY symbol;\
				ALTER TABLE SP500Clean ADD COLUMN AvgRetPB REAL;\
				UPDATE SP500Clean SET AvgRetPB = \
				(SELECT AvgRetPB FROM TP2\
				WHERE SP500Clean.symbol = TP2.symbol);\
				DROP VIEW TP2;";
	sqlite3_exec(db, tmp.c_str(), NULL, NULL, &error);
}

void CalculateFitness(Population& Pop, int start_index, int end_index)
{
	vector<Portfolio> element_ports = Pop.GetPopulation();
	for (int i = start_index; i < end_index; i++)
	{
		element_ports[i].EfficientFrontier();
		Pop.SetElementPorts(element_ports[i], i);
	}
}

void MultiCalculateFitness(Population& Pop, int begin, int over)
{
	int size = (over - begin) / 5;
	int a = 0, b = a + size, c = b + size, d = c + size, e = d + size, f = over - begin;

	thread t1(CalculateFitness, ref(Pop), a, b);
	thread t2(CalculateFitness, ref(Pop), b, c);
	thread t3(CalculateFitness, ref(Pop), c, d);
	thread t4(CalculateFitness, ref(Pop), d, e);
	thread t5(CalculateFitness, ref(Pop), e, f);

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
}

void Portfolio::CalculateDailyRet()
{
	vector < vector<double> > Stocks_DailyRet(11);
	// Extract Daily Return Data
	char* error = NULL;
	char** results = NULL;
	int rows, columns;
	char sql_select[512];
	for (int i = 0; i < symbols.size(); i++)
	{
		string symbol = symbols[i];
		if (period == "BT")
		{
			/*string period1 = "2019-01-01", period2 = "2020-01-01";*/
			string period1 = "2020-01-01", period2 = "2020-07-01";
			sprintf(sql_select, "SELECT DailyRet FROM Prices WHERE symbol = \"%s\"\
								AND date >= \"%s\" AND date < \"%s\";", symbol.c_str(), period1.c_str(), period2.c_str());
		}
		else if (period == "PB")
		{
			/*string period1 = "2020-01-01";*/
			string period1 = "2020-07-01";
			sprintf(sql_select, "SELECT DailyRet FROM Prices WHERE symbol = \"%s\"\
								AND date >= \"%s\";", symbol.c_str(), period1.c_str());
		}
		sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
		for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
		{
			double dailyret = stod(results[rowCtr]);
			Stocks_DailyRet[i].push_back(dailyret);
		}
		Stocks_DailyRet[i][0] = 0.0;	// First Day, Ret = 0
		sqlite3_free_table(results);
	}
	// Calculate Portfolio Daily Return
	/*for (int i = 0; i < Stocks_DailyRet[0].size(); i++)
	{
		double res = 0.0;
		for (int j = 0; j < 11; j++)
			res += optimal_weight[j] * Stocks_DailyRet[j][i];
		daily_ret.push_back(res);
	}*/
	daily_ret = dot(optimal_weight, Stocks_DailyRet);

	double vol = pow(Cov(daily_ret, daily_ret), 0.5);	// Square Root
	summary.SetVol(vol);
}

// Calculate Cmulative Daily Return
void Portfolio::CalculateCumuRetAndMdd()
{
	vector<double> b = daily_ret + 1.0;
	double max_val = b[0], max_dd = 0.0;
	// Cumulative Product
	for (int i = 1; i < b.size(); i++)
	{
		b[i] *= b[i - 1];
		max_val = max(max_val, b[i - 1]);
		max_dd = max(max_dd, 1.0 - b[i] / max_val);
	}
	cumulative_ret = b - 1.0;
	double totalret = cumulative_ret[cumulative_ret.size() - 1];

	summary.SetMDD(max_dd);
	summary.SetTotalRet(totalret);
}

// Monte Carlo -> Max Sharpe Ratio
// Period == "BT" or "PB"
void Portfolio::EfficientFrontier()
{
	// First, Build Covariance Matrix
	char* error = NULL;
	char** results = NULL;
	int rows, columns;
	char sql_select[512];
	vector< vector<double> > Cov_Mat(11);

	clock_t start_c = clock();
	
	for (int i = 0; i < 11; i++)
	{
		vector<double> row(11);
		for (int j = 0; j < 11; j++)
		{
			if (i == j)	// Variance
			{
				if (period == "BT")
					sprintf(sql_select, "SELECT VarBT FROM SP500Clean\
					 WHERE symbol = \"%s\";", symbols[i].c_str());
				else if (period == "PB")
					sprintf(sql_select, "SELECT VarPB FROM SP500Clean\
					 WHERE symbol = \"%s\";", symbols[i].c_str());

				sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
				row[j] = stod(results[1]);
				sqlite3_free_table(results);
			}
			else if (i < j)	// Right Upper Corner
			{
				if (period == "BT")
					sprintf(sql_select, "SELECT CovBT FROM Covariance\
					 WHERE symbol1 = \"%s\" AND symbol2 = \"%s\";", symbols[i].c_str(), symbols[j].c_str());

				else if (period == "PB")
					sprintf(sql_select, "SELECT CovPB FROM Covariance\
					 WHERE symbol1 = \"%s\" AND symbol2 = \"%s\";", symbols[i].c_str(), symbols[j].c_str());

				sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
				row[j] = stod(results[1]);
				sqlite3_free_table(results);
			}
			else if (i > j)	// Left Lower Corner
			{
				row[j] = Cov_Mat[j][i];	// Symmetric
			}
		}
		Cov_Mat[i] = row;
	}
	clock_t end_c = clock();
	// First, Generate 10000 different Weights Combination
	// For each weight combination, sum = 1.0
	// Long 1.3, Short 0.3
	// Third, w.T * Cov_Mat * W -> Generate Portfolio Vol

	vector< vector<double> > Weights(10000);
	vector<double> port_exp_ret(10000);
	vector<double> sr_vec(10000);
	vector<double> vol_vec(10000);
	for (int i = 0; i < 10000; i++)
	{
		vector<double> Weight(11);
		double sum_positive = 0.0, sum_negative = 0.0;
		for (int j = 0; j < 11; j++)
		{
			double w = (float)rand() / RAND_MAX;
			if (buy_or_sell[j] > 3)
			{
				sum_positive += w;
				Weight[j] = w;
			}
			else
			{
				sum_negative += w;
				Weight[j] = -w;
			}
		}
		for (int j = 0; j < 11; j++)
		{
			if (buy_or_sell[j] > 3)
				Weight[j] /= (sum_positive / 1.3);
			else
				Weight[j] /= (sum_negative / 0.3);
		}
		Weights[i] = Weight;
		port_exp_ret[i] = dot(stock_avg_ret, Weight);
		vol_vec[i] = sqrt(dot(Weight, Cov_Mat, Weight));	// Square Root
		sr_vec[i] = (port_exp_ret[i] - rf / 252.0) / vol_vec[i];
	}

	// Find Index of Max SR
	vector<double>::iterator Limit = max_element(begin(sr_vec), end(sr_vec));
	int Limit_dist = distance(std::begin(sr_vec), Limit);
	optimal_weight = Weights[Limit_dist];
	fitness = sr_vec[Limit_dist];
	exp_ret = port_exp_ret[Limit_dist];
}

void Portfolio::SetSPY()
{
	char* error = NULL;
	char** results = NULL;
	int rows, columns;
	char sql_select[512];
	if (period == "BT")
	{
		/*string period1 = "2019-01-01";
		string period2 = "2020-01-01";*/
		string period1 = "2020-01-01";
		string period2 = "2020-07-01";
		sprintf(sql_select, "SELECT DailyRet FROM SPY WHERE date >= \"%s\" and date < \"%s\";", period1.c_str(), period2.c_str());
	}
	else if (period == "PB")
	{
		/*string period1 = "2020-01-01";*/
		string period1 = "2020-07-01";
		sprintf(sql_select, "SELECT DailyRet FROM SPY WHERE date >= \"%s\";", period1.c_str());
	}
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
	//spy_cumulative_ret.push_back(0.0);
	for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
	{
		// Determine Cell Position
		double tmp = stod(results[rowCtr]);
		spy_dailyret.push_back(tmp);
	}
	sqlite3_free_table(results);
	spy_dailyret[0] = 0.0;	// First day, no return

	vector<double> b = spy_dailyret + 1.0;
	// Cumulative Product
	for (int i = 1; i < b.size(); i++)
		b[i] *= b[i - 1];
	spy_cumulative_ret = b - 1.0;

	double total_mkt_ret = spy_cumulative_ret[spy_cumulative_ret.size() - 1];
	summary.SetTotalMarketRet(total_mkt_ret);
}

void Portfolio::CalculateSummaryStats()
{
	// Length of Period
	int days = daily_ret.size();
	summary.SetDaysCount(days);

	// Beta
	double covariance = Cov(daily_ret, spy_dailyret);
	double variance_spy = Cov(spy_dailyret, spy_dailyret);
	double beta = covariance / variance_spy;
	summary.SetBeta(beta);

	// Alpha
	double totalret = summary.GetTotalRet() * 252 / days;	// Annualization
	double totalspyret = summary.GetTotalMktRet() * 252 / days;
	double alpha = totalret - rf - beta * (totalspyret - rf);
	summary.SetAlpha(alpha);
	
	// fitness & IR
	summary.SetFitness(fitness * pow(252, 0.5));	// Annualization

	vector<double> DiffRet = daily_ret - spy_dailyret;
	double TrackError = pow(Cov(DiffRet, DiffRet) * 252, 0.5);
	double IR = (totalret - totalspyret) / TrackError;
	summary.SetIR(IR);

	// 95% Daily VaR
	double vol = summary.GetVol();	// Daily Vol
	vector<double> sorted_dailyret(daily_ret);
	sort(sorted_dailyret.rbegin(), sorted_dailyret.rend());	// Descending Order
	int index = 0.95 * sorted_dailyret.size() - 1;
	double var = -sorted_dailyret[index];	// VaR is Loss -> Negative of ret
	double es = -Avg(sorted_dailyret, index, sorted_dailyret.size());	// Negative
	summary.SetVaR(var);
	summary.SetES(es);
}


void Population::ConstructPopulation()
{
	char* error = NULL;
	char** results = NULL;
	int rows, columns;

	//// Randomly Exclude 1 sector; Make sure 0 <= tmp <= (size - 1)
	symbols.resize(50);
	for (auto it = sectors.begin(); it != sectors.end(); it++)
	{
		string sql_select_str = "SELECT symbol FROM SP500Clean WHERE sector = '" + *it + "';";
		const char* sql_select = sql_select_str.c_str();
		sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);

		for (int i = 0; i < 50; i++)
		{
			int num = ((float)rand() / (RAND_MAX + 1)) * rows + 1;	// Skip the header, 1 <= num <= rows
			symbols[i].push_back(results[num]);
		}
		sqlite3_free_table(results);
	}

	for (int i = 0; i < 50; i++)
	{
		Portfolio port_tmp(period, db, symbols[i], rf, sp500obj);
		element_ports.push_back(port_tmp);
	}
}

void Population::SortFitness(string order)
{
	if (order == "D")
		sort(element_ports.begin(), element_ports.end(), descending);
	else if (order == "A")
		sort(element_ports.begin(), element_ports.end(), ascending);

	for (int i = 0; i < POP_SIZE; i++)
		symbols[i] = element_ports[i].GetSymbols();
}

