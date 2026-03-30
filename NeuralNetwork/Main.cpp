#include <iostream>
#include <fstream>
#include "include/nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

int main(){
    json j;

    j["name"] = "Brandon";
    j["age"] = 25;
    j["isStudent"] = false;
    j["trades"] = json::array();

    for (int i = 0; i< 100; i++){
        j["trades"].push_back({i, "Brandon"});
    }

    std::ofstream file("data.json");
    file << j.dump(2);
    return 0;
}