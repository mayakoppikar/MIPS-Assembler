#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <string>
#include <map>
#include <cstdint> // For int32_t and uint32_t
#include <bitset>
#include <iomanip>

std::map<int, std::string> addressToLineMap;
std::map<std::string, int> opcodeMap;
std::map<std::string, int> fcodeMap;


//globals
std::string inputFilename = "input.txt";
std::string outputFilename = "output.txt";
int numLinesInProgram = 0;

std::string int_to_binary_32bit(int number) {
    std::bitset<32> bits(number);
    return bits.to_string();
}

std::string decimalToHex(int number) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(8) << number;
    return ss.str();
}

void createCodeMap(){
    opcodeMap["add"] = 0;
    opcodeMap["sub"] = 0;
    opcodeMap["addi"] = 8;
    opcodeMap["addu"] = 0;
    opcodeMap["subu"] = 0;
    opcodeMap["addiu"] = 9;
    opcodeMap["mfc0"] = 16;
    opcodeMap["mult"] = 0;
    opcodeMap["multu"] = 0;
    opcodeMap["div"] = 0;
    opcodeMap["divu"] = 0;
    opcodeMap["mfhi"] = 0;
    opcodeMap["mflo"] = 0;
    opcodeMap["and"] = 0;
    opcodeMap["or"] = 0;
    opcodeMap["andi"] = 12;
    opcodeMap["ori"] = 13;
    opcodeMap["sll"] = 0;
    opcodeMap["srl"] = 0;
    opcodeMap["lw"] = 35;
    opcodeMap["sw"] = 43;
    opcodeMap["lui"] = 15;
    opcodeMap["beq"] = 4;
    opcodeMap["bne"] = 5;
    opcodeMap["slt"] = 0;
    opcodeMap["slti"] = 10;
    opcodeMap["sltu"] = 0;
    opcodeMap["sltiu"] = 11;
    opcodeMap["j"] = 2;
    opcodeMap["jr"] = 0;
    opcodeMap["jal"] = 3;
    opcodeMap["rbit"] = 0;
    opcodeMap["rev"] = 0;
    opcodeMap["add8"] = 0;
    opcodeMap["sadd"] = 0;
    opcodeMap["ssub"] = 0;

    //add fcodes for R type instructions
    fcodeMap["add"] = 32;
    fcodeMap["sub"] = 34;
    fcodeMap["addu"] = 33;
    fcodeMap["subu"] = 35;
    fcodeMap["add"] = 32;
    fcodeMap["sub"] = 34;
    fcodeMap["addu"] = 33;
    fcodeMap["subu"] = 35;
    fcodeMap["mfc0"] = 0;
    fcodeMap["mult"] = 24;
    fcodeMap["multu"] = 25;
    fcodeMap["div"] = 26;
    fcodeMap["divu"] = 27;
    fcodeMap["mfhi"] = 16;
    fcodeMap["mflo"] = 18;
    fcodeMap["and"] = 36;
    fcodeMap["or"] = 37;
    fcodeMap["sll"] = 0;
    fcodeMap["srl"] = 2;
    fcodeMap["slt"] = 42;
    fcodeMap["sltu"] = 43;
    fcodeMap["jr"] = 8;
    fcodeMap["rbit"] = 47;
    fcodeMap["rev"] = 48;
    fcodeMap["add8"] = 45;
    fcodeMap["sadd"] = 49;
    fcodeMap["ssub"] = 50;
}

