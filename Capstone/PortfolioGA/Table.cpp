/*
 * @Author: Koda Song
 * @Date: 2020-06-07 11:50:30
 * @LastEditors: Koda Song
 * @LastEditTime: 2020-08-05 10:12:32
 * @Description: Table.cpp
 */ 
#include "Table.h"

 // An integration of functions
 // Download -> Parse -> Store
int Do_First_Time(vector<Trade_New>& trade_vec1, vector<string>& stock_list1, sqlite3* stockDB, string stock_start_date, string stock_end_date)
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
		+ "PRIMARY KEY(symbol, date), "
		+ "FOREIGN KEY(symbol) REFERENCES SP500(symbol));";

	if (CreateTable(stockDB_create_table.c_str(), stockDB) == -1)
		return -1;

	// Download -> Parse -> Store

	/*cout << endl << "Prepare to Download Prices Data" << endl;	Sleep(3000);
	MultiThread_Download(root_vec1, stock_list1, stock_start_date, stock_end_date);

	cout << endl << "Prepare to Parse Json" << endl;	Sleep(5000);
	Parse_JSON(trade_vec1, root_vec1, stock_list1);*/
	/*Multi_Thread(trade_vec1, root_vec1, stock_list1, stock_start_date, stock_end_date);*/
	Multi_Thread(trade_vec1, stock_list1, stock_start_date, stock_end_date);

	cout << endl << "Prepare to Store Data" << endl;	Sleep(5000);
	int count = 0;
	Store_Data(trade_vec1, stockDB);

	cout << "Prepare to Free Memory" << endl;	Sleep(1000);
	//vector<Json::Value>().swap(root_vec1);	// Free memory of root_vec1
	vector<Trade_New>().swap(trade_vec1);	// Free memory of trade_vec1

	return count;
}

// Similar with "Do_First_Time"
// Process left stocks
void Test_Left_Stock(sqlite3* stockDB, string stock_start_date, string stock_end_date)
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
		MultiThread_Download(root_vec2, stock_list2, stock_start_date, stock_end_date);
		Parse_JSON(trade_vec2, root_vec2, stock_list2);
		/*Multi_Thread(trade_vec2, root_vec2, stock_list2, stock_start_date, stock_end_date);*/
		//Multi_Thread(trade_vec2, stock_list2, stock_start_date, stock_end_date);

		/*count = Store_Data2(trade_vec2, stockDB, count);*/
		Store_Data(trade_vec2, stockDB);

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

 // Download Data from "Start" till "End" in "Stock_List" & Parse into vector of objects
int Download_and_Parse(vector<Trade_New>& trade_vec, vector<string>& stock_list, 
	int start_index, int end_index, string stock_start_date, string stock_end_date, int count)
{
	long volume;
	string date, symbol;
	float open, high, low, close, adjusted_close;

	//int count = 0;
	for (int i = start_index; i != end_index; i++)
	{
		symbol = stock_list[i];
		if (symbol == "BF.B")
			symbol = "BFB";
		if (symbol == "BRK.B")
			symbol = "BRKB";
		string stock_url_common = "https://eodhistoricaldata.com/api/eod/";
		string api_token = "5ba84ea974ab42.45160048";
		string stockDB_data_request = stock_url_common + symbol + ".US?" +
			"from=" + stock_start_date + "&to=" + stock_end_date + "&api_token=" + api_token + "&period=d&fmt=json";

		Json::Value stockDB_root;   // will contains the root value after parsing.
		if (RetrieveMarketData(stockDB_data_request, stockDB_root) == -1)
			return -1;
		//root_vec[i] = stockDB_root;

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
			//trade_vec.push_back(aTrade);
			trade_vec[count++] = aTrade;
		}
		//root_vec[i] = NULL;		// Save memory!
	}
}

int Multi_Thread(vector<Trade_New>& trade_vec, vector<string>& stock_list, 
	string stock_start_date, string stock_end_date)
{
	int size = stock_list.size() / 5;
	int a = 0, b = a + size, c = b + size, d = c + size, e = d + size, f = stock_list.size();

	//vector<Trade_New> trade_vec1, trade_vec2, trade_vec3, trade_vec4, trade_vec5;

	thread t6(Download_and_Parse, ref(trade_vec), ref(stock_list), a, b, stock_start_date, stock_end_date, 0);
	thread t7(Download_and_Parse, ref(trade_vec), ref(stock_list), b, c, stock_start_date, stock_end_date, 400000);
	thread t8(Download_and_Parse, ref(trade_vec), ref(stock_list), c, d, stock_start_date, stock_end_date, 800000);
	thread t9(Download_and_Parse, ref(trade_vec), ref(stock_list), d, e, stock_start_date, stock_end_date, 1200000);
	thread t10(Download_and_Parse, ref(trade_vec), ref(stock_list), e, f, stock_start_date, stock_end_date, 1600000);

	t6.join();
	t7.join();
	t8.join();
	t9.join();
	t10.join();

	return 0;
}

