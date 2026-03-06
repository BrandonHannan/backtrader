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