#ifndef Pass1_H
#define Pass1_H
#include <bits/stdc++.h>
using namespace std;
class Pass1
{
    public:
        Pass1(string);
        unordered_map<string,vector<string>> getOPCode();
        unordered_map<string,vector<string>> getSYMTAB();
        vector<string> getCodeToPrint();
        vector<string> getLabels();
        vector<string> getLitLabels();
        string getLOCCTR();
        bool getErrorFound();
        unordered_map<string,vector<string>> getLitTab();
    private:
        unordered_map<string,vector<string>> OPCode;
        unordered_map<string,vector<string>> SYMTAB;
        vector<string> linesOfCode;
        vector<string> codeToPrint;
        vector<string> labels;
        vector<string> litLabels;
        string LOCCTR;
        string syntaxError;
        string labelError;
        string operationError;
        string operandError;
        string expType;
        string prevLoc;
        unsigned int counter = 0;
        bool errorFound = false;
        bool flag = false;
        bool ltorgFlag = false;
        unordered_map<string,vector<string>> litTab;
        void initialize(string);
        void readOPCode();
        void readCodeFile(string);
        vector<string> readLine();
        bool modifyLOCCTR (string, string, string);
        bool insertSymbol(string, string, string);
        bool searchInDirectives(string);
        string calExpression(string);
        void printListFile();
        void checkLiteral(string);
        void writeLit();
        string decToHex (int);
        int hexToDec (string);
        string addZeros(string);
        string addSpaces(string);
        string removeSpaces(string);
        string searchInRegister(string);
};
#endif // Pass1_H
