/********************************************************
 *  							*
 *  Name: Dakoda Patterson				*
 *  Class: CS3421 					*
 *  Assignment: Asg 4 (a simulator of a subset of the   *
 *  MIPS instruction set) 				*
 *  Compile: "g++ simulator.c++ -o sim -std=c++11" 	*
 * 							*
 * ******************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <map>

const size_t MAXPROGRAM = 32768;

//typedef support for table mappings
typedef std::pair<const unsigned int,const std::string> paring;
typedef std::pair<const unsigned int, const unsigned int> functionParing;
typedef std::pair<const functionParing, const std::string> funcParing;

void printRegisterState(std::vector<int> &, std::ofstream &);
void printDataMemory(int *, size_t, std::ofstream &);

int main(int argc, char * argv[])
{
    // Checks to see if the file name was specified
    // if it was not the program exits
    if (argc < 2)
    {
        std::cout << argc << "\n";
        std::cerr << " ** File name not specified. **\n";
        exit(EXIT_FAILURE);
    }
    
    // Opens the file to for the rest of the program to read
    std::ifstream inFile(argv[1], std::ios::in);
    inFile.seekg(std::ios::beg);
    
    if (!inFile)
    {
        std::cerr << "File could not be opened.\n";
        exit(EXIT_FAILURE);
    }
    
    size_t numInst;
    inFile >> numInst; 
    size_t numWords;
    inFile >> numWords;
    
    unsigned int readNum;
    unsigned int progInstructions[MAXPROGRAM] = {0};
    int dataArray[MAXPROGRAM] = {0};
    
    // Reads in the programs instructions
    for (size_t i = 0; i < numInst; ++i)
    {
        inFile >> std::hex >> progInstructions[i];
    }
    
    // reads the dataArray
    for (size_t i = 0; i < numWords; ++i)
    {
        inFile >> std::hex >> readNum;
        dataArray[i] = static_cast<int>(readNum);
    }
    
    inFile.close();
    
    // Struct to decode the instructions
    struct {
        union   {
            struct {
                unsigned int funct:6;
                unsigned int shamt:5;
                unsigned int rd:5;
                unsigned int rt:5;
                unsigned int rs:5;
                unsigned int opcode:6;
            } rFormat;
            struct  {
                unsigned int imm:16;
                unsigned int rt:5;
                unsigned int rs:5;
                unsigned int opcode:6;
            } iFormat;
            struct  {
                unsigned int address:26;
                unsigned int opcode:6;
            } jFormat;
            struct  {
                unsigned int address:26;
                unsigned int opcode:6;
            } opcodeCheck;
            unsigned int encoding;
        } u;
    } instructions[MAXPROGRAM] = {0};
    
    //put program instructions into struct to allow parsing of bit field values
    for (size_t i = 0; i < numInst; ++i)
    {
        instructions[i].u.encoding = progInstructions[i];
    }
    
    // create table of argument values with their mapping to a string
    std::map <const unsigned int, const std::string> arguments;
    arguments.insert(paring(0,"zero")); arguments.insert(paring(1,"at")); 
    arguments.insert(paring(2,"v0")); arguments.insert(paring(3,"v1")); 
    arguments.insert(paring(4,"a0")); arguments.insert(paring(5,"a1"));
    arguments.insert(paring(6,"a2")); arguments.insert(paring(7,"a3")); 
    arguments.insert(paring(8,"t0")); arguments.insert(paring(9,"t1")); 
    arguments.insert(paring(10,"t2")); arguments.insert(paring(11,"t3"));
    arguments.insert(paring(12,"t4")); arguments.insert(paring(13,"t5")); 
    arguments.insert(paring(14,"t6")); arguments.insert(paring(15,"t7")); 
    arguments.insert(paring(16,"s0")); arguments.insert(paring(17,"s1"));
    arguments.insert(paring(18,"s2")); arguments.insert(paring(19,"s3")); 
    arguments.insert(paring(20,"s4")); arguments.insert(paring(21,"s5")); 
    arguments.insert(paring(22,"s6")); arguments.insert(paring(23,"s7"));
    arguments.insert(paring(24,"t8")); arguments.insert(paring(25,"t9")); 
    arguments.insert(paring(26,"k0")); arguments.insert(paring(27,"k1")); 
    arguments.insert(paring(28,"gp")); arguments.insert(paring(29,"sp"));
    arguments.insert(paring(30,"fp")); arguments.insert(paring(31,"ra"));
    
    // create table of opcode values with their mapping to a string
    std::map <const functionParing, const std::string> opcodes;
    opcodes.insert(funcParing(functionParing(9,0),"addiu"));
    opcodes.insert(funcParing(functionParing(0,33),"addu"));
    opcodes.insert(funcParing(functionParing(0,36),"and"));
    opcodes.insert(funcParing(functionParing(4,0),"beq"));
    opcodes.insert(funcParing(functionParing(5,0),"bne"));
    opcodes.insert(funcParing(functionParing(0,26),"div"));
    opcodes.insert(funcParing(functionParing(2,0),"j"));
    opcodes.insert(funcParing(functionParing(35,0),"lw"));
    opcodes.insert(funcParing(functionParing(0,16),"mfhi"));
    opcodes.insert(funcParing(functionParing(0,18),"mflo"));
    opcodes.insert(funcParing(functionParing(0,24),"mult"));
    opcodes.insert(funcParing(functionParing(0,37),"or"));
    opcodes.insert(funcParing(functionParing(0,42),"slt"));
    opcodes.insert(funcParing(functionParing(0,35),"subu"));
    opcodes.insert(funcParing(functionParing(43,0),"sw"));
    opcodes.insert(funcParing(functionParing(0,12),"syscall"));
    
    // The following reads and parses the instructions
    
    // text output file
    std::ofstream outFile("log.txt",std::ios::out);
    outFile.seekp(std::ios::beg);
    
    //instruction storage
    std::vector<std::string> instStorage (numInst,"");
    
    outFile << "insts:\n";
    
    // Prints instructions
    for (size_t i = 0; i < numInst; ++i)
    {
        // determine opcodeFunct pair
        unsigned int opcode = instructions[i].u.opcodeCheck.opcode;
        unsigned int funct = 0;  //funct will be 0 unless opcode is 0
        
        // Determines if the instsruction is an "R" instruction an if the funct
        // pair is valid if it is an "R" instruction.
        if (opcode == 0)
        {
            funct = instructions[i].u.rFormat.funct;
            size_t validOpcodeFunct = opcodes.count(functionParing(0,funct));
            if (!validOpcodeFunct)
            {
                std::cerr << "could not find inst with opcode " << opcode << 
				" and funct " << funct << "\n";
                outFile.close();
                exit(EXIT_FAILURE);
            }
        }
        
        outFile << std::setw(4) << std::right;
        outFile << i << ": ";
    
        
        std::string instString;
        instString += opcodes[functionParing(opcode,funct)];
        
        //determines the sequence to ouput
        //separates by opcode using switch statement
        switch(opcode)
        {
            case 0:
                switch(funct)
                {
                    case 33:
                    case 36:
                    case 37:
                    case 42:
                    case 35:
                        instString += "\t";
                        instString += "$";
                        instString += arguments[instructions[i].u.rFormat.rd];
                        instString += ",$";
                        instString += arguments[instructions[i].u.rFormat.rs];
                        instString += ",$";
                        instString += arguments[instructions[i].u.rFormat.rt];
                 
                        instStorage[i] += instString;
                        break;
                    case 26:
                    case 24:
                        instString += "\t";
                        instString += "$";
                        instString += arguments[instructions[i].u.rFormat.rs];
                        instString += ",$";
                        instString += arguments[instructions[i].u.rFormat.rt];
              
                        instStorage[i] += instString;
                        break;
                    case 12: 
                        instStorage[i] += instString;
                        break;
                    case 16:
                    case 18:
                        instString += "\t";
                        instString += "$";
                        instString += arguments[instructions[i].u.rFormat.rd];
                        instStorage[i] += instString;
                        break;
                    default:
                        break;
                }
                break; 
            case 9:
                instString += "\t";
                instString += "$";
                instString += arguments[instructions[i].u.iFormat.rt];
                instString += ",$";
                instString += arguments[instructions[i].u.iFormat.rs];
                instString += ",";
                instString += 
			std::to_string(static_cast<short>
				(instructions[i].u.iFormat.imm));
                instStorage[i] += instString;
                break;
            case 4:
            case 5:
                instString += "\t";
                instString += "$";
                instString += arguments[instructions[i].u.iFormat.rs];
                instString += ",$";
                instString += arguments[instructions[i].u.iFormat.rt];
                instString += ",";
                instString += 
			std::to_string(static_cast<short>
				((instructions[i].u.iFormat.imm)));
                instStorage[i] += instString;
                break;
            case 2:
                instString += "\t";
                instString += std::to_string(instructions[i].u.jFormat.address);
                instStorage[i] += instString;
                break;
            case 35:
            case 43:
                instString += "\t";
                instString += "$";
                instString += arguments[instructions[i].u.iFormat.rt];
                instString += ",";
                instString += 
			std::to_string(static_cast<short>
				(instructions[i].u.iFormat.imm));
                instString += "($";
                instString += arguments[instructions[i].u.iFormat.rs];
                instString += ")";
                instStorage[i] += instString;
                break;
            default:
                std::cerr << "Invalid opcode / funct cominbation\n";
                exit(EXIT_FAILURE);
                break;
        }
        outFile << instStorage[i];
        outFile << "\n";
    } 
    
    // Prints data
    outFile << "\ndata:\n";
    
    for (size_t i = 0; i < numWords; ++i)
    {
        outFile << std::setw(4) << std::right;
        outFile << (i+numInst) << ": " << dataArray[i] << "\n";
    } 
    
    outFile << "\n";
    
    
    // Mips simulator with logged output
    
    // Program counter
    int progCounter = 0;

    // Create vector to store 32 register values
    std::vector<int> registerStore(34,0);
    
    // Initializes $gp
    registerStore[28] = static_cast<int>(numInst);
  
    bool exitCondition = (progCounter >= numInst);
    
    while (!exitCondition)
    {
        outFile << "PC: " << progCounter << "\n";
        
        unsigned int opcode = instructions[progCounter].u.opcodeCheck.opcode;
        unsigned int funct = 0;
        unsigned int rd;
        unsigned int rs;
        unsigned int rt;
        unsigned short imm;
        unsigned int address;
        short immSigned;
        int addressSigned;
        int addrLoadStore;
        
        struct {
            union {
                struct {
                    long lower:32;
                    long upper:32;
                } divLong;
                struct {
                    long full;
                } fullLong;
            } u;
        } longAssist;
    
        if (opcode == 0)
            funct = instructions[progCounter].u.rFormat.funct;
        
        switch(opcode)
        {
            case 0:
                switch(funct)
                {
                    case 33: 
                        outFile << "inst: " << instStorage[progCounter] << "\n";
                        rd = instructions[progCounter].u.rFormat.rd;
                        if (rd != 0)
                        {
                            rs = instructions[progCounter].u.rFormat.rs;
                            rt = instructions[progCounter].u.rFormat.rt;
                            registerStore[rd] = registerStore[rs] + 
							registerStore[rt];
                        }
                        ++progCounter;
                        break;
                    
                    case 36:
                        outFile << "inst: " << instStorage[progCounter] << "\n";
                        rd = instructions[progCounter].u.rFormat.rd;
                        if (rd != 0)
                        {
                            rs = instructions[progCounter].u.rFormat.rs;
                            rt = instructions[progCounter].u.rFormat.rt;
                            registerStore[rd] = registerStore[rs] & 
							registerStore[rt];
                        }
                        ++progCounter;
                        break;

                    case 37:
                        outFile << "inst: " << instStorage[progCounter] << "\n";
                        rd = instructions[progCounter].u.rFormat.rd;
                        if (rd != 0)
                        {
                            rs = instructions[progCounter].u.rFormat.rs;
                            rt = instructions[progCounter].u.rFormat.rt;
                            registerStore[rd] = registerStore[rs] | 
							registerStore[rt];
                        }
                        ++progCounter;
                        break;
                        
                    case 42:
                        outFile << "inst: " << instStorage[progCounter] << "\n";
                        rd = instructions[progCounter].u.rFormat.rd;
                        if (rd != 0)
                        {
                            rs = instructions[progCounter].u.rFormat.rs;
                            rt = instructions[progCounter].u.rFormat.rt;
                            registerStore[rd] = (registerStore[rs] < 
							registerStore[rt]);
                        }
                        ++progCounter;
                        break;
                        
                    case 35:
                        outFile << "inst: " << instStorage[progCounter] << "\n";
                        rd = instructions[progCounter].u.rFormat.rd;
                        if (rd != 0)
                        {
                            rs = instructions[progCounter].u.rFormat.rs;
                            rt = instructions[progCounter].u.rFormat.rt;
                            registerStore[rd] = registerStore[rs] - 
							registerStore[rt];
                        }
                        ++progCounter;
                        break;
                        
                    case 26:
                        outFile << "inst: " << instStorage[progCounter] << "\n";
                        rt = instructions[progCounter].u.rFormat.rt;
                        if (registerStore[rt] == 0)
                        {
                            std::cerr << "divide by zero for instruction at " 
							<< progCounter << "\n";
                            outFile.close();
                            exit(EXIT_FAILURE);
                        }
                        rs = instructions[progCounter].u.rFormat.rs;
                        registerStore[32] = registerStore[rs] / 
							registerStore[rt];
                        registerStore[33] = registerStore[rs] % 
							registerStore[rt];
                        ++progCounter;
                        break;
                        
                    case 24:
                        outFile << "inst: " << instStorage[progCounter] << "\n";
                        rt = instructions[progCounter].u.rFormat.rt;
                        rs = instructions[progCounter].u.rFormat.rs;
                        longAssist.u.fullLong.full = 
					static_cast<long>(registerStore[rs]) *
                                           static_cast<long>(registerStore[rt]);
                        registerStore[32] = longAssist.u.divLong.lower;
                        registerStore[33] = longAssist.u.divLong.upper;
                        ++progCounter;
                        break;
                        
                    case 12:
                        outFile << "inst: " << instStorage[progCounter] << "\n";
                        if (registerStore[2] == 1)
                        {
                            std::cout << registerStore[4] << "\n";
                        }
                        if (registerStore[2] == 5)
                        {
                            std::cin >> registerStore[2];
                        }
                        ++progCounter;
                        break;
                        
                    case 16:
                        outFile << "inst: " << instStorage[progCounter] << "\n";
                        rd = instructions[progCounter].u.rFormat.rd;
                        if (rd != 0)
                        {
                            registerStore[rd] = registerStore[33];
                        }
                        ++progCounter;
                        break;
                
                    case 18:
                        outFile << "inst: " << instStorage[progCounter] << "\n";
                        rd = instructions[progCounter].u.rFormat.rd;
                        if (rd != 0)
                        {
                            registerStore[rd] = registerStore[32];
                        }
                        ++progCounter;
                        break;
                        
                    default:
                        std::cerr << "Something was wrong.\n";
                        outFile.close();
                        exit(EXIT_FAILURE);
                        break;
                }
                break;
                
            case 9:
                outFile << "inst: " << instStorage[progCounter] << "\n";
                rt = instructions[progCounter].u.iFormat.rt;
                if (rt != 0)
                {
                    rs = instructions[progCounter].u.iFormat.rs;
                    imm = instructions[progCounter].u.iFormat.imm;
                    immSigned = static_cast<short>(imm);
                    registerStore[rt] = registerStore[rs] + immSigned;
                }
                ++progCounter;
                break;
                
            case 4:
                rs = instructions[progCounter].u.iFormat.rs;
                rt = instructions[progCounter].u.iFormat.rt;
                imm = instructions[progCounter].u.iFormat.imm;
                immSigned = static_cast<short>(imm);
                if (registerStore[rs] == registerStore[rt])
                {
                    if ( ((progCounter + immSigned) > (numInst - 1))
					 || ((progCounter + immSigned) < 0)  )
                    {
                        if ((progCounter + immSigned) <= 
					(numInst + numWords - 1))
                        {
                            std::cerr << 
				"PC is accessing data memory at address " 
					<< (progCounter + immSigned) << "\n";
                            outFile << "inst: " << instStorage[progCounter]
					 << "\n";
                            printRegisterState(registerStore,outFile);
                            outFile << "\n\n";
                            printDataMemory(dataArray,numWords,outFile);
                            outFile << "\n\n";
                            progCounter += immSigned;
                            outFile << "PC: " << progCounter << "\n";
                            
                        }
                        else
                        {
                            std::cerr << 
				"PC is accessing illegal memory address " << 
						(progCounter + immSigned) << "\n";
                            outFile << "inst: " << instStorage[progCounter] << 
						"\n";
                            printRegisterState(registerStore,outFile);
                            outFile << "\n\n";
                            printDataMemory(dataArray,numWords,outFile);
                            outFile << "\n\n";
                            progCounter += immSigned;
                            outFile << "PC: " << progCounter << "\n";
                        }
                        outFile.close();
                        exit(EXIT_FAILURE);
                    }
                    outFile << "inst: " << instStorage[progCounter] << "\n";
                    progCounter += immSigned;
                }
                else
                {
                    outFile << "inst: " << instStorage[progCounter] << "\n";
                    ++progCounter;
                }
                break;
                
            case 5:
                rs = instructions[progCounter].u.iFormat.rs;
                rt = instructions[progCounter].u.iFormat.rt;
                imm = instructions[progCounter].u.iFormat.imm;
                immSigned = static_cast<short>(imm);
                if (registerStore[rs] != registerStore[rt])
                {
                    if ( ((progCounter + immSigned) > (numInst - 1)) || 
					((progCounter + immSigned) < 0)  )
                    {
                        if ((progCounter + immSigned) <= 
				(numInst + numWords - 1))
                        {
                            std::cerr 
				<< "PC is accessing data memory at address " 
					<< (progCounter + immSigned) << "\n";
                            outFile << "inst: " << instStorage[progCounter] 
					<< "\n";
                            printRegisterState(registerStore,outFile);
                            outFile << "\n\n";
                            printDataMemory(dataArray,numWords,outFile);
                            outFile << "\n\n";
                            progCounter += immSigned;
                            outFile << "PC: " << progCounter << "\n";
                        }
                        else
                        {
                            std::cerr 
				<< "PC is accessing illegal memory address " 
					<< (progCounter + immSigned) << "\n";
                            outFile << "inst: " 
				<< instStorage[progCounter] << "\n";
                            printRegisterState(registerStore,outFile);
                            outFile << "\n\n";
                            printDataMemory(dataArray,numWords,outFile);
                            outFile << "\n\n";
                            progCounter += immSigned;
                            outFile << "PC: " << progCounter << "\n";
                        }
                        outFile.close();
                        exit(EXIT_FAILURE);
                    }
                    outFile << "inst: " << instStorage[progCounter] << "\n";
                    progCounter += immSigned;
                }
                else
                {
                    outFile << "inst: " << instStorage[progCounter] << "\n";
                    ++progCounter;
                }
                break;
                
            case 2:
                address = instructions[progCounter].u.jFormat.address;
                addressSigned = static_cast<int>(address);
                if ( (addressSigned > (numInst - 1)) || (addressSigned < 0) )
                {
                    if ( address < (numInst + numWords) )
                    {
                        std::cerr << "PC is accessing data memory at address "
				 << addressSigned << "\n";
                        outFile << "inst: " << instStorage[progCounter] << "\n";
                        printRegisterState(registerStore,outFile);
                        outFile << "\n\n";
                        printDataMemory(dataArray,numWords,outFile);
                        outFile << "\n\n";
                        progCounter = addressSigned;
                        outFile << "PC: " << progCounter << "\n";
                    }
                    else
                    {
                        std::cerr 
				<< "PC is accessing illegal memory address " 
					<< addressSigned << "\n";
                        outFile << "inst: " << instStorage[progCounter] << "\n";
                        printRegisterState(registerStore,outFile);
                        outFile << "\n\n";
                        printDataMemory(dataArray,numWords,outFile);
                        outFile << "\n\n";
                        progCounter = addressSigned;
                        outFile << "PC: " << progCounter << "\n";
                    }
                    outFile.close();
                    exit(EXIT_FAILURE);
                }
                outFile << "inst: " << instStorage[progCounter] << "\n";
                progCounter = addressSigned;
                break;
    
            case 35:
                outFile << "inst: " << instStorage[progCounter] << "\n";
                imm = instructions[progCounter].u.iFormat.imm;
                immSigned = static_cast<short>(imm);
                rt = instructions[progCounter].u.iFormat.rt;
                rs = instructions[progCounter].u.iFormat.rs;
                
                addrLoadStore = registerStore[rs] + immSigned;
                if ((addrLoadStore < numInst) && addrLoadStore >= 0)
                {
                    std::cerr 
			<< "load from instruction memory at address " 
				<< addrLoadStore << "\n";
                    outFile.close();
                    exit(EXIT_FAILURE);
                }
                if ((addrLoadStore < (numInst)) || (addrLoadStore >= 
				(numInst + numWords)))
                {
                    std::cerr << 
				"load outside of data memory at address " << 
					addrLoadStore << "\n";
                    outFile.close();
                    exit(EXIT_FAILURE);
                }
                if (rt != 0)
                {
                    registerStore[rt] = dataArray[addrLoadStore - numInst];
                }
                ++progCounter;
                break;
                
            case 43: 
                outFile << "inst: " << instStorage[progCounter] << "\n";
                imm = instructions[progCounter].u.iFormat.imm;
                immSigned = static_cast<short>(imm);
                rt = instructions[progCounter].u.iFormat.rt;
                rs = instructions[progCounter].u.iFormat.rs;
                
                addrLoadStore = registerStore[rs] + immSigned;
                if ((addrLoadStore < numInst) && addrLoadStore >= 0)
                {
                    std::cerr << "store to instruction memory at address " <<
				 addrLoadStore << "\n";
                    outFile.close();
                    exit(EXIT_FAILURE);
                }
                if ((addrLoadStore < (numInst)) || 
			(addrLoadStore >= (numInst + numWords)))
                {
                    std::cerr << "store outside of data memory at address " <<
				 addrLoadStore << "\n";
                    outFile.close();
                    exit(EXIT_FAILURE);
                }
                dataArray[addrLoadStore - numInst] = registerStore[rt];
                ++progCounter;
                break;

            default:
                std::cerr << "Something went wrong.";
                outFile.close();
                exit(EXIT_FAILURE);
                break;
        }
        
        int vzero;
        vzero = registerStore[2];
        exitCondition = (opcode == 0 && funct == 12 && vzero == 10);
        
        if ((progCounter) >= numInst && (!exitCondition))
        {
            if ((progCounter) < (numInst + numWords))
            {
                std::cerr << "PC is accessing data memory at address " << 
			(progCounter) << "\n";
            }
            else
            {
                std::cerr << "PC is accessing illegal memory address " <<
			(progCounter) << "\n";
            }
            printRegisterState(registerStore,outFile);
            outFile << "\n\n";
            printDataMemory(dataArray,numWords,outFile);
            outFile << "\n\n";
            outFile << "PC: " << (progCounter) << "\n";
            outFile.close();
            exit(EXIT_FAILURE);
        }
        
        if (!exitCondition)
        {
            printRegisterState(registerStore,outFile);
            outFile << "\n\n";
            printDataMemory(dataArray,numWords,outFile);
            outFile << "\n\n";
        }

    }
    
    outFile << "exiting simulator\n";
    outFile.close();
}

void printRegisterState(std::vector<int> & registerStore,
		 std::ofstream & outFile)
{
    outFile << std::left << "\nregs:\n";
    outFile << std::right;
    outFile << std::setw(10) << "$zero =" << std::setw(6) << registerStore[0];
    outFile << std::setw(10) << "$at =" << std::setw(6) << registerStore[1];
    outFile << std::setw(10) << "$v0 =" << std::setw(6) << registerStore[2];
    outFile << std::setw(10) << "$v1 =" << std::setw(6) << registerStore[3]
	 << "\n";
    outFile << std::setw(10) << "$a0 =" << std::setw(6) << registerStore[4];
    outFile << std::setw(10) << "$a1 =" << std::setw(6) << registerStore[5];
    outFile << std::setw(10) << "$a2 =" << std::setw(6) << registerStore[6];
    outFile << std::setw(10) << "$a3 =" << std::setw(6) << registerStore[7]
	<< "\n";
    outFile << std::setw(10) << "$t0 =" << std::setw(6) << registerStore[8];
    outFile << std::setw(10) << "$t1 =" << std::setw(6) << registerStore[9];
    outFile << std::setw(10) << "$t2 =" << std::setw(6) << registerStore[10];
    outFile << std::setw(10) << "$t3 =" << std::setw(6) << registerStore[11] 
	<< "\n";
    outFile << std::setw(10) << "$t4 =" << std::setw(6) << registerStore[12];
    outFile << std::setw(10) << "$t5 =" << std::setw(6) << registerStore[13];
    outFile << std::setw(10) << "$t6 =" << std::setw(6) << registerStore[14];
    outFile << std::setw(10) << "$t7 =" << std::setw(6) << registerStore[15] 
	<< "\n";
    outFile << std::setw(10) << "$s0 =" << std::setw(6) << registerStore[16];
    outFile << std::setw(10) << "$s1 =" << std::setw(6) << registerStore[17];
    outFile << std::setw(10) << "$s2 =" << std::setw(6) << registerStore[18];
    outFile << std::setw(10) << "$s3 =" << std::setw(6) << registerStore[19] 
	<< "\n";
    outFile << std::setw(10) << "$s4 =" << std::setw(6) << registerStore[20];
    outFile << std::setw(10) << "$s5 =" << std::setw(6) << registerStore[21];
    outFile << std::setw(10) << "$s6 =" << std::setw(6) << registerStore[22];
    outFile << std::setw(10) << "$s7 =" << std::setw(6) << registerStore[23] 
	<< "\n";
    outFile << std::setw(10) << "$t8 =" << std::setw(6) << registerStore[24];
    outFile << std::setw(10) << "$t9 =" << std::setw(6) << registerStore[25];
    outFile << std::setw(10) << "$k0 =" << std::setw(6) << registerStore[26];
    outFile << std::setw(10) << "$k1 =" << std::setw(6) << registerStore[27] 
	<< "\n";
    outFile << std::setw(10) << "$gp =" << std::setw(6) << registerStore[28];
    outFile << std::setw(10) << "$sp =" << std::setw(6) << registerStore[29];
    outFile << std::setw(10) << "$fp =" << std::setw(6) << registerStore[30];
    outFile << std::setw(10) << "$ra =" << std::setw(6) << registerStore[31] 
	<< "\n";
    outFile << std::setw(10) << "$lo =" << std::setw(6) << registerStore[32];
    outFile << std::setw(10) << "$hi =" << std::setw(6) << registerStore[33];
}

void printDataMemory(int * dataArray, size_t numWords, std::ofstream & outFile)
{
    outFile << std::left << "data memory:\n";
    outFile << std::right;
    
    for (size_t i = 0; i < numWords; ++i)
    {
        if ((i != 0) && (i % 3) == 0)
        {
            outFile << "\n";
        }
        outFile << std::setw(8) << "data[";
        outFile << std::setw(3) << i;
        outFile << "] =";
        outFile << std::setw(6) << dataArray[i];
        
    }
    outFile << "\n";
}
