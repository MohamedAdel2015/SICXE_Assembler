#include <iostream>
#include <algorithm>
#include <regex>
#include <string>
#include <string.h>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "Pass1.h"

using namespace std;

unordered_map<string,vector<string>> OPTAB;
unordered_map<string,vector<string>> SYMTAB;
unordered_map<string,vector<string>> LITTAB;
vector<string> CodeToPrint;
vector<string> readListFile;
vector<string> Labels;
vector<string> LitLabels;
string LOCCTR;
string Start;
string length;
int maxLength=0;
bool errorFound, printFlags = false;
char n,i,x,b,p,e;
bool cutRecord = false;
bool modRecord = false;
bool modRecordAbs = false;
bool baseRelative = false;
string operandError;
string expType;
string base;

string binToHex(string bin);
string hexToBin(string hex);
string calculateObjectCode(vector<string> line);
string decToHex (int decimal);
int hexToDec (string hexadecimal);
string addZeros(string loc, int n);
string removeSpaces(string operands);
string searchInRegister(string r);
string calExpression(string operand);
string addingOPCode(string line,string operand);
string writeTxtRecord(string txtRecord,string lastStartOfTxtRecord);
vector<string> divideLine(string line);
string cutLabel(string name);
string getDisp(int ta, int pc);
void printListFlie();
void printObjectCode(vector<string>records);
void maxStringLength();

int main(int argc, char *argv[])
{
	char *path = argv[1];
    Pass1 pass1("code.txt");
    if(pass1.getErrorFound()){
        return 0;
    }
    OPTAB = pass1.getOPCode();
    SYMTAB = pass1.getSYMTAB();
    LITTAB = pass1.getLitTab();
    readListFile = pass1.getCodeToPrint();
    Labels = pass1.getLabels();
    LitLabels = pass1.getLitLabels();
    LOCCTR = pass1.getLOCCTR();
    errorFound = false;
    vector<string> records;
    vector<string> modificationRecords;
    string lastStartOfTxtRecord;
    string record("");
    string txtRecord("");
    maxStringLength();
    for(int j=0; j<readListFile.size(); j++){
        cutRecord = false;
        modRecord = false;
        printFlags = false;
        baseRelative = false;
        operandError = "";
        modRecordAbs = false;
        if(readListFile[j][7] == '.'){
            CodeToPrint.push_back(readListFile[j]);
        }
        else{
            vector<string> line = divideLine(readListFile[j]);
            string opCode = calculateObjectCode(line);
            CodeToPrint.push_back(addingOPCode(readListFile[j],line[3])+" "+opCode);
            if(printFlags && (operandError.length() == 0)){
                string registers = "n = ";
                registers += n;
                registers += ", i = ";
                registers += i;
                registers += ", x = ";
                registers += x;
                registers += ", b = ";
                registers += b;
                registers += ", p = ";
                registers += p;
                registers += ", e = ";
                registers += e;
                CodeToPrint.push_back(registers);
            }
            if(operandError.length() != 0){
                CodeToPrint.push_back(operandError);
                errorFound = true;
            }
            if(line[2] == "START" && !errorFound){
                lastStartOfTxtRecord = line[0];
                record = "H" + cutLabel(line[1]) + line[0] + length;
                records.push_back(record);
                record = "";
            }
            else if (line[2] == "END"&& !errorFound){
                if(txtRecord.length() != 0){
                    record = writeTxtRecord(txtRecord,lastStartOfTxtRecord);
                    records.push_back(record);
                    record = "";
                    txtRecord = "";
                }
                for(int k=0;k<modificationRecords.size();k++){
                    records.push_back(modificationRecords[k]);
                }
                record = "E"+Start;
                records.push_back(record);
                record = "";
            }
            else if(!errorFound){
                if(cutRecord && (txtRecord.length() != 0)){
                    record = writeTxtRecord(txtRecord,lastStartOfTxtRecord);
                    records.push_back(record);
                    record = "";
                    txtRecord = "";
                }
                else{
                    if((opCode.length()+txtRecord.length()) == 60){
                        txtRecord += opCode;
                        record = writeTxtRecord(txtRecord,lastStartOfTxtRecord);
                        records.push_back(record);
                        record = "";
                        txtRecord ="";
                    }
                    else if((opCode.length()+txtRecord.length()) < 60){
                        if(txtRecord.length()==0){
                            lastStartOfTxtRecord = line[0];
                        }
                        txtRecord += opCode;
                    }
                    else if((opCode.length()+txtRecord.length()) > 60){
                        record = writeTxtRecord(txtRecord,lastStartOfTxtRecord);
                        records.push_back(record);
                        record = "";
                        txtRecord = "";
                        lastStartOfTxtRecord = line[0];
                        txtRecord += opCode;
                    }
                }
                if(modRecord){
                    string mRecord("");
                    mRecord = "M" + addZeros(decToHex(hexToDec(line[0])+1),6) + "05+";
                    modificationRecords.push_back(mRecord);
                }
                if(modRecordAbs){
                    string mRecord("");
                    mRecord = "M" + addZeros(decToHex(hexToDec(line[0])+1),6) + "03-";
                    modificationRecords.push_back(mRecord);
                }
            }
        }
    }
    printListFlie();
    printObjectCode(records);
    return 0;
}

