#ifndef POSITIONTYPE_H
#define POSITIONTYPE_H

#include <iostream>
#include <string>

using namespace std;

class PositionType{
    private:
        string positionType;

        bool null;

        void DeterminePositionType();
    
    public:
        PositionType();
        PositionType(string pType);

        string getPositiontype();

        void setPositionType(string pType);

        bool isNull();

        void setNull(bool null);

        PositionType& operator=(const PositionType &obj);
};

#endif