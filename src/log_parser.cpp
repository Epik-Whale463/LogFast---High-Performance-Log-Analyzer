#include "log_parser.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void parseLogs(const string &filename) {

    ifstream logFile(filename);
    
    if(!logFile) {
        cerr << "Error opening the file: " << filename << endl;
        return;
    }

    string line;
    while( getline(logFile, line)) {
        cout << line << "\n";
    }

    logFile.close();

} 