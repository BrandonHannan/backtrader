#include "StringHelper.h"

string combineSideBySide(const string& leftBlock, const string& rightBlock){
    stringstream ssLeft(leftBlock);
    stringstream ssRight(rightBlock);
    string lineLeft;
    string lineRight;
    string result = "";

    // Read both streams line by line
    while (getline(ssLeft, lineLeft)) {
        // Try to get a corresponding line from the right block
        if (getline(ssRight, lineRight)) {
            // Combine: Left Line + Separator + Right Line + Newline
            result += lineLeft + "   |   " + lineRight + "\n";
        } else {
            // If right block runs out of lines, just print the left one
            result += lineLeft + "\n";
        }
    }

    return result;
}

string formatDoubleStat(const string &titlePrefix, const double &value, const int &maxLength){
    string result = "";
    string valString = format("{:.2f}", value);

    result.append(titlePrefix);
    result.append(": ");
    result.append(valString);

    int stringLength = result.size();
    for (int i = stringLength; i <= maxLength; i++){
        result.append(" ");
    }
    result.append("\n");

    return result;
}

string formatBoolStat(const string &titlePrefix, const bool &value, const int &maxLength){
    string result = "";
    string valString = value ? "True" : "False";

    result.append(titlePrefix);
    result.append(": ");
    result.append(valString);

    int stringLength = result.size();
    for (int i = stringLength; i <= maxLength; i++){
        result.append(" ");
    }
    result.append("\n");

    return result;
}