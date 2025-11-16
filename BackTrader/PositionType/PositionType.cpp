#include "PositionType.h"

PositionType::PositionType(): null(true) {}

PositionType::PositionType(string pType): positionType(pType), null(false) {
    this->DeterminePositionType();
}

// Helper function for constructor 
void PositionType::DeterminePositionType(){
    if (this->positionType != "SHORT" || this->positionType != "LONG"){
        this->setNull(true);
        this->setPositionType("");
    }
}

string PositionType::getPositiontype(){
    if (this->isNull()){
        return "";
    }
    return this->positionType;
}

void PositionType::setPositionType(string pType){
    this->positionType = pType;
    this->setNull(false);
}

bool PositionType::isNull(){
    return this->null;
}

void PositionType::setNull(bool null){
    this->null = null;
}

PositionType& PositionType::operator=(const PositionType &obj){
    if (this != &obj){
        this->setNull(obj.null);
        this->setPositionType(obj.positionType);
    }
    return *this;
}

