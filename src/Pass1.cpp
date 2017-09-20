#include "Pass1.h"
using namespace std;
Pass1::Pass1(string str)
{
    initialize(str);
}

void Pass1::initialize(string str){
    readCodeFile(str);
	readOPCode();
    string lastOperation;
    bool startFound = false;
    while(!startFound){
        vector<string> data = readLine();
        if(data[1] == "START"){
            startFound = true;
            if(data[2].size() != 0){
                LOCCTR = addZeros(data[2]);
            }
            else{
                LOCCTR = "000000";
            }
            lastOperation = "START";
            string str1 = LOCCTR;
            str1 = addSpaces(str1);
            data[0] = addSpaces(data[0]);
            data[1] = addSpaces(data[1]);
            data[2] = addSpaces(data[2]);
            data[3] = addSpaces(data[3]);
            codeToPrint.push_back(LOCCTR+" " + data[0] + data[1] + data[2] + data[3]);
        }
        else{
            string str = linesOfCode[counter-1];
            while(str[0] == ' '){
                str = str.erase(0,1);
            }
            if(str[0] == '.'){
                codeToPrint.push_back("       "+linesOfCode[counter-1]);
            }
            else{
                syntaxError = "\t*****No line can be written before start except a comment";
                errorFound = true;
                codeToPrint.push_back(linesOfCode[counter-1]);
                codeToPrint.push_back(syntaxError);
                syntaxError = "";
            }
        }
    }
    vector<string> data;
    string oldLoc;
    string error;
    while(lastOperation != "END"){
        data = readLine();
        lastOperation = data[1];
        oldLoc = LOCCTR;
        string str = linesOfCode[counter-1];
        while(str[0] == ' '){
            str = str.erase(0,1);
        }
        if(str[0] == '.'){
            codeToPrint.push_back("       "+linesOfCode[counter-1]);
        }
        else{
            if(data[1] == "EQU"){
                if((data[0].length() != 0) && (data[2].length() != 0)){
                    string tmp = calExpression(data[2]);
                    if(operandError.size() == 0){
                        insertSymbol(data[0], tmp,expType);
                        expType = "";
                    }else{
                        errorFound = true;
                    }
                }
                else {
                    labelError = "***** EQU must has a label and operand!!!!";
                    errorFound = true;
                }
            }else{
                if(data[1] != "ORG" && data[1] != "WORD" && data[1] != "RESW" && data[1] != "RESB" && (data[2].find('+') != std::string::npos
                            || data[2].find('-') != std::string::npos
                            || data[2].find('/') != std::string::npos
                            || ((data[2] != "*" )&& (data[2] != "=*") && (data[2].find('*') != std::string::npos)))){
                    operandError = "\t***** extra characters at end of statement";
                    errorFound = true;
                }
                if(data[2][0] == '='){
                    checkLiteral(data[2]);
                }
                insertSymbol(data[0], oldLoc, "R");
                modifyLOCCTR(data[0], data[1],data[2]);
            }
            if((data[1] == "END") && (litTab.size() != 0)){
                writeLit();
                litLabels.clear();
                oldLoc = LOCCTR;
            }
            data[0] = addSpaces(data[0]);
            data[1] = addSpaces(data[1]);
            data[2] = addSpaces(data[2]);
            data[3] = addSpaces(data[3]);
            if(syntaxError.size() != 0){
                codeToPrint.push_back(oldLoc + " " + linesOfCode[counter-1]);
                codeToPrint.push_back(syntaxError);
                errorFound = true;
            }
            else {
                codeToPrint.push_back(oldLoc + " " + data[0] + data[1] + data[2] + data[3]);
                if (ltorgFlag) {
                    writeLit();
                    litLabels.clear();
                    ltorgFlag = false;
                }
                if(labelError.size() != 0){
                    codeToPrint.push_back(labelError);
                    errorFound = true;
                }
                if(operationError.size() != 0){
                    codeToPrint.push_back(operationError);
                    errorFound = true;
                }
                if(operandError.size() != 0){
                    codeToPrint.push_back(operandError);
                    errorFound = true;
                }
            }
        }
        syntaxError = "";
        labelError = "";
        operationError = "";
        operandError = "";
    }
    while(counter != linesOfCode.size()){
        codeToPrint.push_back("       "+linesOfCode[counter]);
        if(linesOfCode[counter][0] != '.'){
            codeToPrint.push_back("*****No line can be written after end except a comment");
            errorFound = true;
        }
        counter++;
    }
    printListFile();
}