int main() {
    //do initial setup
    createCodeMap();
    //later add functions to convert labels to numerical offsets so i dont have to hand convert

    std::ifstream inputFile(inputFilename);
    std::ofstream outputFile(outputFilename);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open input file " << inputFilename << std::endl;
        return 1;
    }

    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to open output file " << outputFilename << std::endl;
        inputFile.close();
        return 1;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        numLinesInProgram++;
        std::stringstream ss(line);
        std::string opcodeName;
        std::getline(ss, opcodeName, ' ');
        std::string restOfLine;
        std::getline(ss, restOfLine);

        char format = ' ';
        int specialCase = 0; //0 is not special, 1 means special opcode form
        int opcode = opcodeMap[opcodeName]; //6 MSB
        int lower26bits;
        int32_t shiftedVal = 0;

        if(!opcodeName.compare("j") || !opcodeName.compare("jal")){
            format = 'J';
            shiftedVal |= ((opcode & 0x3F) << 26);
            shiftedVal |= ((std::stoi(restOfLine)  & 0x3FFFFFF) << 0);
            outputFile << decimalToHex(shiftedVal) << std::endl; 
        }
        else if(!opcodeName.compare("addi") || !opcodeName.compare("addiu") || !opcodeName.compare("andi") || !opcodeName.compare("ori") || !opcodeName.compare("lw") || !opcodeName.compare("sw") || !opcodeName.compare("lui") || !opcodeName.compare("beq") || !opcodeName.compare("bne") || !opcodeName.compare("slti") || !opcodeName.compare("sltiu")){
            format = 'I';
            int rs, rt, immVal;   
            if((!opcodeName.compare("lw")) || (!opcodeName.compare("sw"))){ //I instructions - format: $a #imm($b) 
                char dollar, comma, parenOpen, parenClose;
                std::istringstream ss(restOfLine);
                ss >> dollar >> rt >> comma >> immVal >> parenOpen >> dollar >> rs >> parenClose;
            }
            else if(!opcodeName.compare("lui")){  //I instructions - format: $a #imm
                char dollar, comma;
                std::istringstream ss(restOfLine);   
                rs = 0;
                ss >> dollar >> rt >> comma >> immVal;
            }
            else{ //I instructions - format: $a $a #imm
                char dollar;
                char comma;  
                std::istringstream ss(restOfLine);
                ss >> dollar >> rt >> comma;  
                ss >> dollar >> rs >> comma; 
                ss >> immVal;                                    
            }

            shiftedVal |= ((opcode & 0x3F) << 26);
            shiftedVal |= ((rs & 0x1F) << 21);                       
            shiftedVal |= ((rt & 0x1F) << 16);
            shiftedVal |= ((immVal & 0xFFFF) << 0);  
            outputFile << decimalToHex(shiftedVal) << std::endl;  
        }
        else { //make sure to add extra opcodes as well
            format = 'R'; 
            int rs, rt, rd, shamt, fcode;
            fcode = fcodeMap[opcodeName];
            if(!opcodeName.compare("jr") || !opcodeName.compare("mfhi") || !opcodeName.compare("mflo")){ 
                 //I instructions - format: $a
                char dollar;
                std::istringstream ss(restOfLine);
                if(!opcodeName.compare("jr")){
                    ss >> dollar >> rs; 
                    rd = 0;
                }
                else{
                    ss >> dollar >> rd;
                    rs = 0;
                }
                rt = 0;
                shamt = 0;
            }
            else if(!opcodeName.compare("mfc0") || !opcodeName.compare("mult") || !opcodeName.compare("multu") || !opcodeName.compare("div") || !opcodeName.compare("divu") || !opcodeName.compare("rbit") || !opcodeName.compare("rev")){
                if(!opcodeName.compare("mfc0")){
                    outputFile << "Have not yet implemented mfc0 inst." << std::endl; 
                }
                else{
                    char dollar, comma;
                    std::istringstream ss(restOfLine);
                    rd = 0;
                    shamt = 0;
                    ss >> dollar >> rs >> comma >> dollar >> rt;
                }
            }
            else if(!opcodeName.compare("sll") || !opcodeName.compare("srl")){
                char dollar, comma;
                std::istringstream ss(restOfLine);
                ss >> dollar >> rd >> comma;  
                ss >> dollar >> rt >> comma;   
                ss >> shamt;    
                rs = 0; 
            }
            else{
                char dollar, comma;
                std::istringstream ss(restOfLine);
                ss >> dollar >> rd >> comma;
                ss >> dollar >> rs >> comma;
                ss >> dollar >> rt;
                shamt = 0;
            }

            shiftedVal |= ((opcode & 0x3F) << 26);
            shiftedVal |= ((rs & 0x1F) << 21);                       
            shiftedVal |= ((rt & 0x1F) << 16);
            shiftedVal |= ((rd & 0x1F) << 11);
            shiftedVal |= ((shamt & 0x1F) << 6);
            shiftedVal |= ((fcode & 0xFFFF) << 0);  
            outputFile << decimalToHex(shiftedVal) << std::endl;              
        } 
    }

    inputFile.close();
    outputFile.close();

    return 0;
}