string binToHex(string bin_num){
    bitset<36> b(bin_num);
    stringstream res;
    res << hex << uppercase << b.to_ulong();
    return res.str();
}

string hexToBin(string hex_num){
    stringstream ss;
    ss << hex << hex_num;
    int dec = 0;
    ss >> dec;
    bitset<36> b(dec);
    string bin_num = b.to_string();
    int diff = 36 - hex_num.length()*4;
    bin_num = bin_num.erase(0,diff);
    return bin_num;
}

string calculateObjectCode(vector<string> line){
    string loc = line[0];
    string label = line[1];
    string operation = line[2];
    string operand = line[3];
    bool extended = false;
    if (operation == "START"){
        Start = loc;
        int len = hexToDec(LOCCTR) - hexToDec(Start);
        length = addZeros(decToHex(len), 6);
        return "";
    }
    else if (operation == "END"){
        return "";
    }
    if (label == "*" && operation[0] == '='){
        operation = operation.erase(0,1);
        regex remover("((\\w\'[^.]+\')|(\\d+))");
        smatch match;
        regex_search(operation,match,remover);
        string s = match[1];
        unordered_map<string,vector<string>>::const_iterator found2 = LITTAB.find(s);
        if (found2 != LITTAB.end()){
            string hexValue = "";
            hexValue = LITTAB[s][0];
            return hexValue;
        }
        return "";
    }
    if (operation[0] == '+'){
        extended = true;
        modRecord = true;
        operation = operation.erase(0,1);
    }
    unordered_map<string,vector<string>>::const_iterator found = OPTAB.find(operation);
    if (found != OPTAB.end()){ //found in OPTAB
        vector<string> temp = OPTAB[operation];
        string format;
        if (temp[0] == "3/4" && extended){
            format = "4";
            printFlags = true;
        }
        else if (temp[0] == "3/4" && !extended){
            format = "3";
            printFlags = true;
        }
        else{
            format = "2";
        }
        string opcode = temp[1];
        string operandsNum = temp[2];
        string operandsTypes = temp[3];
        string disp = "";
        if (format == "2"){
            operand = removeSpaces(operand);
            if ((operandsNum == "1") && (operand.length() == 1)){
                string rNum = searchInRegister(operand);
                string objectCode = opcode;
                objectCode += rNum;
                objectCode += "0";
                return objectCode;
            }
            else if ((operandsNum == "2") && (operand.length() == 3)){
                string rNum1 = "";
                rNum1 += operand[0];
                rNum1 = searchInRegister(rNum1);
                string rNum2 = "";
                rNum2 += operand[2];
                rNum2 = searchInRegister(rNum2);
                string objectCode = opcode;
                objectCode += rNum1;
                objectCode += rNum2;
                return objectCode;
            }
        }
        else if (format == "3"){
            if (operand.length() == 0){
                int op = hexToDec(opcode) + 3;
                string objectCode = decToHex(op);
                objectCode += "0000";
                objectCode = addZeros(objectCode,6);
                n = '1'; i = '1'; x = '0'; p = '0'; b = '0'; e = '0';
                return objectCode;
            }
            if (operand[0] != '='){
                operand = removeSpaces(operand);
            }
            int pc = hexToDec(loc) + 3;
            string PC = decToHex(pc);
            if (operand[0] == '@'){
                n = '1'; i = '0'; x = '0'; e = '0';
                // check operand
                operand = operand.erase(0,1);
                string TA = calExpression(operand);
                if (TA.length() == 0){
                    return "";
                }
                if (expType != "R"){
                    operandError = "*****Invalid Operand!!!!!";
                    printFlags = false;
                    return "";
                }
                int ta = hexToDec(TA);
                disp = getDisp(ta, pc);
                if (!baseRelative && operandError.size() == 0){
                    p = '1'; b = '0';
                }
                else if (baseRelative && operandError.size() == 0){
                    b = '1'; p = '0';
                }
                else{
                    printFlags = false;
                    return "";
                }
            }
            else if (operand[0] == '#'){
                n = '0'; i = '1'; x = '0'; e = '0';
                // check operand
                operand = operand.erase(0,1);
                string TA = calExpression(operand);
                if (TA.length() == 0){
                    return "";
                }
                if (expType == "R"){
                    int ta = hexToDec(TA);
                    disp = getDisp(ta, pc);
                    if (!baseRelative && operandError.size() == 0){
                        p = '1'; b = '0';
                    }
                    else if (baseRelative && operandError.size() == 0){
                        b = '1'; p = '0';
                    }
                    else{
                        printFlags = false;
                        return "";
                    }
                }
                else{
                    if (hexToDec(TA) <= 4095){
                        disp = TA;
                        disp = disp.erase(0,3);
                        p = '0'; b = '0';
                    }
                    else{
                        operandError = "*****Invalid Operand!!!!";
                        printFlags = false;
                        return "";
                    }
                }
            }
            else if ((operand[operand.length()-1] == 'X') && (operand[operand.length()-2] == ',')){
                n = '1'; i = '1'; x = '1'; e = '0';
                // check operand
                operand = operand.erase(operand.length()-2, operand.length());
                string TA = calExpression(operand);
                if (TA.length() == 0){
                    return "";
                }
                if (expType != "R"){
                    operandError = "*****Invalid Operand!!!!!";
                    printFlags = false;
                    return "";
                }
                int ta = hexToDec(TA);
                disp = getDisp(ta, pc);
                if (!baseRelative && operandError.size() == 0){
                    p = '1'; b = '0';
                }
                else if (baseRelative && operandError.size() == 0){
                    b = '1'; p = '0';
                }
                else{
                    printFlags = false;
                    return "";
                }
            }
            else {
                n = '1'; i = '1'; x = '0'; e = '0';
                string TA = calExpression(operand);
                if (TA.length() == 0){
                    return "";
                }
                int ta = hexToDec(TA);
                disp = getDisp(ta, pc);
                if (expType == "A"){
                    modRecordAbs = true;
                }
                if (!baseRelative && operandError.size() == 0){
                    p = '1'; b = '0';
                }
                else if (baseRelative && operandError.size() == 0){
                    b = '1'; p = '0';
                }
                else{
                    printFlags = false;
                    return "";
                }
            }
            string op = hexToBin(opcode);
            op[op.length()-2] = n;
            op[op.length()-1] = i;
            op += x; op += b; op += p; op += e;
            op = binToHex(op);
            op += disp;
            op = addZeros(op, 6);
            return op;
        }
        else if (format == "4"){
            if (operand.length() == 0){
                int op = hexToDec(opcode) + 3;
                string objectCode = decToHex(op);
                objectCode += "100000";
                n = '1'; i = '1'; x = '0'; p = '0'; b = '0'; e = '1';
                return objectCode;
            }
            if (operand[0] != '='){
                operand = removeSpaces(operand);
            }
            int pc = hexToDec(loc) + 4;
            string PC = decToHex(pc);
            string TA = "";
            if (operand[0] == '@'){
                n = '1'; i = '0'; x = '0'; e = '1'; p = '0'; b = '0';
                // check operand
                operand = operand.erase(0,1);
                TA = calExpression(operand);
                if (TA.length() == 0){
                    return "";
                }
                if (TA.length() < 5){
                    TA = addZeros(TA, 5);
                }
                else if (TA.length() > 5){
                    TA = TA.erase(0, TA.length()-5);
                }
                if (expType != "R"){
                    operandError = "*****Invalid Operand!!!!!";
                    printFlags = false;
                    return "";
                }
            }
            else if (operand[0] == '#'){
                n = '0'; i = '1'; x = '0'; e = '1'; p = '0'; b = '0';
                // check operand
                operand = operand.erase(0,1);
                TA = calExpression(operand);
                if (TA.length() == 0){
                    return "";
                }
                if (TA.length() < 5){
                    TA = addZeros(TA, 5);
                }
                else if (TA.length() > 5){
                    TA = TA.erase(0, TA.length()-5);
                }
            }
            else if ((operand[operand.length()-1] == 'X') && (operand[operand.length()-2] == ',')){
                n = '1'; i = '1'; x = '1'; e = '1'; p = '0'; b = '0';
                // check operand
                operand = operand.erase(operand.length()-2, operand.length());
                TA = calExpression(operand);
                if (TA.length() == 0){
                    return "";
                }
                if (TA.length() < 5){
                    TA = addZeros(TA, 5);
                }
                else if (TA.length() > 5){
                    TA = TA.erase(0, TA.length()-5);
                }
                if (expType != "R"){
                    operandError = "*****Invalid Operand!!!!!";
                    printFlags = false;
                    return "";
                }
            }
            else {
                n = '1'; i = '1'; x = '0'; e = '1'; p = '0'; b = '0';
                TA = calExpression(operand);
                if (TA.length() == 0){
                    return "";
                }
                if (TA.length() < 5){
                    TA = addZeros(TA, 5);
                }
                else if (TA.length() > 5){
                    TA = TA.erase(0, TA.length()-5);
                }
            }
            string op = hexToBin(opcode);
            op[op.length()-2] = n;
            op[op.length()-1] = i;
            op += x; op += b; op += p; op += e;
            op = binToHex(op);
            op += TA;
            op = addZeros(op, 8);
            return op;
        }
    }
    else if ((operation == "RESW") || (operation == "RESB") || (operation == "ORG")){
        cutRecord = true;
        return "";
    }
    else if (operation == "WORD"){
        string val = calExpression(operand);
        if (val.length() == 0){
            return "";
        }
        if (expType == "A"){
            if (val.length() > 6){
                val = val.erase(0,val.length()-6);
            }
            val = addZeros(val,6);
            return val;
        }
        else{
            operandError = "*****illegal operand!!!!";
            return "";
        }
    }
    else if (operation == "BYTE"){
        if (operand[0] == 'C'){
            regex reg("C\'([^.]+)\'");
            std::smatch match;
            if (regex_match(operand,reg)){
                regex_search(operand,match,reg);
                string str = match[1];
                string hexValue = "";
                for (int i = 0; i < str.length(); i++){
                    int t = (int)str[i];
                    hexValue = hexValue + decToHex(t);
                }
                return hexValue;
            }
        }
        else if (operand[0] == 'X'){
            regex reg("X\'(\\w+)\'");
            std::smatch match;
            if (regex_match(operand,reg)){
                regex_search(operand,match,reg);
                string str = match[1];
                return str;
            }
        }
    }
    else if ((operation == "EQU") || (operation == "LTORG")){
        return "";
    }
    else if (operation == "BASE"){
        if (operand.length() == 0){
            base = Start;
        }
        else{
            base = calExpression(operand);
            if (base.length() == 0){
                return "";
            }
        }
        return "";
    }
    else if(operation == "NOBASE"){
        base = "";
    }
    return "";
}

