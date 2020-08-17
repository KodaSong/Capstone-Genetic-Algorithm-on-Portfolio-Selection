/*
 marketdata.h
 */

#pragma once
#ifndef marketdata_h
#define marketdata_h

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iomanip>

#include "json/json.h"
#include "curl/curl.h"

#include "database.h"
using namespace std;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
int RetrieveMarketData(string url_request, Json::Value& root);

#endif // !marketdata_h
