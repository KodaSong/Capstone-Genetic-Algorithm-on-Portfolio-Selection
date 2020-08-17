/*
 * @Author: Koda Song
 * @Date: 2020-06-07 11:49:54
 * @LastEditors: Koda Song
 * @LastEditTime: 2020-08-05 10:12:13
 * @Description: Table.h
 */ 
#pragma once
#ifndef Table_h
#define Table_h

#include "database.h"
#include "marketdata.h"
#include "Stock.h"
#include <thread>

// Bind Download & Parse
// Download Data from "Start" till "End" in "Stock_List" 
// & Parse into vector of objects
int DownloadAndParse(vector<Trade_New>& trade_vec, vector<string>& stock_list,
	int start_index, int end_index, string stock_start_date, string stock_end_date, int count)
{
	long volume;
	string date, symbol;
	float open, high, low, close, adjusted_close;

	//int count = 0;
	for (int i = start_index; i != end_index; i++)
	{
		symbol = stock_list[i];
		if (symbol == "LOWE")	// I first change "LOW" -> "LOWE" in SP500 Table, in order to avoid ambiguous in further select
			symbol = "LOW";		// When download, we need to use original symbol "LOW"
		/*if (symbol == "BF.B")
			symbol = "BFB";
		if (symbol == "BRK.B")
			symbol = "BRKB";*/
		string stock_url_common = "https://eodhistoricaldata.com/api/eod/";
		string api_token = "5ba84ea974ab42.45160048";
		string stockDB_data_request = stock_url_common + symbol + ".US?" +
			"from=" + stock_start_date + "&to=" + stock_end_date + "&api_token=" + api_token + "&period=d&fmt=json";

		Json::Value stockDB_root;   // will contains the root value after parsing.
		if (RetrieveMarketData(stockDB_data_request, stockDB_root) == -1)
			return -1;

		if (symbol == "LOW")	// After downloading, we change back to "LOWE" again
			symbol = "LOWE";

		for (Json::Value::const_iterator itr = stockDB_root.begin(); itr != stockDB_root.end(); itr++)
		{
			date = (*itr)["date"].asString();
			open = (*itr)["open"].asFloat();
			high = (*itr)["high"].asFloat();
			low = (*itr)["low"].asFloat();
			close = (*itr)["close"].asFloat();
			adjusted_close = (*itr)["adjusted_close"].asFloat();
			volume = (*itr)["volume"].asInt64();
			Trade_New aTrade(symbol, date, open, high, low, close, adjusted_close, volume);
			trade_vec[count++] = aTrade;
		}
		//root_vec[i] = NULL;		// Save memory!
	}
}

int MultiThread(vector<Trade_New>& trade_vec, vector<string>& stock_list,
	string stock_start_date, string stock_end_date)
{
	int size1 = stock_list.size() / 5;
	int a1 = 0, b1 = a1 + size1, c1 = b1 + size1, d1 = c1 + size1, e1 = d1 + size1, f1 = stock_list.size();

	int size2 = trade_vec.size() / 5;
	int a2 = 0, b2 = a2 + size2, c2 = b2 + size2, d2 = c2 + size2, e2 = d2 + size2;

	//vector<Trade_New> trade_vec1, trade_vec2, trade_vec3, trade_vec4, trade_vec5;

	thread t6(DownloadAndParse, ref(trade_vec), ref(stock_list), a1, b1, stock_start_date, stock_end_date, a2);
	thread t7(DownloadAndParse, ref(trade_vec), ref(stock_list), b1, c1, stock_start_date, stock_end_date, b2);
	thread t8(DownloadAndParse, ref(trade_vec), ref(stock_list), c1, d1, stock_start_date, stock_end_date, c2);
	thread t9(DownloadAndParse, ref(trade_vec), ref(stock_list), d1, e1, stock_start_date, stock_end_date, d2);
	thread t10(DownloadAndParse, ref(trade_vec), ref(stock_list), e1, f1, stock_start_date, stock_end_date, e2);

	t6.join();
	t7.join();
	t8.join();
	t9.join();
	t10.join();

	return 0;
}

// Store Prices Data into dB
void StoreData(vector<Trade_New>& trade_vec1, sqlite3* db)
{
	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	const char* sql = "INSERT INTO Prices VALUES(?, ?, ?, ?, ?, ?, ?, ?)";
	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
	long N1 = trade_vec1.size();

	string d, s;
	const char* date, * symbol;

	for (long i = 0; i < N1; i++)
	{
		d = trade_vec1[i].get_date(); date = d.c_str();
		s = trade_vec1[i].get_symbol(); symbol = s.c_str();

		if (d.size() == 0)
			continue;

		sqlite3_bind_text(stmt, 1, symbol, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, date, -1, SQLITE_STATIC);
		sqlite3_bind_double(stmt, 3, trade_vec1[i].get_open());
		sqlite3_bind_double(stmt, 4, trade_vec1[i].get_high());
		sqlite3_bind_double(stmt, 5, trade_vec1[i].get_low());
		sqlite3_bind_double(stmt, 6, trade_vec1[i].get_close());
		sqlite3_bind_double(stmt, 7, trade_vec1[i].get_adjclose());
		sqlite3_bind_int64(stmt, 8, trade_vec1[i].get_volume());

		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);
}