string decToHex (int decimal) {
    if (decimal == 0){
        return "0";
    }
    stringstream sstream;
    sstream << showbase << uppercase << hex << decimal;
    string result = sstream.str().erase(0,2);
    return result;
}

int hexToDec (string hexadecimal) {
    int result = 0;
    stringstream sstream;
    sstream << hex << hexadecimal;
    sstream >> result;
    return result;
}

string addZeros(string loc, int n){
    string locctr("");
    int diff = n - loc.length();
    for (int i=0; i<diff; i++){
        locctr += "0";
    }
    locctr += loc;
    return locctr;
}

string removeSpaces(string operands){
    string temp = "";
    for (int i = 0; i < operands.length(); i++){
        if (operands[i] != ' '){
            temp += operands[i];
        }
    }
    return temp;
}

string searchInRegister(string r){
    string temp = "7";
    string registers[] = {"A","X","L","B","S","T","F"};
    for (int i = 0; i < 7; i++){
        if (r == registers[i]){
            ostringstream ss;
            ss << i;
            temp = ss.str();
            break;
        }
    }
    return temp;
}

string calExpression(string operand){
    int firstInt = 0, secondInt =0;
    bool type1 = false, type2 = false ,firstIsNegative = false;
    string operation;
    string firstStr("");
    string secondStr("");
    if (operand.size() == 1 && operand[0] == '*') {
        expType = "R";
        return LOCCTR;
    }
    regex line("\\s*(-?\\w+)\\s*(\\+|-|\\*|\\/)\\s*(\\w+)\\s*");
    regex num("-?\\d+");
    smatch match1;
    if(regex_match(operand,line)){
        regex_search(operand,match1,line);
        firstStr = match1[1];
        operation = match1[2];
        secondStr = match1[3];
        if(firstStr[0] == '0' && firstStr.size() > 1){
            firstStr = firstStr.erase(0,1);
            for(int i=0;i<firstStr.length();i++){
                if(isalpha(firstStr[i])){
                    if((firstStr[i] < 'A') || (firstStr[i] >'F')){
                        operandError = "\t*****this first operand is not a Hexadecimal Number!!!";
                        return "";
                    }
                }
            }
            type1 = true;
            firstInt = hexToDec(firstStr);
        }
        else{
            if(regex_match(firstStr,num)){
                istringstream ss(firstStr);
                ss >> firstInt;
                type1 = true;
            }
            else{
                if(firstStr[0] == '-'){
                    firstIsNegative = true;
                    firstStr = firstStr.erase(0,1);
                }
                unordered_map<string,vector<string>>::const_iterator found = SYMTAB.find(firstStr);
                if (found == SYMTAB.end()) {
                    operandError = "\t***** Illegal operand !!!!!!";
                    return "";
                }
                if(SYMTAB[firstStr][1] == "A"){
                    type1 = true;
                }
                firstInt = hexToDec(SYMTAB[firstStr][0]);
                if(firstIsNegative){
                    firstInt = -1 * firstInt;
                }
            }
        }
        if(secondStr[0] == '0' && secondStr.size() > 1){
            secondStr = secondStr.erase(0,1);
            for(int i=0;i<secondStr.length();i++){
                if(isalpha(secondStr[i])){
                    if((secondStr[i] < 'A') || (secondStr[i] >'F')){
                        operandError = "\t*****this second operand is not a Hexadecimal Number!!!";
                        return "";
                    }
                }
            }
            type2 = true;
            secondInt = hexToDec(secondStr);
        }
        else{
            if(regex_match(secondStr,num)){
                istringstream ss(secondStr);
                ss >> secondInt;
                type2 = true;
            }
            else{
                unordered_map<string,vector<string>>::const_iterator found = SYMTAB.find(secondStr);
                if (found == SYMTAB.end()) {
                    operandError = "\t***** Illegal operand !!!!!!";
                    return "";
                }
                if(SYMTAB[secondStr][1] == "A"){
                    type2 = true;
                }
                secondInt = hexToDec(SYMTAB[secondStr][0]);
            }
        }
        if(operation == "+"){
            if((type1 && type2) || (type1 && !type2) || (!type1 && type2)){
                int result = firstInt + secondInt;
                string loc = decToHex(result);
                loc = addZeros(loc,6);
                if(type1 && type2){
                    expType = "A";
                }
                else{
                    expType = "R";
                }
                return loc;
            }
            else{
                operandError = "\t****** illegal address expression in operand field";
                errorFound = true;
                return "";
            }
        }
        else if(operation == "-"){
            if((type1 && type2) || (!type1 && !type2) || (!type1 && type2)){
                int result = firstInt - secondInt;
                string loc = decToHex(result);
                loc = addZeros(loc,6);
                if(!type1 && type2){
                    expType = "R";
                }
                else{
                    expType = "A";
                }
                return loc;
            }
            else{
                operandError = "\t****** illegal address expression in operand field";
                errorFound = true;
                return "";
            }
        }
        else if(operation == "*"){
            if(type1 && type2){
                int result = firstInt * secondInt;
                string loc = decToHex(result);
                loc = addZeros(loc,6);
                expType = "A";
                return loc;
            }
            else{
                operandError = "\t****** illegal address expression in operand field";
                errorFound = true;
                return "";
            }
        }
        else if(operation == "/"){
            if(type1 && type2){
                int result = firstInt / secondInt;
                string loc = decToHex(result);
                loc = addZeros(loc,6);
                expType = "A";
                return loc;
            }
            else{
                operandError = "\t****** illegal address expression in operand field";
                errorFound = true;
                return "";
            }
        }
    }
    else{
        regex oneParam("\\s*(-?\\w+)\\s*");
        if(regex_match(operand,oneParam)){
            regex_search(operand,match1,oneParam);
            firstStr = match1[1];
            if(firstStr[0] == '0' && firstStr.size() > 1){
                firstStr = firstStr.erase(0,1);
                for(int i=0;i<firstStr.length();i++){
                    if(isalpha(firstStr[i])){
                        if((firstStr[i] < 'A') || (firstStr[i] >'F')){
                            operandError = "\t*****this operand is not a Hexadecimal Number!!!";
                            return "";
                        }
                    }
                }
                expType = "A";
                return firstStr;
            }
            if(regex_match(firstStr,num)){
                istringstream ss(firstStr);
                ss >> firstInt;
                expType = "A";
                string loc = decToHex(firstInt);
                loc = addZeros(loc,6);
                return loc;
            }
            else{
                if(firstStr[0] == '-'){
                    firstIsNegative = true;
                    firstStr = firstStr.erase(0,1);
                }
                unordered_map<string,vector<string>>::const_iterator found = SYMTAB.find(firstStr);
                if (found == SYMTAB.end()) {
                    operandError = "\t***** Illegal operand !!!!!!";
                    return "";
                }
                expType = SYMTAB[firstStr][1];
                string loc = SYMTAB[firstStr][0];
                if(firstIsNegative){
                    int negative = -1 * hexToDec(loc);
                    loc = decToHex(negative);
                }
                loc = addZeros(loc,6);
                return loc;
            }
        }
        else if(operand[0] == '='){
            firstStr = operand.erase(0,1);
            unordered_map<string,vector<string>>::const_iterator found2 = LITTAB.find(firstStr);
            if (found2 != LITTAB.end()) {
                expType = "R";
                string s = LITTAB[firstStr][2];
                return s;
            }
            else{
                operandError = "\t***** Illegal operand !!!!!!";
                return "";
            }
        }
        else{
            operandError = "\t***** Illegal operand !!!!!!";
            return "";
        }
    }
    return "";
}