// Separate Download & Parse
// Download Prices Data for stocks from "Start" till "End" in "Stock_List"
// Store them in "Root_Vec"
int Download_JSON(vector<Json::Value>& root_vec, vector<string>& stock_list, int start_index, int end_index, string stock_start_date, string stock_end_date)
{
	string symbol;
	for (int i = start_index; i != end_index; i++)
	{
		symbol = stock_list[i];
		if (symbol == "BF.B")
			symbol = "BFB";
		if (symbol == "BRK.B")
			symbol = "BRKB";

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

void MultiThread_Download(vector<Json::Value>& root_vec, vector<string>& stock_list, string stock_start_date, string stock_end_date)
{
	int size = stock_list.size() / 5;
	int a = 0, b = a + size, c = b + size, d = c + size, e = d + size, f = stock_list.size();

	thread t1(Download_JSON, ref(root_vec), ref(stock_list), a, b, stock_start_date, stock_end_date);
	thread t2(Download_JSON, ref(root_vec), ref(stock_list), b, c, stock_start_date, stock_end_date);
	thread t3(Download_JSON, ref(root_vec), ref(stock_list), c, d, stock_start_date, stock_end_date);
	thread t4(Download_JSON, ref(root_vec), ref(stock_list), d, e, stock_start_date, stock_end_date);
	thread t5(Download_JSON, ref(root_vec), ref(stock_list), e, f, stock_start_date, stock_end_date);

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
}

// Parse "Root_Vec" & Store in "Trade_Vec"	object(symbol, date, high...)
void Parse_JSON(vector<Trade_New>& trade_vec, vector<Json::Value>& root_vec, vector<string>& stock_list)
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

// Store Prices Data into dB
void Store_Data(vector<Trade_New>& trade_vec1, sqlite3* db)
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

		//sqlite3_bind_int(stmt, 1, count++);
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


// Parse Data of single stock in Root and Store them in Table "Prices"
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

// Download + Parse + Store data of single stock
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


// No use anymore
void Store_Data1(vector<Trade_New>& trade_vec, sqlite3* db)
{
	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	const char* sql = "INSERT INTO Prices VALUES(?, ?, ?, ?, ?, ?, ?, ?)";
	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
	long N = trade_vec.size();

	for (long i = 0; i < N; i++)
	{
		string d = trade_vec[i].get_date();
		string s = trade_vec[i].get_symbol();
		const char* date = d.c_str();
		const char* symbol = s.c_str();

		if (d.size() == 0)
			break;

		//sqlite3_bind_int(stmt, 1, int(i + 1));
		sqlite3_bind_text(stmt, 1, symbol, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, date, -1, SQLITE_STATIC);
		sqlite3_bind_double(stmt, 3, trade_vec[i].get_open());
		sqlite3_bind_double(stmt, 4, trade_vec[i].get_high());
		sqlite3_bind_double(stmt, 5, trade_vec[i].get_low());
		sqlite3_bind_double(stmt, 6, trade_vec[i].get_close());
		sqlite3_bind_double(stmt, 7, trade_vec[i].get_adjclose());
		sqlite3_bind_int64(stmt, 8, trade_vec[i].get_volume());
		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);
}

// No use anymore
// Store Prices Data into dB
// Return the last index in dB
// Used to store left stocks data (Almost same as 1)
int Store_Data2(vector<Trade_New>& trade_vec2, sqlite3* db, int count)
{
	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	const char* sql = "INSERT INTO Prices VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)";
	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
	long N2 = trade_vec2.size();

	string d, s;
	const char* date, * symbol;

	for (long i = 0; i < N2; i++)
	{
		d = trade_vec2[i].get_date(); date = d.c_str();
		s = trade_vec2[i].get_symbol(); symbol = s.c_str();

		if (d.size() == 0)
			continue;
		sqlite3_bind_int(stmt, 1, count++);
		sqlite3_bind_text(stmt, 2, symbol, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, date, -1, SQLITE_STATIC);
		sqlite3_bind_double(stmt, 4, trade_vec2[i].get_open());
		sqlite3_bind_double(stmt, 5, trade_vec2[i].get_high());
		sqlite3_bind_double(stmt, 6, trade_vec2[i].get_low());
		sqlite3_bind_double(stmt, 7, trade_vec2[i].get_close());
		sqlite3_bind_double(stmt, 8, trade_vec2[i].get_adjclose());
		sqlite3_bind_int64(stmt, 9, trade_vec2[i].get_volume());
		//count++;

		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}

	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);

	return count;
}


int Process_Fundamental(sqlite3* db)
{
	// Get symbols from Table SP500
	char* error = NULL;
	char** results = NULL;
	int rows, columns;

	string tmp = "SELECT symbol FROM SP500;";
	const char* sql_select = tmp.c_str();
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);

	// Extract Table
	vector<string> stock_list;
	string symbol;
	for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
	{
		symbol = results[rowCtr];
		// Determine Cell Position
		if (symbol == "BF.B")
			symbol = "BFB";
		if (symbol == "BRK.B")
			symbol = "BRKB";
		cout << rowCtr << " " << symbol << endl;
		stock_list.push_back(symbol);
	}
	sqlite3_free_table(results);

	// Download data and store them in objects
	
	int64_t mktcap;
	double div_yield, beta, High52, Low52, MA50, MA200, EPS, short_ratio;
	vector<Fundamental> funda_vec;

	for (int i = 0; i < stock_list.size(); i++)
	{
		symbol = stock_list[i];
		string stock_url_common = "https://eodhistoricaldata.com/api/fundamentals/";
		string api_token = "5ba84ea974ab42.45160048";
		/*string stockDB_data_request = stock_url_common + symbol + ".US?" +
			"from=" + stock_start_date + "&to=" + stock_end_date + "&api_token=" + api_token + "&period=d&fmt=json";*/
		string stockDB_data_request = stock_url_common + symbol + ".US?api_token=" + api_token;

		Json::Value stockDB_root;   // will contains the root value after parsing.
		if (RetrieveMarketData(stockDB_data_request, stockDB_root) == -1)
		{
			//cout << endl << "-----------" << symbol << "-----------" << endl;
			return -1;
		}
			
		//cout << endl << symbol << endl << stockDB_root.size() << endl;
		//cout << stockDB_root << endl;
		int count = 0;
		for (Json::Value::const_iterator itr = stockDB_root.begin(); itr != stockDB_root.end(); itr++)
		{
			count++;
			if (count == 5)
			{
				mktcap = (*itr)["MarketCapitalization"].asInt64();
				EPS = (*itr)["EarningsShare"].asDouble();
				div_yield = (*itr)["DividendYield"].asDouble();
			}
			else if (count == 8)
			{
				MA200 = (*itr)["200DayMA"].asDouble();
				MA50 = (*itr)["50DayMA"].asDouble();
				beta = (*itr)["Beta"].asDouble();
				High52 = (*itr)["52WeekHigh"].asDouble();
				Low52 = (*itr)["52WeekLow"].asDouble();
				short_ratio = (*itr)["ShortRatio"].asDouble();

				Fundamental funda(symbol, div_yield, beta, High52, Low52, MA50, MA200, mktcap, EPS, short_ratio);
				//cout << funda << endl;
				funda_vec.push_back(funda);
			}
		}
	}
	// Insert Into Table Fundamental
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
		+ "PRIMARY KEY(symbol), "
		+ "FOREIGN KEY(symbol) REFERENCES SP500(symbol));";
	if (CreateTable(Fundamental_create_table.c_str(), db) == -1)
		return -1;

	sqlite3_exec(db, "begin;", 0, 0, 0);
	sqlite3_stmt* stmt;
	const char* sql = "INSERT INTO Fundamental VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
	long N = funda_vec.size();

	string s;
	const char* date, * ticker;

	for (long i = 0; i < N; i++)
	{
		s = funda_vec[i].get_symbol(); 
		ticker = s.c_str();

		sqlite3_bind_text(stmt, 1, ticker, -1, SQLITE_STATIC);
		sqlite3_bind_double(stmt, 2, funda_vec[i].get_div());
		sqlite3_bind_double(stmt, 3, funda_vec[i].get_beta());
		sqlite3_bind_double(stmt, 4, funda_vec[i].get_High52());
		sqlite3_bind_double(stmt, 5, funda_vec[i].get_Low52());
		sqlite3_bind_double(stmt, 6, funda_vec[i].get_MA50());
		sqlite3_bind_double(stmt, 7, funda_vec[i].get_MA200());
		sqlite3_bind_int64(stmt, 8, funda_vec[i].get_mktcap());
		sqlite3_bind_double(stmt, 9, funda_vec[i].get_EPS());
		sqlite3_bind_double(stmt, 10, funda_vec[i].get_shortratio());
		//count++;

		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);
}