void Pass1::readOPCode(){
    ifstream infile;
	infile.open ("Opcode.txt");
    while(!infile.eof()){
        string str;
        regex readOPFile("\\s*(\\w+)\\s+(\\d/?\\d?)\\s+(\\w\\w)\\s+(\\d)\\s+(\\w+)");
        std::smatch match;
        getline(infile,str);
        if(regex_search(str,match,readOPFile)){
            string key(match[1].str());
            vector<string> values;
            values.push_back(match[2].str());
            values.push_back(match[3].str());
            values.push_back(match[4].str());
            values.push_back(match[5].str());
            OPCode[key] = values;
        }
    }
	infile.close();
}

void Pass1::readCodeFile(string path){
    ifstream infile;
	infile.open (path);
	string str;
    while(!infile.eof()){
        getline(infile,str);
        if(str.size()>0){
            linesOfCode.push_back(str);
        }
    }
	infile.close();
}

string Pass1::decToHex (int decimal) {
    if (decimal == 0){
        return "0";
    }
    stringstream sstream;
    sstream << showbase << uppercase << hex << decimal;
    string result = sstream.str().erase(0,2);
    return result;
}

int Pass1::hexToDec (string hexadecimal) {
    int result = 0;
    stringstream sstream;
    sstream << hex << hexadecimal;
    sstream >> result;
    return result;
}

vector<string> Pass1::readLine(){
    string str = linesOfCode.at(counter++);
    vector<string> data;
    string label("");
    string operation("");
    string operand("");
    string comment("");
    int i=0;
    while(str[i] == ' '){
        i++;
    }
    if(str[i] == '.'){
        comment = str;
        data.push_back(label);
        data.push_back(operation);
        data.push_back(operand);
        data.push_back(comment);
    }
    else{
        regex codeSplitter1("\\s*(\\w+)\\s+(\\+?\\s*\\w+)\\s+((=?\\*)|(=?\\d+)|(\\s*(-?\\w+)\\s*(\\+|-|\\*|\\/)\\s*(\\w+)\\s*)|(\\w+\\s*,\\s*\\w)|(=?\\w\'[^.]+\')|(@\\s*\\w+)|(#\\s*\\w+)|(-?\\w+))\\s+(/[^.]+)\\s*");
        regex codeSplitter2("\\s*(\\w+)\\s+(\\+?\\s*\\w+)\\s+((=?\\*)|(=?\\d+)|(\\s*(-?\\w+)\\s*(\\+|-|\\*|\\/)\\s*(\\w+)\\s*)|(\\w+\\s*,\\s*\\w)|(=?\\w\'[^.]+\')|(@\\s*\\w+)|(#\\s*\\w+)|(-?\\w+))\\s*");
        regex codeSplitter3("\\s*(\\+?\\s*\\w+)\\s+((=?\\*)|(=?\\d+)|(\\s*(-?\\w+)\\s*(\\+|-|\\*|\\/)\\s*(\\w+)\\s*)|(\\w+\\s*,\\s*\\w)|(=?\\w\'[^.]+\')|(@\\s*\\w+)|(#\\s*\\w+)|(-?\\w+))\\s+(/[^.]+)\\s*");
        regex codeSplitter4("\\s*(\\+?\\s*\\w+)\\s+((=?\\*)|(=?\\d+)|(\\s*(-?\\w+)\\s*(\\+|-|\\*|\\/)\\s*(\\w+)\\s*)|(\\w+\\s*,\\s*\\w)|(=?\\w\'[^.]+\')|(@\\s*\\w+)|(#\\s*\\w+)|(-?\\w+))\\s*");
        regex codeSplitter5("\\s*(\\+?\\s*\\w+)\\s+(/[^.]+)\\s*");
        regex codeSplitter6("\\s*(\\+?\\s*\\w+)\\s*");
        std::smatch match1;
        if(regex_match(str,codeSplitter1)){
            regex_search(str,match1,codeSplitter1);
            label = match1[1];
            operation = match1[2];
            operand = match1[3];
            comment = match1[9];
        }
        else if(regex_match(str,codeSplitter2)){
            regex_search(str,match1,codeSplitter2);
            label = match1[1];
            operation = match1[2];
            operand = match1[3];
        }
        else if(regex_match(str,codeSplitter3)){
            regex_search(str,match1,codeSplitter3);
            string checkStr1 = match1[1];
            string checkStr2 = match1[2];
            comment = match1[8];
            if (checkStr1[0] == '+'){
                operation = checkStr1;
                operand = checkStr2;
            }
            else{
                string upper = checkStr1;
                transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
                bool found2 = searchInDirectives(upper);
                unordered_map<string,vector<string>>::const_iterator found = OPCode.find(upper);
                if(found != OPCode.end() || found2){
                    operation = checkStr1;
                    operand = checkStr2;
                }
                else{
                    label = checkStr1;
                    operation = checkStr2;
                }
            }
        }
        else if(regex_match(str,codeSplitter4)){
            regex_search(str,match1,codeSplitter4);
            string checkStr1 = match1[1];
            string checkStr2 = match1[2];
            if (checkStr1[0] == '+'){
                operation = checkStr1;
                operand = checkStr2;
            }
            else{
                string upper = checkStr1;
                transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
                bool found2 = searchInDirectives(upper);
                unordered_map<string,vector<string>>::const_iterator found = OPCode.find(upper);
                if(found != OPCode.end() || found2){
                    operation = checkStr1;
                    operand = checkStr2;
                }
                else{
                    label = checkStr1;
                    operation = checkStr2;
                }
            }
        }
        else if(regex_match(str,codeSplitter5)){
            regex_search(str,match1,codeSplitter5);
            operation = match1[1];
            comment = match1[2];
        }
        else if(regex_match(str,codeSplitter6)){
            regex_search(str,match1,codeSplitter6);
            operation = match1[1];
        }
        else{
            syntaxError = "\t*****illegal code!!!!!";
        }
        transform(label.begin(), label.end(), label.begin(), ::toupper);
        data.push_back(label);
        transform(operation.begin(), operation.end(), operation.begin(), ::toupper);
        data.push_back(operation);
        regex characterStr("=?(c|C)'[^.]+'");
        if(regex_match(operand,characterStr)){
            if(operand[0] == '=' && operand[1] == 'c'){
                operand[1] = toupper(operand[1]);
            }
            else if(operand[0] == 'c'){
                operand[0] = toupper(operand[0]);
            }
        }
        else{
            transform(operand.begin(), operand.end(), operand.begin(), ::toupper);
        }
        data.push_back(operand);
        data.push_back(comment);
    }
    return data;
}