// Parse SP500 List in Root and Store it in Table "SP500"
int PopulateStockTable(const Json::Value& root, string symbol, sqlite3* db)
{
	string d;
	float open, high, low, close, adjusted_close;
	long volume;

	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	/*const char* sql = "INSERT INTO Prices VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)";*/
	const char* sql = "INSERT INTO Prices VALUES(?, ?, ?, ?, ?, ?, ?, ?)";

	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);

	for (Json::Value::const_iterator itr = root.begin(); itr != root.end(); itr++)
	{
		adjusted_close = (*itr)["adjusted_close"].asFloat();
		close = (*itr)["close"].asFloat();
		high = (*itr)["high"].asFloat();
		low = (*itr)["low"].asFloat();
		open = (*itr)["open"].asFloat();
		volume = (*itr)["volume"].asInt64();
		d = (*itr)["date"].asString();

		const char* date = d.c_str();
		const char* s = symbol.c_str();

		//sqlite3_bind_int(stmt, 1, count++);
		sqlite3_bind_text(stmt, 1, s, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, date, -1, SQLITE_STATIC);
		sqlite3_bind_double(stmt, 3, open);
		sqlite3_bind_double(stmt, 4, high);
		sqlite3_bind_double(stmt, 5, low);
		sqlite3_bind_double(stmt, 6, close);
		sqlite3_bind_double(stmt, 7, adjusted_close);
		sqlite3_bind_int64(stmt, 8, volume);
		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);
	return 0;
}

// Download + Parse + Store SP500 List
int StockTable(Json::Value& stockDB_root, string stockDB_symbol, sqlite3* stockDB,
	string stock_start_date, string stock_end_date)
{
	string stock_url_common = "https://eodhistoricaldata.com/api/eod/";
	/*string stock_start_date = "2019-01-01";
	string stock_end_date = "2019-12-31";*/
	string api_token = "5ba84ea974ab42.45160048";
	string stockDB_data_request = stock_url_common + stockDB_symbol + ".US?" +
		"from=" + stock_start_date + "&to=" + stock_end_date + "&api_token=" + api_token + "&period=d&fmt=json";

	//Json::Value stockDB_root;   // will contains the root value after parsing.
	if (RetrieveMarketData(stockDB_data_request, stockDB_root) == -1)
		return -1;
	if (PopulateStockTable(stockDB_root, stockDB_symbol, stockDB) == -1)
		return -1;
}


// Separate Download & Parse
// Download Prices Data for stocks from "Start" till "End" in "Stock_List"
// Store them in "Root_Vec"
int DownloadJson(vector<Json::Value>& root_vec, vector<string>& stock_list, int start_index, int end_index, string stock_start_date, string stock_end_date)
{
	string symbol;
	for (int i = start_index; i != end_index; i++)
	{
		symbol = stock_list[i];
		if (symbol == "LOWE")	// I first change "LOW" -> "LOWE" in SP500 Table, in order to avoid ambiguous in further select
			symbol = "LOW";		// When download, we need to use original symbol "LOW"
		/*if (symbol == "BF.B")
			symbol = "BFB";
		if (symbol == "BRK.B")
			symbol = "BRKB";*/

		string stock_url_common = "https://eodhistoricaldata.com/api/eod/";
		string api_token = "5ba84ea974ab42.45160048";
		string stockDB_data_request = stock_url_common + symbol + ".US?" +
			"from=" + stock_start_date + "&to=" + stock_end_date + "&api_token=" + api_token + "&period=d&fmt=json";

		Json::Value stockDB_root;   // will contains the root value after parsing.
		if (RetrieveMarketData(stockDB_data_request, stockDB_root) == -1)
			return -1;
		root_vec[i] = stockDB_root;
	}
}

void MultiDownload(vector<Json::Value>& root_vec, vector<string>& stock_list, string stock_start_date, string stock_end_date)
{
	int size = stock_list.size() / 5;
	int a = 0, b = a + size, c = b + size, d = c + size, e = d + size, f = stock_list.size();

	thread t1(DownloadJson, ref(root_vec), ref(stock_list), a, b, stock_start_date, stock_end_date);
	thread t2(DownloadJson, ref(root_vec), ref(stock_list), b, c, stock_start_date, stock_end_date);
	thread t3(DownloadJson, ref(root_vec), ref(stock_list), c, d, stock_start_date, stock_end_date);
	thread t4(DownloadJson, ref(root_vec), ref(stock_list), d, e, stock_start_date, stock_end_date);
	thread t5(DownloadJson, ref(root_vec), ref(stock_list), e, f, stock_start_date, stock_end_date);

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
}

