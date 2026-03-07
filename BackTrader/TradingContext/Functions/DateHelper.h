#ifndef DATEHELPER_H
#define DATEHELPER_H

#include "string"
#include "cmath"
#include "limits"

using namespace std;

int ToJulian(int y, int m, int d);

int LengthOfTradeBetweenDates(const string &date1, const string &date2);

#endif