bool Pass1::modifyLOCCTR (string label, string operation, string operands) {
    bool extended = false;
    //string opcode = operation;
    if (operation[0] == '+'){
        extended = true;
        operation = operation.erase(0,1);
        while(operation[0] == ' '){
            operation = operation.erase(0,1);
        }
    }
    int loc = hexToDec(LOCCTR);
    unordered_map<string,vector<string>>::const_iterator found = OPCode.find(operation);
    if (found != OPCode.end()) {
        vector<string> temp = OPCode[operation];
        string format = temp[0];
        string opcode = temp[1];
        string operandsNum = temp[2];
        string operandsTypes = temp[3];
        if ((operands.length() == 0) && (operation != "RSUB")){
            operandError = "****** missing or misplaced operand field!!!!";
            return false;
        }
        if (extended){
            if (format == "3/4"){
                operands = removeSpaces(operands);
                if (operands[operands.length() - 2] == ','){
                    if (operands[operands.length()-1] == 'X'){
                        loc += 4;
                    }
                    else{
                        operandError = "*****illegal register with indexed addressing!!";
                        return false;
                    }
                }
                else{
                    loc += 4;
                }
            }
            else {
                operationError = "*****can not be Format 4 instruction!!!!";
                return false;
            }
        }
        else {
            if (format == "2"){
                operands = removeSpaces(operands);
                if ((operandsNum == "1") && (operands.length() == 1)){
                    string rNum = searchInRegister(operands);
                    if(rNum == "7"){
                        operandError = "*****illegal address for a register!!!!";
                        return false;
                    }
                }
                else if ((operandsNum == "2") && (operands.length() == 3) && (operands[1] == ',')){
                    string rNum1 = "";
                    rNum1 += operands[0];
                    rNum1 = searchInRegister(rNum1);
                    string rNum2 = "";
                    rNum2 += operands[2];
                    rNum2 = searchInRegister(rNum2);
                    if((rNum1 == "7") ||(rNum2 == "7")){
                        operandError = "*****illegal address for a register!!!!";
                        return false;
                    }
                }
                else{
                    operandError = "*****illegal operand!!!!";
                    return false;
                }
                loc += 2;
            }
            else {
                operands = removeSpaces(operands);
                if (operands[operands.length() - 2] == ','){
                    if (operands[operands.length()-1] == 'X'){
                        loc += 3;
                    }
                    else{
                        operandError = "*****illegal register with indexed addressing!!";
                        return false;
                    }
                }
                else{
                    loc += 3;
                }
            }
        }
        string locctr = decToHex(loc);
        LOCCTR = addZeros(locctr);
        return true;
    }
    else if (!extended) {
        if (operation == "WORD"){
            loc += 3;
        }
        else if (operation == "RESW"){
            int value = 0;
            istringstream iss(operands);
            iss >> value;
            if((iss.fail()) || (value < 0)){
                if (value < 0){
                    operandError = "*****Illegal Operand!!!!!";
                    return false;
                }
                else{
                    string val = calExpression(operands);
                    if (val.length() == 0){
                        return false;
                    }
                    else if (expType == "R"){
                        operandError = "*****Illegal Operand!!!!!";
                        return false;
                    }
                    else{
                        int len = hexToDec(val);
                        len *= 3;
                        loc += len;
                    }
                }
            }
            else {
                value *= 3;
                loc += value;
            }
        }
        else if (operation == "RESB"){
            int value = 0;
            istringstream iss(operands);
            iss >> value;
            if((iss.fail()) || (value < 0)){
                if (value < 0){
                    operandError = "*****Illegal Operand!!!!!";
                    return false;
                }
                else{
                    string val = calExpression(operands);
                    if (val.length() == 0){
                        return false;
                    }
                    else if (expType == "R"){
                        operandError = "*****Illegal Operand!!!!!";
                        return false;
                    }
                    else{
                        int len = hexToDec(val);
                        loc += len;
                    }
                }
            }
            else {
                loc += value;
            }
        }
        else if (operation == "BYTE"){
            if (operands[0] == 'C'){
                regex reg("C\'([^.]+)\'");
                std::smatch match;
                if (regex_match(operands,reg)){
                    regex_search(operands,match,reg);
                    string str = match[1];
                    loc += str.length();
                }
                else {
                    operandError = "*****Illegal Operand!!!!!";
                    return false;
                }
            }
            else if (operands[0] == 'X'){
                regex reg("X\'(\\w+)\'");
                std::smatch match;
                if (regex_match(operands,reg)){
                    regex_search(operands,match,reg);
                    string str = match[1];
                    if (str.length() % 2 == 0){
                        for (int i = 0; i < str.length(); i++){
                            if ((isdigit(str[i])) || ((str[i] >= 'A') && ((str[i] <= 'F')))){
                                continue;
                            }
                            else{
                                operandError = "*****Not a Hexadecimal Number!!!";
                                return false;
                            }
                        }
                        loc += str.length()/2;
                    }
                    else {
                        operandError = "*****Length of Hexadecimal Byte must be Even!!!!!";
                        return false;
                    }
                }
                else {
                    operandError = "*****Illegal Operand!!!!!";
                    return false;
                }
            }
            else {
                operandError = "*****Illegal Operand!!!!!";
                return false;
            }
        }
        else if(operation == "ORG"){
            if(label.length() != 0){
                labelError = "***** ORG must has no label!!!";
                return false;
            }
            if (operands.length() != 0) {
                string tmp = calExpression(operands);
                prevLoc = LOCCTR;
                if(operandError.size() == 0){
                    loc = hexToDec(tmp);
                }
            }
            else {
                if(operandError.size() == 0){
                    if (prevLoc.length() == 0){
                        operandError = "***** no previous org used!!!!!";
                        return false;
                    }
                    loc = hexToDec(prevLoc);
                }
            }
            expType = "";
        }
        else if(operation == "LTORG"){
            if(label.length() != 0){
                labelError = "***** LTORG must has no label or operand!!!!";
                return false;
            }
            if (operands.length() == 0){
                ltorgFlag = true;
            }
            else{
                operandError = "*****LTORG must has no operand or label!!!!";
                return false;
            }
        }
        else if (operation == "BASE") {
            if(label.length() != 0){
                labelError = "***** BASE must has no label!!!!";
                return false;
            }
            return true;
        }
        else if(operation == "NOBASE"){
            if(label.length() != 0){
                labelError = "***** NOBASE must has no label or operand!!!!";
                return false;
            }
            if(operands.length() != 0){
                operandError = "***** NOBASE must has no operand or label!!!!";
                return false;
            }
            return true;
        }
        else {
            if(operation != "END"){
                operationError = "*****Invalid Operation Code!!!!!";
                return false;
            }
        }
        string locctr = decToHex(loc);
        LOCCTR = addZeros(locctr);
        return true;
    }
    else {
        operationError = "*****Invalid Operation Code!!!!!";
        return false;
    }
}