// Parse "Root_Vec" & Store in "Trade_Vec"	object(symbol, date, high...)
void ParseJson(vector<Trade_New>& trade_vec, vector<Json::Value>& root_vec, vector<string>& stock_list)
{
	string date, symbol;
	float open, high, low, close, adjusted_close;
	long volume;

	int count = 0;
	for (int i = 0; i != root_vec.size(); i++)
	{
		symbol = stock_list[i];
		cout << (i + 1) << " " << symbol << endl;
		for (Json::Value::const_iterator itr = root_vec[i].begin(); itr != root_vec[i].end(); itr++)
		{
			date = (*itr)["date"].asString();
			open = (*itr)["open"].asFloat();
			high = (*itr)["high"].asFloat();
			low = (*itr)["low"].asFloat();
			close = (*itr)["close"].asFloat();
			adjusted_close = (*itr)["adjusted_close"].asFloat();
			volume = (*itr)["volume"].asInt64();
			Trade_New aTrade(symbol, date, open, high, low, close, adjusted_close, volume);
			//trade_vec.push_back(aTrade);
			trade_vec[count++] = aTrade;
		}
		root_vec[i] == NULL;	// Save memory!
	}
}

// An integration of functions 
// "MultiThread_Download" -> "Parse_JSON" -> "Store_Data1"
// Return the last index in dB
int DoFirstTime(vector<Trade_New>& trade_vec1, vector<string>& stock_list1, sqlite3* stockDB, string stock_start_date, string stock_end_date)
{
	string price_drop_table = "DROP TABLE IF EXISTS Prices;";
	if (DropTable(price_drop_table.c_str(), stockDB) == -1)
		return -1;

	string table_name = "Prices";

	string stockDB_create_table = "CREATE TABLE " + table_name
		+ "(symbol CHAR(20) NOT NULL, "
		+ "date CHAR(20) NOT NULL, "
		+ "open REAL NOT NULL, "
		+ "high REAL NOT NULL, "
		+ "low REAL NOT NULL, "
		+ "close REAL NOT NULL, "
		+ "adjusted_close REAL NOT NULL, "
		+ "volume INT NOT NULL, "
		+ "PRIMARY KEY(symbol, date)) ";
	//	+ "FOREIGN KEY(symbol) REFERENCES SP500(symbol));";

	if (CreateTable(stockDB_create_table.c_str(), stockDB) == -1)
		return -1;

	// Download -> Parse -> Store

	/*cout << endl << "Prepare to Download Prices Data" << endl;	Sleep(3000);
	MultiThread_Download(root_vec1, stock_list1, stock_start_date, stock_end_date);

	cout << endl << "Prepare to Parse Json" << endl;	Sleep(5000);
	Parse_JSON(trade_vec1, root_vec1, stock_list1);*/
	/*Multi_Thread(trade_vec1, root_vec1, stock_list1, stock_start_date, stock_end_date);*/
	MultiThread(trade_vec1, stock_list1, stock_start_date, stock_end_date);

	cout << endl << "Prepare to Store Data" << endl;	Sleep(5000);
	int count = 0;
	StoreData(trade_vec1, stockDB);

	cout << "Prepare to Free Memory" << endl;	Sleep(1000);
	//vector<Json::Value>().swap(root_vec1);	// Free memory of root_vec1
	vector<Trade_New>().swap(trade_vec1);	// Free memory of trade_vec1

	return count;
}

// Similar with "Do_First_Time"
// Process left stocks
void TestLeftStock(sqlite3* stockDB, string stock_start_date, string stock_end_date)
{
	vector<Trade_New> trade_vec2(100000);
	vector<string> stock_list2;
	Json::Value stockDB_root;
	// Find the difference set between symbol in SP500 and Prices, & store in "stock_list2"
	Select_Left_Stock_Table(stockDB, stock_list2);
	vector<Json::Value> root_vec2(stock_list2.size());

	int g1 = 0;
	while (g1++ < 3 && stock_list2.size() >= 5)	// When Num of Left Stocks >= 5
	{
		cout << "These stocks need to be redownloaded!" << endl;
		for (int i = 0; i < stock_list2.size(); i++)
			cout << (i + 1) << " " << stock_list2[i] << endl;

		int i = 1;
		// Download -> Parse -> Store
		cout << i++ << " Try Begin" << endl;	Sleep(3000);
		MultiDownload(root_vec2, stock_list2, stock_start_date, stock_end_date);
		ParseJson(trade_vec2, root_vec2, stock_list2);
		/*Multi_Thread(trade_vec2, root_vec2, stock_list2, stock_start_date, stock_end_date);*/
		//Multi_Thread(trade_vec2, stock_list2, stock_start_date, stock_end_date);

		/*count = Store_Data2(trade_vec2, stockDB, count);*/
		StoreData(trade_vec2, stockDB);

		vector<string>().swap(stock_list2);	// Free memory of stock_list2
		vector<Json::Value>().swap(root_vec2);	// Free memory of root_vec2
		vector<Trade_New>().swap(trade_vec2);	// Free memory of trade_vec2

		vector<Trade_New> trade_vec2(100000);
		// Find the difference set between symbol in SP500 and Prices, & store in "stock_list2"
		Select_Left_Stock_Table(stockDB, stock_list2);
		vector<Json::Value> root_vec2(stock_list2.size());
	}

	int g2 = 0;
	while (g2++ < 3 && stock_list2.size() > 0)	// When Num of Left Stocks < 5
	{
		cout << "Try One More Time!" << endl;
		for (int i = 0; i < stock_list2.size(); i++)
		{
			Sleep(1000);
			//count = StockTable(stockDB_root, stock_list2[i], stockDB, stock_start_date, stock_end_date, count);
			StockTable(stockDB_root, stock_list2[i], stockDB, stock_start_date, stock_end_date);
		}

		vector<string>().swap(stock_list2);	// Free memory of stock_list2
		Select_Left_Stock_Table(stockDB, stock_list2);
	}

	if (stock_list2.size() > 0)
	{
		cout << "These stocks cannot be downloaded!" << endl;
		for (int i = 0; i < stock_list2.size(); i++)
			cout << (i + 1) << " " << stock_list2[i] << endl << endl;
	}
	else
	{
		cout << "All Stocks Downloaded!" << endl << endl;
	}
}