vector<string> divideLine(string line){
    string loc("");
    string label("");
    string operation("");
    string operand("");
    string innerComment("");
    int i = 0;
    for (i=0;i<6;i++){
        loc += line[i];
    }
    i++;
    for(i=i;i<16;i++){
        if(line[i] == ' '){
            continue;
        }
        else{
            label += line[i];
        }
    }
    i++;
    if(label == "*"){
        string str = "";
        while(i < line.length()){
            str += line[i++];
        }
        regex literal("\\s*((=\\w\'[^.]+\')|(=\\d+))\\s*");
        smatch match;
        regex_search(str,match,literal);
        operation = match[1];
    }
    else{
        for(i=i;i<26;i++){
            if(line[i] == ' '){
                continue;
            }
            else{
                operation += line[i];
            }
        }
        string str ="";
        while(i < line.length()){
            str += line[i++];
        }
        regex codeSplitter1("\\s*((=?\\*)|(=?\\d+)|(\\s*(-?\\w+)\\s*(\\+|-|\\*|\\/)\\s*(\\w+)\\s*)|(\\w+\\s*,\\s*\\w)|(=?\\w\'[^.]+\')|(@\\s*\\w+)|(#\\s*\\w+)|(-?\\w+))\\s+(/[^.]+)\\s*");
        regex codeSplitter2("\\s*((=?\\*)|(=?\\d+)|(\\s*(-?\\w+)\\s*(\\+|-|\\*|\\/)\\s*(\\w+)\\s*)|(\\w+\\s*,\\s*\\w)|(=?\\w\'[^.]+\')|(@\\s*\\w+)|(#\\s*\\w+)|(-?\\w+))\\s*");
        regex codeSplitter3("\\s*(/[^.]+)\\s*");
        smatch match;
        if(regex_match(str,codeSplitter1)){
            regex_search(str,match,codeSplitter1);
            operand = match[1];
            innerComment = match[12];
        }
        else if(regex_match(str,codeSplitter2)){
            regex_search(str,match,codeSplitter2);
            operand = match[1];
        }
        else if(regex_match(str,codeSplitter3)){
            regex_search(str,match,codeSplitter3);
            operand = match[1];
        }
    }
    vector<string> data;
    data.push_back(loc);
    data.push_back(label);
    data.push_back(operation);
    data.push_back(operand);
    data.push_back(innerComment);
    return data;
}

