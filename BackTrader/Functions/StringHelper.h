#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include <sstream>
#include <string>
#include <format>

using namespace std;

string combineSideBySide(const string& leftBlock, const string& rightBlock);

string formatDoubleStat(const string &titlePrefix, const double &value, const int &maxLength);

string formatBoolStat(const string &titlePrefix, const bool &value, const int &maxLength);

#endif