// Parse SP500 List in Root and Store it in Table "SP500"
int PopulateSP500Table(const Json::Value& root, sqlite3* db, vector<string>& stock_list)
{
	string name, symbol, sector;
	for (Json::Value::const_iterator itr = root.begin(); itr != root.end(); itr++)
	{
		name = (*itr)["Name"].asString();
		sector = (*itr)["Sector"].asString();
		symbol = (*itr)["Symbol"].asString();
		stock_list.push_back(symbol);

		// Execute SQL
		char sp500_insert_table[512];
		sprintf_s(sp500_insert_table, "INSERT INTO SP500 (symbol, name, sector) VALUES(\"%s\", \"%s\", \"%s\")", symbol.c_str(), name.c_str(), sector.c_str());
		if (InsertTable(sp500_insert_table, db) == -1)
			return -1;
	}
	return 0;
}

// Download + Parse + Store SP500 List
int SP500Table(Json::Value& sp500_root, sqlite3* stockDB, vector<string>& stock_list)
{
	string sp500_create_table = "CREATE TABLE SP500 (symbol CHAR(20) PRIMARY KEY NOT NULL, name CHAR(20) NOT NULL, sector CHAR(20) NOT NULL);";

	if (CreateTable(sp500_create_table.c_str(), stockDB) == -1)
		return -1;

	string sp500_data_request = "https://pkgstore.datahub.io/core/s-and-p-500-companies/constituents_json/data/64dd3e9582b936b0352fdd826ecd3c95/constituents_json.json";
	//Json::Value sp500_root;   // will contains the root value after parsing.
	if (RetrieveMarketData(sp500_data_request, sp500_root) == -1)
		return -1;
	if (PopulateSP500Table(sp500_root, stockDB, stock_list) == -1)
		return -1;

}


int ProcessFundamental(vector<Fundamental>& funda_vec, vector<string>& stock_list, int start_index, int end_index)
{
	int64_t mktcap;
	double div_yield, beta, High52, Low52, MA50, MA200, EPS, short_ratio;
	for (int i = start_index; i != end_index; i++)
	{
		string symbol = stock_list[i];
		if (symbol == "LOWE")	// I first change "LOW" -> "LOWE" in SP500 Table, in order to avoid ambiguous in further select
			symbol = "LOW";		// When download, we need to use original symbol "LOW"

		string stock_url_common = "https://eodhistoricaldata.com/api/fundamentals/";
		string api_token = "5ba84ea974ab42.45160048";
		string stockDB_data_request = stock_url_common + symbol + ".US?api_token=" + api_token;

		Json::Value stockDB_root;   // will contains the root value after parsing.
		if (RetrieveMarketData(stockDB_data_request, stockDB_root) == -1)
			return -1;

		//int count = 0;
		if (symbol == "LOW")	// Change back to "LOWE"
			symbol = "LOWE";		
		for (Json::Value::const_iterator itr = stockDB_root.begin(); itr != stockDB_root.end(); itr++)
		{
			for (Json::Value::const_iterator inner = (*itr).begin(); inner != (*itr).end(); inner++)
			{
				if (inner.key().asString() == "MarketCapitalization")
					mktcap = inner->asInt64();
				else if (inner.key().asString() == "EarningsShare")
					EPS = inner->asDouble();
				else if (inner.key().asString() == "DividendYield")
					div_yield = inner->asDouble();
				else if (inner.key().asString() == "200DayMA")
					MA200 = inner->asDouble();
				else if (inner.key().asString() == "50DayMA")
					MA50 = inner->asDouble();
				else if (inner.key().asString() == "Beta")
					beta = inner->asDouble();
				else if (inner.key().asString() == "52WeekHigh")
					High52 = inner->asDouble();
				else if (inner.key().asString() == "52WeekLow")
					Low52 = inner->asDouble();
				else if (inner.key().asString() == "ShortRatio")
					short_ratio = inner->asDouble();
			}
			
		}
		Fundamental funda(symbol, div_yield, beta, High52, Low52, MA50, MA200, mktcap, EPS, short_ratio);
		funda_vec[i] = funda;
	}
}