string addingOPCode(string line,string operand){
    int max = maxLength-line.length();
    for(int i=0; i<max; i++){
        line += " ";
    }
    return line;
}

string writeTxtRecord(string txtRecord,string lastStartOfTxtRecord){
    string record("T"+lastStartOfTxtRecord);
    int length = txtRecord.length() / 2;
    record += addZeros(decToHex(length),2);
    record += txtRecord;
    return record;
}

string cutLabel(string name){
    string str("");
    for(int i=0;i<6 && i<name.length();i++){
        str += name[i];
    }
    while(str.length() < 6){
        str+=" ";
    }
    return str;
}

string getDisp(int ta, int pc){
    int dis = ta - pc;
    string DIS = "";
    if ((dis <= 2047) && (dis >= -2048)){
        DIS = decToHex(dis);
        baseRelative = false;
        if (DIS.length() < 3){
            DIS = addZeros(DIS, 3);
        }
        else if (DIS.length() > 3){
            DIS = DIS.erase(0, DIS.length()-3);
        }
    }
    else if (base.length() != 0){
        dis = ta - hexToDec(base);
        if ((dis <= 4095) && (dis >= 0)){
            DIS = decToHex(dis);
            baseRelative = true;
            if (DIS.length() < 3){
                DIS = addZeros(DIS, 3);
            }
            else if (DIS.length() > 3){
                DIS = DIS.erase(0, DIS.length()-3);
            }
        }
        else {
            operandError = "*****Base Relative doesn't Apply!!!!!";
        }
    }
    else{
        operandError = "*****PC & Base Relative don't Apply!!!!";
    }
    return DIS;
}