int Download_and_Parse_Fundamental(vector<Fundamental>& funda_vec, vector<string>& stock_list, int start_index, int end_index)
{
	string symbol;
	int64_t mktcap;
	double div_yield, beta, High52, Low52, MA50, MA200, EPS, short_ratio;
	for (int i = start_index; i != end_index; i++)
	{
		symbol = stock_list[i];
		if (symbol == "BF.B")
			symbol = "BFB";
		if (symbol == "BRK.B")
			symbol = "BRKB";

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
			if (count == 5)
			{
				mktcap = (*itr)["MarketCapitalization"].asInt64();
				EPS = (*itr)["EarningsShare"].asDouble();
				div_yield = (*itr)["DividendYield"].asDouble();
			}
			else if (count == 8)
			{
				MA200 = (*itr)["200DayMA"].asDouble();
				MA50 = (*itr)["50DayMA"].asDouble();
				beta = (*itr)["Beta"].asDouble();
				High52 = (*itr)["52WeekHigh"].asDouble();
				Low52 = (*itr)["52WeekLow"].asDouble();
				short_ratio = (*itr)["ShortRatio"].asDouble();

				Fundamental funda(symbol, div_yield, beta, High52, Low52, MA50, MA200, mktcap, EPS, short_ratio);
				//funda_vec.push_back(funda);
				funda_vec[i] = funda;
			}
		}
	}
}