void MultiProcessFundamental(sqlite3* db)
{
	// Get symbols from Table SP500
	char* error = NULL;
	char** results = NULL;
	int rows, columns;

	string tmp = "SELECT symbol FROM SP500;";
	sqlite3_get_table(db, tmp.c_str(), &results, &rows, &columns, &error);

	// Extract Table
	vector<string> stock_list;
	string symbol;
	for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
	{
		symbol = results[rowCtr];
		// Determine Cell Position
		/*if (symbol == "BF.B")
			symbol = "BFB";
		if (symbol == "BRK.B")
			symbol = "BRKB";*/
		cout << rowCtr << " " << symbol << endl;
		stock_list.push_back(symbol);
	}
	sqlite3_free_table(results);


	int size = stock_list.size() / 5;
	int a = 0, b = a + size, c = b + size, d = c + size, e = d + size, f = stock_list.size();
	vector<Fundamental> funda_vec(size * 5);

	thread t1(ProcessFundamental, ref(funda_vec), ref(stock_list), a, b);
	thread t2(ProcessFundamental, ref(funda_vec), ref(stock_list), b, c);
	thread t3(ProcessFundamental, ref(funda_vec), ref(stock_list), c, d);
	thread t4(ProcessFundamental, ref(funda_vec), ref(stock_list), d, e);
	thread t5(ProcessFundamental, ref(funda_vec), ref(stock_list), e, f);

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();

	// Insert Into Table Fundamental
	tmp = "DROP TABLE IF EXISTS Fundamental;";
	sqlite3_exec(db, tmp.c_str(), NULL, NULL, &error);

	string table_name = "Fundamental";
	string Fundamental_create_table = "CREATE TABLE " + table_name
		+ "(symbol CHAR(20) NOT NULL, "
		+ "DivYield REAL NOT NULL, "
		+ "Beta REAL NOT NULL, "
		+ "High52 REAL NOT NULL, "
		+ "Low52 REAL NOT NULL, "
		+ "MA50 REAL NOT NULL, "
		+ "MA200 REAL NOT NULL, "
		+ "MktCap INT NOT NULL, "
		+ "EPS REAL NOT NULL, "
		+ "ShortRatio REAL NOT NULL, "
		+ "PRIMARY KEY(symbol)); ";
	//	+ "FOREIGN KEY(symbol) REFERENCES SP500(symbol));";
	CreateTable(Fundamental_create_table.c_str(), db);

	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	const char* sql = "INSERT INTO Fundamental VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
	int N = funda_vec.size();

	string s;
	const char* ticker;

	for (int i = 0; i < N; i++)
	{
		s = funda_vec[i].GetSymbol();
		ticker = s.c_str();

		sqlite3_bind_text(stmt, 1, ticker, -1, SQLITE_STATIC);
		sqlite3_bind_double(stmt, 2, funda_vec[i].GetDivYield());
		sqlite3_bind_double(stmt, 3, funda_vec[i].GetBeta());
		sqlite3_bind_double(stmt, 4, funda_vec[i].GetHigh52());
		sqlite3_bind_double(stmt, 5, funda_vec[i].GetLow52());
		sqlite3_bind_double(stmt, 6, funda_vec[i].GetMA50());
		sqlite3_bind_double(stmt, 7, funda_vec[i].GetMA200());
		sqlite3_bind_int64(stmt, 8, funda_vec[i].GetMktcap());
		sqlite3_bind_double(stmt, 9, funda_vec[i].GetEPS());
		sqlite3_bind_double(stmt, 10, funda_vec[i].GetShortRatio());
		//count++;

		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);
}