bool Pass1::insertSymbol(string label, string loc,string type) {
    if(label.size() == 0){
        return false;
    }
    unordered_map<string,vector<string>>::const_iterator found = SYMTAB.find(label);
    if (found == SYMTAB.end()) {
        labels.push_back(label);
        vector<string> tmp;
        tmp.push_back(loc);
        tmp.push_back(type);
        SYMTAB[label] = tmp;
        return true;
    }
    else {
        labelError = "\t*****Duplicate Symbol!!!!!";
        return false;
    }
}

bool Pass1::searchInDirectives(string operation){
    string directives[] = {"START","END","WORD","BYTE","RESW","RESB","EQU","ORG","LTORG","BASE", "NOBASE"};
    for(int i=0; i < 11;i++){
        if(directives[i] == operation){
            return true;
        }
    }
    return false;
}

void Pass1::printListFile(){
 //   ofstream outFile;
  //  outFile.open("LISAFILE.txt");
    cout << ">>  Source Program statements with value of LC indicated\n" <<endl;
    int max = codeToPrint.size();
    for(int i=0;i<max;i++){
        cout << codeToPrint[i] << endl;
    }
    if(!errorFound){
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
        }
    }
    else{
        cout << ">>    i n c o p l e t e    a s s e m b l y" << endl;
    }
}