void MultiThread_Download_and_Parse_Fundamental(sqlite3 *db)
{
	// Get symbols from Table SP500
	char* error = NULL;
	char** results = NULL;
	int rows, columns;

	string tmp = "SELECT symbol FROM SP500;";
	const char* sql_select = tmp.c_str();
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);

	// Extract Table
	vector<string> stock_list;
	string symbol;
	for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
	{
		symbol = results[rowCtr];
		// Determine Cell Position
		if (symbol == "BF.B")
			symbol = "BFB";
		if (symbol == "BRK.B")
			symbol = "BRKB";
		cout << rowCtr << " " << symbol << endl;
		stock_list.push_back(symbol);
	}
	sqlite3_free_table(results);


	int size = stock_list.size() / 5;
	int a = 0, b = a + size, c = b + size, d = c + size, e = d + size, f = stock_list.size();
	vector<Fundamental> funda_vec(size * 5);

	thread t1(Download_and_Parse_Fundamental, ref(funda_vec), ref(stock_list), a, b);
	thread t2(Download_and_Parse_Fundamental, ref(funda_vec), ref(stock_list), b, c);
	thread t3(Download_and_Parse_Fundamental, ref(funda_vec), ref(stock_list), c, d);
	thread t4(Download_and_Parse_Fundamental, ref(funda_vec), ref(stock_list), d, e);
	thread t5(Download_and_Parse_Fundamental, ref(funda_vec), ref(stock_list), e, f);

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();

	// Insert Into Table Fundamental
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
		+ "PRIMARY KEY(symbol), "
		+ "FOREIGN KEY(symbol) REFERENCES SP500(symbol));";
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
		s = funda_vec[i].get_symbol();
		ticker = s.c_str();

		sqlite3_bind_text(stmt, 1, ticker, -1, SQLITE_STATIC);
		sqlite3_bind_double(stmt, 2, funda_vec[i].get_div());
		sqlite3_bind_double(stmt, 3, funda_vec[i].get_beta());
		sqlite3_bind_double(stmt, 4, funda_vec[i].get_High52());
		sqlite3_bind_double(stmt, 5, funda_vec[i].get_Low52());
		sqlite3_bind_double(stmt, 6, funda_vec[i].get_MA50());
		sqlite3_bind_double(stmt, 7, funda_vec[i].get_MA200());
		sqlite3_bind_int64(stmt, 8, funda_vec[i].get_mktcap());
		sqlite3_bind_double(stmt, 9, funda_vec[i].get_EPS());
		sqlite3_bind_double(stmt, 10, funda_vec[i].get_shortratio());
		//count++;

		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db, "commit;", 0, 0, 0);
}