void printListFlie(){
 /*   cout << ">>  Source Program statements with value of LC indicated\n" <<endl;
    for(int i=0;i<readListFile.size();i++){
        cout << readListFile[i] << endl;
    }
    cout << "\n>>    e n d    o f   p a s s   1 \n" << endl;
    cout << ">>   *****************************************************" << endl;
    cout << ">>    s y m b o l     t a b l e   (values in decimal)\n" << endl;
    cout << "  \tname\t\tvalue\t\tType" << endl;
    cout << "  \t----------------------------------------" << endl;
    int max = labels.size();
    for(int i=0;i<max;i++){
        string label = labels[i];
        cout << "  \t" << labels[i];
        if(label.size() > 7){
            cout << "\t" << hexToDec(SYMTAB[labels[i]][0]) << "\t\t" << SYMTAB[labels[i]][1] << endl;
        }
        else{
            cout << "\t\t" << hexToDec(SYMTAB[labels[i]][0]) << "\t\t" << SYMTAB[labels[i]][1] << endl;
        }
    }*/
    cout << ">>   *****************************************************" <<endl;
    cout << ">>   S t a r t   o f    P a s s   I I\n" << endl;
    cout << ">>   A s s e m b l e d    p r o g r a m     l i s t i n g" <<endl;
    for(int i=0;i<CodeToPrint.size();i++){
        cout << CodeToPrint[i] << endl;
    }
    if(errorFound){
        cout << ">>    i n c o p l e t e    a s s e m b l y" << endl;
    }
    else{
        cout << ">>    s u c c e s s f u l    a s s e m b l y" << endl;
    }
}

void printObjectCode(vector<string>records){
    for(int i=0;i<records.size();i++){
        cout << records[i] << endl;
    }
}

void maxStringLength(){
    for(int i=0;i<readListFile.size();i++){
        if(readListFile[i].length() > maxLength){
            maxLength = readListFile[i].length();
        }
    }
}
