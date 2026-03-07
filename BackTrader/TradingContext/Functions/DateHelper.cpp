#include "DateHelper.h"

int ToJulian(int y, int m, int d){
    if (m <= 2) {
        y -= 1;
        m += 12;
    }
    int A = y / 100;
    int B = 2 - A + A / 4;
    return int(365.25 * (y + 4716)) + int(30.6001 * (m + 1)) + d + B - 1524;
}

int LengthOfTradeBetweenDates(const string &date1, const string &date2) {
    if (date1 == "" || date2 == "" || date1.length() != 10 || date2.length() != 10){
        return -1;
    }
    int length = 0;
    int beginningYear = stoi(date1.substr(0, 4));
    int beginningMonth = stoi(date1.substr(5, 7));
    int beginningDay = stoi(date1.substr(8, 10));

    int endYear = stoi(date2.substr(0, 4));
    int endMonth = stoi(date2.substr(5, 7));
    int endDay = stoi(date2.substr(8, 10));

    length = ToJulian(endYear, endMonth, endDay) - ToJulian(beginningYear, beginningMonth, beginningDay);
    return length;
}