void CalculateRet_DB(sqlite3* db)
{
	string table_name = "test";
	string stockDB_create_table = "CREATE TABLE " + table_name
		+ "(symbol CHAR(20) NOT NULL, "
		+ "date CHAR(20) NOT NULL, "
		+ "open REAL NOT NULL, "
		+ "high REAL NOT NULL, "
		+ "low REAL NOT NULL, "
		+ "close REAL NOT NULL, "
		+ "adjusted_close REAL NOT NULL, "
		+ "volume INT NOT NULL, "
		+ "DailyRet REAL NOT NULL, "
		+ "PRIMARY KEY(symbol, date)) ";
	CreateTable(stockDB_create_table.c_str(), db);

	char* error = NULL;
	char** results = NULL;
	int rows, columns;

	string tmp = "SELECT * FROM Prices;";
	const char* sql_select = tmp.c_str();
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);

	// Extract Table
	string symbol1, ssymbol;
	//vector<double> ret_vec;	ret_vec.push_back(0.0);		// First row -> 0.0
	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	const char* sql = "INSERT INTO test VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)";
	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);

	// First row's Ret == 0.0
	ssymbol = results[columns];
	string ddate = results[columns + 1];
	double ret = 0.0;
	sqlite3_bind_text(stmt, 1, ssymbol.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, ddate.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_double(stmt, 3, stod(results[columns + 2]));
	sqlite3_bind_double(stmt, 4, stod(results[columns + 3]));
	sqlite3_bind_double(stmt, 5, stod(results[columns + 4]));
	sqlite3_bind_double(stmt, 6, stod(results[columns + 5]));
	sqlite3_bind_double(stmt, 7, stod(results[columns + 6]));
	sqlite3_bind_int64(stmt, 8, stod(results[columns + 7]));
	sqlite3_bind_double(stmt, 9, 0.0);
	sqlite3_step(stmt);
	sqlite3_reset(stmt);

	for (int rowCtr = 2; rowCtr <= rows; ++rowCtr)
	{
		int CellPosition = rowCtr * columns;
		symbol1 = results[(rowCtr - 1) * columns];
		ssymbol = results[rowCtr * columns];
		ddate = results[rowCtr * columns + 1];
		if (symbol1 == ssymbol)
		{
			ret = stod(results[CellPosition + 6]) / stod(results[(rowCtr - 1) * columns + 6]) - 1.0;
		}
		else
			ret = 0.0;
		sqlite3_bind_text(stmt, 1, ssymbol.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, ddate.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_double(stmt, 3, stod(results[CellPosition + 2]));
		sqlite3_bind_double(stmt, 4, stod(results[CellPosition + 3]));
		sqlite3_bind_double(stmt, 5, stod(results[CellPosition + 4]));
		sqlite3_bind_double(stmt, 6, stod(results[CellPosition + 5]));
		sqlite3_bind_double(stmt, 7, stod(results[CellPosition + 6]));
		sqlite3_bind_int64(stmt, 8, stod(results[CellPosition + 7]));
		sqlite3_bind_double(stmt, 9, ret);
		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);
	sqlite3_free_table(results);

	tmp = "DROP TABLE Prices;";
	sqlite3_exec(db, tmp.c_str(), NULL, NULL, &error);
	tmp = "ALTER TABLE test RENAME TO Prices;";
	sqlite3_exec(db, tmp.c_str(), NULL, NULL, &error);
}

void CountNums(sqlite3* db, string dateBT, string datePB)
{
	char* error = NULL;
	char sql[512];
	// Collect Num of Data before 2019-01-01
	sprintf_s(sql, "CREATE VIEW IF NOT EXISTS TP1 AS\
		SELECT symbol, count(*) as count\
		FROM Prices\
		WHERE Prices.date < \"%s\"\
		GROUP BY symbol;\
		UPDATE SP500 SET numsBT = \
		(SELECT count FROM TP1\
		WHERE TP1.symbol = SP500.symbol);\
		DROP VIEW TP1;", dateBT.c_str());
	sqlite3_exec(db, sql, NULL, NULL, &error);
	/*string s = "CREATE VIEW IF NOT EXISTS TP1 AS\
		SELECT symbol, count(*) as count\
		FROM Prices\
		WHERE Prices.date < \"2020-01-01\"\
		GROUP BY symbol;\
		ALTER TABLE SP500 ADD COLUMN numsBT;\
		UPDATE SP500 SET numsBT = \
		(SELECT count FROM TP1\
		WHERE TP1.symbol = SP500.symbol);\
		DROP VIEW TP1;";*/
	
	// Collect Num of Data before 2019-01-01
	sprintf_s(sql, "CREATE VIEW IF NOT EXISTS TP1 AS\
		SELECT symbol, count(*) as count\
		FROM Prices\
		WHERE Prices.date < \"%s\"\
		GROUP BY symbol;\
		UPDATE SP500 SET numsPB = \
		(SELECT count FROM TP1\
		WHERE TP1.symbol = SP500.symbol);\
		DROP VIEW TP1;", datePB.c_str());
	sqlite3_exec(db, sql, NULL, NULL, &error);
	/*s = "CREATE VIEW IF NOT EXISTS TP1 AS\
		SELECT symbol, count(*) as count\
		FROM Prices\
		WHERE Prices.date <= \"2020-07-20\"\
		GROUP BY symbol;\
		ALTER TABLE SP500 ADD COLUMN numsPB;\
		UPDATE SP500 SET numsPB = \
		(SELECT count FROM TP1\
		WHERE TP1.symbol = SP500.symbol);\
		DROP VIEW TP1;";*/
}

void PopulateSP500Clean(sqlite3* db)
{
	char* error = NULL;
	string tmp = "CREATE TABLE IF NOT EXISTS tmp1\
		(symbol CHAR(20) NOT NULL, \
			sector CHAR(20) NOT NULL, \
			VarBT REAL NOT NULL, \
			AvgRetBT REAL NOT NULL);\
			INSERT INTO tmp1\
            SELECT c.symbol as symbol, c.sector as sector,\
            Avg(a.DailyRet * a.DailyRet) - Avg(a.DailyRet) * Avg(a.DailyRet) as VarBT,\
            Avg(a.DailyRet) as AvgRetBT\
            FROM SP500 c, Prices a\
            WHERE c.symbol = a.symbol\
            AND a.date < '2020-01-01'\
            AND c.numsBT = 2516\
            GROUP BY a.symbol\
            ORDER BY a.symbol;";
	sqlite3_exec(db, tmp.c_str(), NULL, NULL, &error);

	tmp = "CREATE TABLE IF NOT EXISTS tmp2\
			(symbol CHAR(20) NOT NULL,\
			VarPB REAL NOT NULL,\
			AvgRetPB REAL NOT NULL);\
			INSERT INTO tmp2\
            SELECT c.symbol as symbol,\
            Avg(a.DailyRet * a.DailyRet) - Avg(a.DailyRet) * Avg(a.DailyRet) as VarPB,\
            Avg(a.DailyRet) as AvgRetPB\
            FROM SP500 c, Prices a\
            WHERE c.symbol = a.symbol\
            AND a.date < '2020-07-01'\
            AND c.numsPB = 2641\
            GROUP BY a.symbol\
            ORDER BY a.symbol;";
	sqlite3_exec(db, tmp.c_str(), NULL, NULL, &error);

	tmp = "DROP TABLE IF EXISTS SP500Clean;\
			CREATE TABLE IF NOT EXISTS SP500Clean(symbol CHAR(20) NOT NULL,\
			 sector CHAR(20) NOT NULL,\
			VarBT REAL NOT NULL,\
			VarPB REAL NOT NULL,\
			AvgRetBT REAL NOT NULL,\
			AvgRetPB REAL NOT NULL);\
			INSERT INTO SP500Clean\
            SELECT a.symbol as symbol,\
            a.sector as sector,\
            a.VarBT as VarBT,\
            b.VarPB as VarPB,\
            a.AvgRetBT as AvgRetBT,\
            b.AvgRetPB as AvgRetPB\
            FROM tmp1 a, tmp2 b\
            WHERE a.symbol = b.symbol;";
	sqlite3_exec(db, tmp.c_str(), NULL, NULL, &error);

	tmp = "DROP TABLE IF EXISTS tmp1;\
			DROP TABLE IF EXISTS tmp2;";
	sqlite3_exec(db, tmp.c_str(), NULL, NULL, &error);
}

// DailyRet From 2010-01-02 - 2018-12-31
void BacktestVol(sqlite3* db)
{
	char* error = NULL;
	// Calculate Variance of Daily Return
	// Data Complete Nums = 2264 & Date before 2019-01-01
	string s = "CREATE TABLE IF NOT EXISTS test AS\
					SELECT SP500.symbol, SP500.name, SP500.sector, Avg(Prices.DailyRet * Prices.DailyRet) - Avg(Prices.DailyRet) * Avg(Prices.DailyRet) AS VarBT\
					FROM SP500, Prices\
					WHERE SP500.symbol = Prices.symbol\
					AND SP500.numsBT = 2264\
					AND Prices.date < \"2019-01-01\"\
					GROUP BY Prices.symbol\
					ORDER BY Prices.symbol;";
	sqlite3_exec(db, s.c_str(), NULL, NULL, &error);

	// Set PRIMARY KEY
	s = "CREATE TABLE IF NOT EXISTS SP500Clean(symbol CHAR(20) NOT NULL PRIMARY KEY,\
		name CHAR(20) NOT NULL,\
		sector CHAR(20) NOT NULL,\
		VarBT REAL NOT NULL);\
		INSERT INTO SP500Clean\
		SELECT * FROM test;\
		DROP TABLE test;";
	sqlite3_exec(db, s.c_str(), NULL, NULL, &error);
}

// DailyRet From 2010-01-02 - 2019-12-31
void ProbationVol(sqlite3* db)
{
	char* error = NULL;
	char** results = NULL;
	int rows, columns;

	// Calculate Variance of Daily Return
	string s = "CREATE VIEW IF NOT EXISTS TP AS\
					SELECT SP500Clean.symbol, Avg(Prices.DailyRet * Prices.DailyRet) - Avg(Prices.DailyRet) * Avg(Prices.DailyRet) AS VarPB\
					FROM SP500Clean, Prices\
					WHERE SP500Clean.symbol = Prices.symbol\
					AND Prices.date < \"2020-01-01\"\
					GROUP BY Prices.symbol\
					ORDER BY Prices.symbol;";
	sqlite3_exec(db, s.c_str(), NULL, NULL, &error);

	s = "ALTER TABLE SP500Clean ADD COLUMN VarPB;\
		UPDATE SP500Clean SET VarPB = \
		(SELECT TP.VarPB FROM TP, SP500Clean\
		WHERE TP.symbol = SP500Clean.symbol);";
	/*s = "UPDATE SP500Clean SET VarPB = \
		(SELECT TP.VarPB FROM TP, SP500Clean\
		WHERE TP.symbol = SP500Clean.symbol);\
		DROP VIEW TP;";*/
	sqlite3_exec(db, s.c_str(), NULL, NULL, &error);
}

int TestFundamental(string symbol)
{
	int64_t mktcap = 0;
	double div_yield = 0.0, beta = 0.0, High52 = 0.0, Low52 = 0.0;
	double MA50 = 0.0, MA200 = 0.0, EPS = 0.0, short_ratio = 0.0;

	string stock_url_common = "https://eodhistoricaldata.com/api/fundamentals/";
	string api_token = "5ba84ea974ab42.45160048";
	string stockDB_data_request = stock_url_common + symbol + ".US?api_token=" + api_token;

	Json::Value stockDB_root;   // will contains the root value after parsing.
	if (RetrieveMarketData(stockDB_data_request, stockDB_root) == -1)
		return -1;

	int count = 0;
	for (Json::Value::const_iterator itr = stockDB_root.begin(); itr != stockDB_root.end(); itr++)
	{
		count++;
		cout << *itr << endl;
		cout << count << "---------------------------------------------------------" << endl;

		//for (Json::Value::const_iterator inner = (*itr).begin(); inner != (*itr).end(); inner++)
		//{
		//	if (inner.key().asString() == "MarketCapitalization")
		//		mktcap = inner->asInt64();
		//	else if (inner.key().asString() == "EarningsShare")
		//		EPS = inner->asDouble();
		//	else if (inner.key().asString() == "DividendYield")
		//		div_yield = inner->asDouble();
		//	else if (inner.key().asString() == "200DayMA")
		//		MA200 = inner->asDouble();
		//	else if (inner.key().asString() == "50DayMA")
		//		MA50 = inner->asDouble();
		//	else if (inner.key().asString() == "Beta")
		//		beta = inner->asDouble();
		//	else if (inner.key().asString() == "52WeekHigh")
		//		High52 = inner->asDouble();
		//	else if (inner.key().asString() == "52WeekLow")
		//		Low52 = inner->asDouble();
		//	else if (inner.key().asString() == "ShortRatio")
		//		short_ratio = inner->asDouble();
		//}
		//Fundamental funda(symbol, div_yield, beta, High52, Low52, MA50, MA200, mktcap, EPS, short_ratio);
		////funda_vec.push_back(funda);
		//cout << funda << endl;
	}
}

void ProcessSPY(sqlite3 *db, string start, string end)
{
	// First, Download and Parse SPY Data into "trade_vec"
	vector<string> SPY;
	SPY.push_back("SPY");
	vector<Trade_New> trade_vec(2663);
	DownloadAndParse(trade_vec, SPY, 0, 1, start, end, 0);

	char* error = NULL;
	string tmp = "DROP TABLE IF EXISTS SPY;";
	sqlite3_exec(db, tmp.c_str(), NULL, NULL, &error);

	string table_name = "SPY";
	tmp = "CREATE TABLE " + table_name
		+ "(symbol CHAR(20) NOT NULL, "
		+ "date CHAR(20) NOT NULL, "
		+ "open REAL NOT NULL, "
		+ "high REAL NOT NULL, "
		+ "low REAL NOT NULL, "
		+ "close REAL NOT NULL, "
		+ "adjusted_close REAL NOT NULL, "
		+ "volume INT NOT NULL, "
		+ "DailyRet REAL NOT NULL, "
		+ "PRIMARY KEY(symbol, date));";
	sqlite3_exec(db, tmp.c_str(), NULL, NULL, &error);

	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	const char* sql = "INSERT INTO SPY VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)";
	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
	long N1 = trade_vec.size();

	string d, s;
	const char* date, * symbol;
	for (long i = 0; i < N1; i++)
	{
		d = trade_vec[i].get_date(); date = d.c_str();
		s = trade_vec[i].get_symbol(); symbol = s.c_str();
		double DailyRet = 0.0;

		if (i > 0)
			DailyRet = trade_vec[i].get_adjclose() / trade_vec[i-1].get_adjclose() - 1.0;

		sqlite3_bind_text(stmt, 1, symbol, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, date, -1, SQLITE_STATIC);
		sqlite3_bind_double(stmt, 3, trade_vec[i].get_open());
		sqlite3_bind_double(stmt, 4, trade_vec[i].get_high());
		sqlite3_bind_double(stmt, 5, trade_vec[i].get_low());
		sqlite3_bind_double(stmt, 6, trade_vec[i].get_close());
		sqlite3_bind_double(stmt, 7, trade_vec[i].get_adjclose());
		sqlite3_bind_int64(stmt, 8, trade_vec[i].get_volume());
		sqlite3_bind_double(stmt, 9, DailyRet);

		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);
}


#endif // !Table_h