string Pass1::calExpression(string operand){
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
        if((firstStr[0] == '0'  && firstStr.size() > 1)){
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
                    operandError = "\t***** No forward reference is allowed for the first operand in the expression";
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
        if(secondStr[0] == '0'  && secondStr.size() > 1){
            secondStr = secondStr.erase(0,1);
            for(int i=0;i<secondStr.length();i++){
                if(isalpha(secondStr[i])){
                    if((secondStr[i] < 'A') || (secondStr[i] >'F')){
                        operandError = "\t*****this first operand is not a Hexadecimal Number!!!";
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
                    operandError = "\t***** No forward reference is allowed for the second operand in the expression";
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
                if(result < 0){
                    operandError = "\t***** The expression gives a negative location";
                    return "";
                }
                string loc = decToHex(result);
                loc = addZeros(loc);
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
                if(result < 0){
                    operandError = "\t***** The expression gives a negative location";
                    return "";
                }
                string loc = decToHex(result);
                loc = addZeros(loc);
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
                if(result < 0){
                    operandError = "\t***** The expression gives a negative location";
                    return "";
                }
                string loc = decToHex(result);
                loc = addZeros(loc);
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
                if(result < 0){
                    operandError = "\t***** The expression gives a negative location";
                    return "";
                }
                string loc = decToHex(result);
                loc = addZeros(loc);
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
        regex oneParam("\\s*(\\w+)\\s*");
        if(regex_match(operand,oneParam)){
            regex_search(operand,match1,oneParam);
            firstStr = match1[1];
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
                expType = "A";
                return firstStr;
            }
            else{
                if(regex_match(firstStr,num)){
                    istringstream ss(firstStr);
                    ss >> firstInt;
                    expType = "A";
                    string loc = decToHex(firstInt);
                    loc = addZeros(loc);
                    return loc;
                }
                else{
                    unordered_map<string,vector<string>>::const_iterator found = SYMTAB.find(firstStr);
                    if (found == SYMTAB.end()) {
                        operandError = "\t***** No forward reference is allowed !!!!!!";
                        return "";
                    }
                    else{
                        expType = SYMTAB[firstStr][1];
                        string loc = SYMTAB[firstStr][0];
                        loc = addZeros(loc);
                        return loc;
                    }
                }
            }
        }
        else{
            operandError = "\t****** illegal address expression in operand field";
            errorFound = true;
            return "";
        }
    }
    return "";
}

void Pass1::checkLiteral(string literal){
    literal = literal.erase(0,1);
    unordered_map<string,vector<string>>::const_iterator found = litTab.find(literal);
    if (found == litTab.end()) {
        if(literal[0] == 'C'){
            regex reg("C\'([^.]+)\'");
            std::smatch match;
            if (regex_match(literal,reg)){
                regex_search(literal,match,reg);
                string str = match[1];
                vector <string> lit;
                string hexValue = "";
                for (int i = 0; i < str.length(); i++){
                    int t = (int)str[i];
                    hexValue = hexValue + decToHex(t);
                }
                lit.push_back(hexValue);
                ostringstream os;
                os << str.length();
                lit.push_back(os.str());
                lit.push_back("");
                litLabels.push_back(literal);
                litTab[literal] = lit;
            }
        }
        else if(literal[0] == 'X'){
            regex reg("X\'(\\w+)\'");
            std::smatch match;
            if (regex_match(literal,reg)){
                regex_search(literal,match,reg);
                string str = match[1];
                if (str.length() % 2 == 0){
                    for (int i = 0; i < str.length(); i++){
                        if ((isdigit(str[i])) || ((str[i] >= 'A') && ((str[i] <= 'F')))){
                            continue;
                        }
                        else{
                            operandError = "*****Not a Hexadecimal Number!!!";
                            return;
                        }
                    }
                    vector <string> lit;
                    lit.push_back(str);
                    ostringstream os;
                    os << str.length() / 2;
                    lit.push_back(os.str());
                    lit.push_back("");
                    litLabels.push_back(literal);
                    litTab[literal] = lit;
                }
            }
        }
        else if (literal == "*"){
            vector <string> lit;
            lit.push_back(LOCCTR);
            ostringstream os;
            os << LOCCTR.length() / 2;
            lit.push_back(os.str());
            lit.push_back("");
            litLabels.push_back(literal);
            litTab[literal] = lit;
        }
        else{
            regex digit("\\s*(\\d+)\\s*");
            if(regex_match(literal,digit)){
                smatch match;
                regex_search(literal,match,digit);
                literal = match[1];
                int value = 0;
                istringstream ss(literal);
                ss >> value;
                string tmp = decToHex(value);
                if(tmp.length() % 2 == 1){
                    tmp = "0" + tmp;
                }
                vector <string> lit;
                lit.push_back(tmp);
                ostringstream os;
                os << tmp.length() / 2;
                lit.push_back(os.str());
                lit.push_back("");
                litLabels.push_back(literal);
                litTab[literal] = lit;
            }
        }
    }
    else if(literal == "*"){
        operandError = "\t****** '=*' literal is allowed for 1 time only";
    }
}

void Pass1::writeLit(){
    for(int i = 0; i < litLabels.size(); i++){
        if(litTab[litLabels[i]][2] == ""){
            string str1 = LOCCTR + " ";
            string str2 = addSpaces("*");
            string str3 = "="+litLabels[i];
            codeToPrint.push_back(str1+str2+str3);
            litTab[litLabels[i]][2] = LOCCTR;
            int value = 0;
            istringstream ss(litTab[litLabels[i]][1]);
            ss >> value;
            int loc = hexToDec(LOCCTR);
            loc += value;
            string locctr = decToHex(loc);
            LOCCTR = addZeros(locctr);
        }
    }
}

string Pass1::addZeros(string loc){
    string locctr("");
    int diff = 6 - loc.length();
    for (int i=0; i<diff; i++){
        locctr += "0";
    }
    locctr += loc;
    return locctr;
}

string Pass1::addSpaces(string str1){
    while(str1.length() <= 10){
        str1 = str1 +" ";
    }
    return str1;
}

string Pass1::removeSpaces(string operands){
    string temp = "";
    for (int i = 0; i < operands.length(); i++){
        if (operands[i] != ' '){
            temp += operands[i];
        }
    }
    return temp;
}

string Pass1::searchInRegister(string r){
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

unordered_map<string,vector<string>> Pass1::getOPCode(){
    return OPCode;
}

unordered_map<string,vector<string>> Pass1::getSYMTAB(){
    return SYMTAB;
}

vector<string> Pass1::getCodeToPrint(){
    return codeToPrint;
}

vector<string> Pass1::getLabels(){
    return labels;
}

vector<string> Pass1::getLitLabels(){
    return litLabels;
}

string Pass1::getLOCCTR(){
    return LOCCTR;
}

bool Pass1::getErrorFound(){
    return errorFound;
}

unordered_map<string,vector<string>> Pass1::getLitTab(){
    return litTab;
}
