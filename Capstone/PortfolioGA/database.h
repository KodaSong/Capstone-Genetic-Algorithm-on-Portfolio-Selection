/*
 database.h
 */


#pragma once
#ifndef database_h
#define database_h

#include <sqlite3.h>
#include <string>
#include <vector>
using namespace std;

// Find Data Not Downloaded Yet
// Difference Set between Table "SP500" & "Prices" on column "symbol"
int Select_Left_Stock_Table(sqlite3* db, vector<string>& stock_list2);

// Display SP500 List
int Display_SP_Table(const char* sql_select, sqlite3* db);
// Display Single Stock Prices from "Start" till "End"
int Display_Stock_Table(const char* sql_select, sqlite3* db, string start_date, string end_date);


int OpenDatabase(const char* name, sqlite3*& db);
void CloseDatabase(sqlite3* db);
int DropTable(const char* sql_drop_table, sqlite3* db);
int CreateTable(const char* sql_create_table, sqlite3* db);

// Slow! Not very useful!
int InsertTable(const char* sql_insert, sqlite3* db);

int DisplayTable(const char* sql_select, sqlite3* db);
#endif // !database_h