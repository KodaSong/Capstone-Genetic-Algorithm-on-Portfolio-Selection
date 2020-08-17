/*
  database.cpp
*/

#include "database.h"
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

// Find Data Not Downloaded Yet
int Select_Left_Stock_Table(sqlite3* db, vector<string>& stock_list2)
{
	int rc = 0;
	char* error = NULL;

	char** results = NULL;
	int rows, columns;

	string tmp = "SELECT symbol FROM SP500 EXCEPT SELECT DISTINCT symbol FROM Prices;";
	const char* sql_select = tmp.c_str();
	// A result table is memory data structure created by the sqlite3_get_table() interface.
	// A result table records the complete query results from one or more queries.
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		return -1;
	}

	// Extract Table
	for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
	{
		// Determine Cell Position
		stock_list2.push_back(results[rowCtr]);
	}
	// This function properly releases the value array returned by sqlite3_get_table()
	sqlite3_free_table(results);
	return 0;
}

int Display_SP_Table(const char* sql_select, sqlite3* db)
{
	int rc = 0;
	char* error = NULL;

	// Display MyTable
	//cout << "Retrieving values in a table ..." << endl;
	char** results = NULL;
	int rows, columns;
	// A result table is memory data structure created by the sqlite3_get_table() interface.
	// A result table records the complete query results from one or more queries.
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		return -1;
	}

	// Display Table
	cout << endl;
	for (int rowCtr = 0; rowCtr <= rows; ++rowCtr)
	{
		for (int colCtr = 0; colCtr < columns; ++colCtr)
		{
			// Determine Cell Position
			int cellPosition = (rowCtr * columns) + colCtr;

			// Display Cell Value
			if (colCtr == 0)
				cout.width(15);
			else
				cout.width(40);

			cout.setf(ios::left);
			cout << results[cellPosition];
		}

		// End Line
		cout << endl;
	}
	// This function properly releases the value array returned by sqlite3_get_table()
	sqlite3_free_table(results);
	return 0;
}

int Display_Stock_Table(const char* sql_select, sqlite3* db, string start_date, string end_date)
{
	if (start_date > end_date)
	{
		cout << "Wrong Date!" << endl;
		return 0;
	}

	int rc = 0;
	char* error = NULL;

	// Display MyTable
	//cout << "Retrieving values in a table ..." << endl;
	char** results = NULL;
	int rows, columns;
	// A result table is memory data structure created by the sqlite3_get_table() interface.
	// A result table records the complete query results from one or more queries.
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		return -1;
	}

	// Display Header
	for (int colCtr = 0; colCtr < columns; ++colCtr)
	{
		if (colCtr == 0)
			cout.width(10);
		else if (colCtr == 6)
			cout.width(20);
		else
			cout.width(15);
		cout.setf(ios::left);
		cout << results[colCtr];
	}
	cout << endl;

	// Display Table
	for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
	{
		int cellPosition = (rowCtr * columns);// +colCtr;

		//int id = stoi(results[cellPosition]);
		string symbol = results[cellPosition];
		string date = results[cellPosition + 1];
		float open = stof(results[cellPosition + 2]);
		float high = stof(results[cellPosition + 3]);
		float low = stof(results[cellPosition + 4]);
		float close = stof(results[cellPosition + 5]);
		float adj_close = stof(results[cellPosition + 6]);
		long volume = stol(results[cellPosition + 7]);

		if (date >= start_date && date <= end_date)
		{
			cout << setiosflags(ios::fixed) << setprecision(2);
			cout << setw(10) << left << symbol << setw(15) << left << date
				<< setw(15) << left << open << setw(15) << left << high
				<< setw(15) << left << low << setw(15) << left << close
				<< setw(20) << left << adj_close << setw(15) << left << volume
				<< endl;
		}

	}
	// This function properly releases the value array returned by sqlite3_get_table()
	sqlite3_free_table(results);
	return 0;
}


int OpenDatabase(const char* name, sqlite3*& db)
{
	int rc = 0;
	// Open Database
	cout << "Opening database: " << name << endl;
	rc = sqlite3_open(name, &db);
	if (rc)
	{
		cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << endl;
		sqlite3_close(db);
		return -1;
	}
	cout << "Opened database: " << name << endl;
	return 0;

}

void CloseDatabase(sqlite3* db)
{
	cout << "Closing a database ..." << endl;
	sqlite3_close(db);
	cout << "Closed a database" << endl << endl;
}

int DropTable(const char* sql_drop_table, sqlite3* db)
{
	// Drop the table if exists
	if (sqlite3_exec(db, sql_drop_table, 0, 0, 0) != SQLITE_OK) { // or == -- same effect
		std::cout << "SQLite can't drop sessions table" << std::endl;
		sqlite3_close(db);
		return -1;
	}
	return 0;
}

int CreateTable(const char* sql_create_table, sqlite3* db)
{
	int rc = 0;
	char* error = NULL;
	// Create the table
	//cout << "Creating a table..." << endl;
	rc = sqlite3_exec(db, sql_create_table, NULL, NULL, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		return -1;
	}
	//cout << "Created a table." << endl;
	return 0;
}

int InsertTable(const char* sql_insert, sqlite3* db)
{
	int rc = 0;
	char* error = NULL;
	// Execute SQL
	//cout << "Inserting a value into a table ..." << endl;
	//sqlite3_exec(db, "begin;", NULL, NULL, &error);
	sqlite3_exec(db, "PRAGMA synchronous = OFF; ", NULL, NULL, &error);
	rc = sqlite3_exec(db, sql_insert, NULL, NULL, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		return -1;
	}
	//cout << "Inserted a value into the table." << endl;
	return 0;
}

int DisplayTable(const char* sql_select, sqlite3* db)
{
	int rc = 0;
	char* error = NULL;

	// Display MyTable
	cout << "Retrieving values in a table ..." << endl;
	char** results = NULL;
	int rows, columns;
	// A result table is memory data structure created by the sqlite3_get_table() interface.
	// A result table records the complete query results from one or more queries.
	sqlite3_get_table(db, sql_select, &results, &rows, &columns, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
		sqlite3_free(error);
		return -1;
	}

	// Display Table
	for (int rowCtr = 0; rowCtr <= rows; ++rowCtr)
	{
		for (int colCtr = 0; colCtr < columns; ++colCtr)
		{
			// Determine Cell Position
			int cellPosition = (rowCtr * columns) + colCtr;

			// Display Cell Value
			cout.width(12);
			cout.setf(ios::left);
			cout << results[cellPosition] << " ";
		}

		// End Line
		cout << endl;

		// Display Separator For Header
		if (0 == rowCtr)
		{
			for (int colCtr = 0; colCtr < columns; ++colCtr)
			{
				cout.width(12);
				cout.setf(ios::left);
				cout << "~~~~~~~~~~~~ ";
			}
			cout << endl;
		}
	}
	// This function properly releases the value array returned by sqlite3_get_table()
	sqlite3_free_table(results);
	return 0;
}