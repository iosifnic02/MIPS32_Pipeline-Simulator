#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <cctype>
using namespace std;

//---------------------------
// Instruction types and control signals
//---------------------------
enum class InstrType {
    R_TYPE, I_TYPE, MEM_TYPE, BRANCH_TYPE, SHIFT_TYPE, NOP_TYPE
};

// Δημιουργία μιας δομής Monitor για κάθε στάδιο με 26 monitors
struct Monitor {

    //IF
    string monitor1;
    string monitor2;
    string monitor3;

    //ID
    string monitor4;
    string monitor5;
    string monitor6;
    string monitor7;
    string monitor8;
    string monitor9;
    string monitor10;
    string monitor11;
    string monitor12;
    string monitor13;
    string monitor14;
    string monitor15;

    //EX
    string monitor16;
    string monitor17;
    string monitor18;
    string monitor19;

    //MEM
    string monitor20;
    string monitor21;
    string monitor22;
    string monitor23;
    string monitor24;

    //WB
    string monitor25;
    string monitor26;

    // Κατασκευαστής για αρχικοποίηση όλων των πεδίων ως "-"
    Monitor() : monitor1("-"), monitor2("-"), monitor3("-"), monitor4("-"), monitor5("-"),
                monitor6("-"), monitor7("-"), monitor8("-"), monitor9("-"), monitor10("-"),
                monitor11("-"), monitor12("-"), monitor13("-"), monitor14("-"), monitor15("-"),
                monitor16("-"), monitor17("-"), monitor18("-"), monitor19("-"), monitor20("-"),
                monitor21("-"), monitor22("-"), monitor23("-"), monitor24("-"), monitor25("-"),
                monitor26("-") { }


};



struct ControlSignals {
    int RegDst;    // destination register select
    int Jump;      // jump signal
    int Branch;    // branch signal
    int MemRead;   // memory read signal
    int MemtoReg;  // memory-to-register signal
    int ALUOp;     // ALU operation code
    int MemWrite;  // memory write signal
    int ALUSrc;    // ALU source select (register or immediate)
    int RegWrite;  // register write signal
};

//---------------------------
// Structures for instructions and labels
//---------------------------
struct Instruction {
    string mnemonic;      // e.g., "add", "lw", "ori", etc.
    InstrType type;       // instruction type
    int rs = 0;
    int rt = 0;
    int rd = 0;
    int immediate = 0;
    string labelTarget;
    int PC = 0;
    string fullLine;      // the complete line as read (for output)
    int sltresult;
};

struct Label {
    string name;
    int address;
};

//---------------------------
// Memory class – holds values mapped by addresses
//---------------------------
class Memory {
public:
    map<int, int> mem;

    int read(int address) {
        if(mem.find(address) != mem.end())
            return mem[address];
        return 0;
    }

    void write(int address, int value) {
        mem[address] = value;
    }

    string getMemoryState() {
        if(mem.empty()) return "";
        vector<int> addresses;
        for(auto &p : mem)
            addresses.push_back(p.first);
        sort(addresses.begin(), addresses.end());
        ostringstream oss;
        for (size_t i = 0; i < addresses.size(); i++) {
            oss << toHex(mem[addresses[i]]);
            if(i < addresses.size() - 1)
                oss << "\t";
        }
        return oss.str();
    }

private:
    string toHex(int num) {
        unsigned int u = static_cast<unsigned int>(num);
        stringstream ss;
        ss << hex << nouppercase << u;
        return ss.str();
    }
};

//---------------------------
// CPU class – holds the register file
//---------------------------
class CPU {
public:
    vector<int> registers;
    vector<int> registersfinal;

    CPU() {
        registers.resize(33, 0);
        registersfinal.resize(33,0);
        registers[28] = 0x10008000;
        registers[29] = 0x7ffffffc;
        registersfinal[28] = 0x10008000;
        registersfinal[29] = 0x7ffffffc;
    }
};

//---------------------------
// Utility functions for parsing and formatting
//---------------------------
string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if(start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

vector<string> split(const string &s, const string &delims = " ,\t") {
    vector<string> tokens;
    size_t start = s.find_first_not_of(delims), end = 0;
    while(start != string::npos) {
        end = s.find_first_of(delims, start);
        tokens.push_back(s.substr(start, end - start));
        start = s.find_first_not_of(delims, end);
    }
    return tokens;
}

string removeComma(const string &s) {
    string res = s;
    res.erase(remove(res.begin(), res.end(), ','), res.end());
    return res;
}

int getRegisterNumber(const string &regStr) {
    string r = removeComma(regStr);
    if(!r.empty() && r[0]=='$')
        r = r.substr(1);
    if(r=="r0") return 0;
    if(r=="at") return 1;
    if(r=="v0") return 2;
    if(r=="v1") return 3;
    if(r=="a0") return 4;
    if(r=="a1") return 5;
    if(r=="a2") return 6;
    if(r=="a3") return 7;
    if(r=="t0") return 8;
    if(r=="t1") return 9;
    if(r=="t2") return 10;
    if(r=="t3") return 11;
    if(r=="t4") return 12;
    if(r=="t5") return 13;
    if(r=="t6") return 14;
    if(r=="t7") return 15;
    if(r=="s0") return 16;
    if(r=="s1") return 17;
    if(r=="s2") return 18;
    if(r=="s3") return 19;
    if(r=="s4") return 20;
    if(r=="s5") return 21;
    if(r=="s6") return 22;
    if(r=="s7") return 23;
    if(r=="t8") return 24;
    if(r=="t9") return 25;
    if(r=="k0") return 26;
    if(r=="k1") return 27;
    if(r=="gp") return 28;
    if(r=="sp") return 29;
    if(r=="fp") return 30;
    if(r=="ra") return 31;
    if(r=="zero") return 32;
    return -1;
}

string getRegisterName(int reg) {
    static vector<string> names = {"r0", "at", "v0", "v1", "a0", "a1", "a2", "a3",
                                   "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
                                   "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
                                   "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra", "zero"};
    if(reg >= 0 && reg < (int)names.size())
        return "$" + names[reg];
    return "$?";
}

int parseNumber(const string &str) {
    string s = trim(str);
    if(s.size() >= 2 && s[0]=='0' && (s[1]=='x' || s[1]=='X')) {
        try {
            return stoi(s.substr(2), nullptr, 16);
        }
        catch (const std::exception &e) {
            throw;
        }
    }
    else {
        try {
            return stoi(s, nullptr, 10);
        }
        catch (const std::exception &e) {
            throw;
        }
    }
}

int convertImmediate(const string &str) {
    return parseNumber(str);
}

string toHex(int num) {
    unsigned int u = static_cast<unsigned int>(num);
    stringstream ss;
    ss << hex << nouppercase << u;
    return ss.str();
}

// A helper to create a bubble (NOP) instruction
Instruction createBubble() {
    Instruction nop;
    nop.mnemonic = "NOP";
    nop.type = InstrType::NOP_TYPE;
    nop.PC = 0;
    nop.fullLine = "NOP";
    return nop;
}

//---------------------------
// Simulator class – with a 5‐stage pipeline and modified monitor formatting
//---------------------------
class Simulator {
private:
    vector<Instruction> instructions;
    vector<Label> labels;
    CPU cpu;
    Memory memory;
    int pc;          // internal program counter (not printed)
    int cycleCount;  // cycle counter
    bool branchTakenFlag = false;
    int branchTargetAddress = 0;
    int sllflagtelos=-1;
    string EXE_monitor16,EXE_monitor17;
    int ALU_RESULT;

     //Dimiourgia 5 monitor objects
     Monitor ifMonitor;
     Monitor idMonitor;
     Monitor exMonitor;
     Monitor memMonitor;
     Monitor wbMonitor;


    // Pipeline register structure (one per stage)
    struct PipelineRegister {
        Instruction instr;
        ControlSignals signals;
        int aluResult;
        bool valid;
        PipelineRegister() : instr(createBubble()), aluResult(0), valid(false) {
            signals = {0,0,0,0,0,0,0,0,0};
        }
    };

    PipelineRegister IF_reg, ID_reg, EX_reg, MEM_reg, WB_reg;

    //---------------------------
    // Parsing functions (similar to single-cycle version)
    //---------------------------
    void parseLine(const string &line, int currentPC) {
        string trimmed = trim(line);
        size_t commentPos = trimmed.find('#');
        if(commentPos != string::npos) {
            trimmed = trim(trimmed.substr(0, commentPos));
        }
        if(trimmed.empty() || trimmed[0]=='.')
            return;
        size_t colonPos = trimmed.find(':');
        if(colonPos != string::npos) {
            string labName = trim(trimmed.substr(0, colonPos));
            labels.push_back(Label{labName, currentPC});
            if(colonPos+1 < trimmed.size()){
                string rest = trim(trimmed.substr(colonPos+1));
                if(!rest.empty())
                    parseInstruction(rest, currentPC);
            }
        } else {
            parseInstruction(trimmed, currentPC);
        }
    }

    void parseInstruction(const string &instLine, int currentPC) {
        Instruction instr;
        instr.fullLine = instLine;
        instr.PC = currentPC;
        vector<string> tokens = split(instLine);
        if(tokens.empty()) return;
        instr.mnemonic = tokens[0];

        if(instr.mnemonic=="beq" || instr.mnemonic=="bne") {
            instr.type = InstrType::BRANCH_TYPE;
            if(tokens.size()>=4) {
                instr.rs = getRegisterNumber(tokens[1]);
                instr.rt = getRegisterNumber(tokens[2]);
                instr.labelTarget = removeComma(tokens[3]);
            }
        }
        else if(instr.mnemonic=="lw" || instr.mnemonic=="sw") {
            instr.type = InstrType::MEM_TYPE;
            if(tokens.size()>=3) {
                if(instr.mnemonic=="lw") {
                    instr.rd = getRegisterNumber(tokens[1]);
                } else {
                    instr.rd = getRegisterNumber(tokens[1]);
                }
                parseMemoryOperand(tokens[2], instr);
            }
        }
        else if(instr.mnemonic=="sll" || instr.mnemonic=="srl") {
            instr.type = InstrType::SHIFT_TYPE;
            if(tokens.size()>=4) {
                instr.rd = getRegisterNumber(tokens[1]);
                instr.rt = getRegisterNumber(tokens[2]);
                instr.immediate = parseNumber(tokens[3]);
            }
        }
        else if(instr.mnemonic=="add" || instr.mnemonic=="addu" ||
                instr.mnemonic=="sub" || instr.mnemonic=="subu" ||
                instr.mnemonic=="and" || instr.mnemonic=="or" ||
                instr.mnemonic=="nor" || instr.mnemonic=="slt" || instr.mnemonic=="sltu") {
            instr.type = InstrType::R_TYPE;
            if(tokens.size()>=4) {
                instr.rd = getRegisterNumber(tokens[1]);
                instr.rs = getRegisterNumber(tokens[2]);
                instr.rt = getRegisterNumber(tokens[3]);
            }
        }
        else if(instr.mnemonic=="addi" || instr.mnemonic=="addiu" ||
                instr.mnemonic=="andi" || instr.mnemonic=="ori" ||
                instr.mnemonic=="slti" || instr.mnemonic=="sltiu") {
            instr.type = InstrType::I_TYPE;
            if(tokens.size()>=4) {
                instr.rd = getRegisterNumber(tokens[1]);
                instr.rs = getRegisterNumber(tokens[2]);
                instr.immediate = convertImmediate(tokens[3]);
            }
        }
        else {
            instr.type = InstrType::I_TYPE;
        }
        instructions.push_back(instr);
        cout << "Parsed instruction: " << instr.fullLine << endl;
    }

    void parseMemoryOperand(const string &operand, Instruction &instr) {
        size_t lparen = operand.find('(');
        size_t rparen = operand.find(')');
        if(lparen != string::npos && rparen != string::npos) {
            string offsetStr = operand.substr(0, lparen);
            string baseStr = operand.substr(lparen+1, rparen - lparen - 1);
            instr.immediate = convertImmediate(trim(offsetStr));
            int base = getRegisterNumber(trim(baseStr));
            if(instr.mnemonic=="lw")
                instr.rs = base;
            else
                instr.rt = base;
        }
    }

    int getLabelAddress(const string &name) {
        for(auto &lab : labels)
            if(lab.name == name)
                return lab.address;
        return -1;
    }

    //---------------------------
    // Pipeline stage functions
    //---------------------------

    // EX_stage: perform ALU operations (without updating registers immediately)
    void EX_stage() {
        if(!EX_reg.valid || EX_reg.instr.mnemonic=="NOP") return;
        Instruction &instr = EX_reg.instr;
        ControlSignals &signals = EX_reg.signals;
        int aluResult = 0;
        int finalslt=EX_reg.instr.sltresult;
        vector<int> &reg = cpu.registers;
        signals = {0,0,0,0,0,0,0,0,0};

        if(instr.mnemonic=="add") {
            aluResult = reg[instr.rs] + reg[instr.rt];
            signals = {1,0,0,0,0,10,0,0,1};
        }
        else if(instr.mnemonic=="addi") {
            aluResult = reg[instr.rs] + instr.immediate;
            signals = {0,0,0,0,0,10,0,1,1};
        }
        else if(instr.mnemonic=="addiu") {
            unsigned int u1 = static_cast<unsigned int>(reg[instr.rs]);
            unsigned int u2 = static_cast<unsigned int>(instr.immediate);
            aluResult = u1 + u2;
            signals = {0,0,0,0,0,10,0,1,1};
        }
        else if(instr.mnemonic=="addu") {
            unsigned int u1 = static_cast<unsigned int>(reg[instr.rs]);
            unsigned int u2 = static_cast<unsigned int>(reg[instr.rt]);
            aluResult = u1 + u2;
            signals = {1,0,0,0,0,10,0,0,1};
        }
        else if(instr.mnemonic=="sub") {
            aluResult = reg[instr.rs] - reg[instr.rt];
            signals = {1,0,0,0,0,10,0,0,1};
        }
        else if(instr.mnemonic=="subu") {
            unsigned int u1 = static_cast<unsigned int>(reg[instr.rs]);
            unsigned int u2 = static_cast<unsigned int>(reg[instr.rt]);
            aluResult = u1 - u2;
            signals = {1,0,0,0,0,10,0,0,1};
        }
        else if(instr.mnemonic=="and") {
            aluResult = reg[instr.rs] & reg[instr.rt];
            signals = {1,0,0,0,0,0,0,0,1};
        }
        else if(instr.mnemonic=="andi") {
            aluResult = reg[instr.rs] & instr.immediate;
            signals = {0,0,0,0,0,0,0,1,1};
        }
        else if(instr.mnemonic=="or") {
            aluResult = reg[instr.rs] | reg[instr.rt];
            signals = {1,0,0,0,0,1,0,0,1};
        }
        else if(instr.mnemonic=="ori") {
            aluResult = reg[instr.rs] | instr.immediate;
            signals = {0,0,0,0,0,1,0,1,1};
        }
        else if(instr.mnemonic=="nor") {
            aluResult = ~(reg[instr.rs] | reg[instr.rt]);
            signals = {1,0,0,0,0,0,0,0,1};
        }
        else if(instr.mnemonic=="slt") {
            unsigned int z1 = static_cast<unsigned int>(reg[instr.rs]);
            unsigned int z2 = static_cast<unsigned int>(reg[instr.rt]);
            aluResult = (z1 < z2) ? 1 : 0;
            ALU_RESULT=z1-z2;
            if(aluResult==1)
                finalslt=z2-z1;
            else
                finalslt=z1-z2;


            signals = {1,0,0,0,0,11,0,0,1};
        }
        else if(instr.mnemonic=="sltu") {
            unsigned int u1 = static_cast<unsigned int>(reg[instr.rs]);
            unsigned int u2 = static_cast<unsigned int>(reg[instr.rt]);
            aluResult = (u1 < u2) ? 1 : 0;
            signals = {1,0,0,0,0,11,0,0,1};
        }
        else if(instr.mnemonic=="slti") {
            aluResult = (reg[instr.rs] < instr.immediate) ? 1 : 0;
            signals = {0,0,0,0,0,11,0,1,1};
        }
        else if(instr.mnemonic=="sltiu") {
            unsigned int u1 = static_cast<unsigned int>(reg[instr.rs]);
            unsigned int u2 = static_cast<unsigned int>(instr.immediate);
            aluResult = (u1 < u2) ? 1 : 0;
            signals = {0,0,0,0,0,11,0,1,1};
        }
        else if(instr.mnemonic=="sll") {
            aluResult = reg[instr.rt] << instr.immediate;
            signals = {0,0,0,0,0,10,0,1,1};
        }
        else if(instr.mnemonic=="srl") {
            aluResult = reg[instr.rt] >> instr.immediate;
            signals = {0,0,0,0,0,10,0,1,1};
        }
        else if(instr.mnemonic=="lw") {
            aluResult = reg[instr.rs] + instr.immediate;
            signals = {0,0,0,1,1,0,0,1,1};
        }
        else if(instr.mnemonic=="sw") {
            aluResult = reg[instr.rt] + instr.immediate;
            signals = {0,0,0,0,0,0,1,1,0};
        }
        EX_reg.aluResult = aluResult;
        EX_reg.instr.sltresult=finalslt;
    }

    // MEM_stage: for memory instructions (lw/sw)
    void MEM_stage() {
        if(!MEM_reg.valid || MEM_reg.instr.mnemonic=="NOP") return;
        Instruction &instr = MEM_reg.instr;
        if(instr.mnemonic=="lw") {
            MEM_reg.aluResult = memory.read(MEM_reg.aluResult);
        }
        else if(instr.mnemonic=="sw") {
            memory.write(MEM_reg.aluResult, cpu.registers[instr.rd]);
        }


    }

    // WB_stage: write back to registers if needed
    void WB_stage() {
        if(!WB_reg.valid || WB_reg.instr.mnemonic=="NOP") return;
        Instruction &instr = WB_reg.instr;
        if(WB_reg.signals.RegWrite && instr.mnemonic != "sw" &&
           instr.mnemonic != "j" && instr.mnemonic != "beq" && instr.mnemonic != "bne") {
            cpu.registers[instr.rd] = WB_reg.aluResult;
        }
        cpu.registersfinal[instr.rd] = WB_reg.aluResult;  // Ενημέρωση του νέου vector
    }

    bool ID_stage() {
        if (!ID_reg.valid || ID_reg.instr.mnemonic == "NOP")
            return false;
        Instruction &instr = ID_reg.instr;
        if (instr.mnemonic == "beq") {
            if (cpu.registers[instr.rs] == cpu.registers[instr.rt]) {
                int target = getLabelAddress(instr.labelTarget);
                if (target != -1) {
                    branchTakenFlag = true;
                    branchTargetAddress = target;
                }
            }
        }
        else if (instr.mnemonic == "bne") {
            if (cpu.registers[instr.rs] != cpu.registers[instr.rt]) {
                int target = getLabelAddress(instr.labelTarget);
                if (target != -1) {
                    branchTakenFlag = true;
                    branchTargetAddress = target;
                }
            }
        }

        return branchTakenFlag;
    }



    // IF_stage: fetch an instruction based on current PC
    void IF_stage() {
        int index = pc / 4;
        if(index < (int)instructions.size()) {
            IF_reg.instr = instructions[index];
            IF_reg.instr.PC = pc-4;
            IF_reg.valid = true;
            pc += 4;
        }
        else {
            IF_reg.instr = createBubble();
            IF_reg.valid = false;
        }
    }

            // Συνάρτηση για int τιμές
        string formatMonitorField(int value) {
            return toHex(value);
        }

        // Συνάρτηση για string τιμές
        string formatMonitorField(const string &value) {
            return value;
        }


 // IF-monitors
vector<string> computeIFMonitors() {
    // Δημιουργούμε vector 3 θέσεων, αρχικοποιημένο με "-"
    vector<string> mon(3, "-");
    // Monitor 1: Εάν είναι branch (beq/bne) εμφανίζεται ο target label, αλλιώς η τιμή του next pc
    if(ID_reg.instr.mnemonic == "beq" || ID_reg.instr.mnemonic == "bne")
        mon[0] = formatMonitorField(ID_reg.instr.labelTarget);
    else
        mon[0] = formatMonitorField(pc);

    // Monitor 2: current pc
    mon[1] = formatMonitorField(pc - 4);
    // Monitor 3: oli grammi
    mon[2] = formatMonitorField(IF_reg.instr.fullLine);

    return mon;
}



vector<string> computeIDMonitors() {
    // Δημιουργούμε vector 12 θέσεων (Monitors 4 έως 15), αρχικοποιημένο με "-"
    vector<string> mon(12, "-");

    if (ID_reg.valid && ID_reg.instr.mnemonic != "NOP") {
        // Monitor 4
        mon[0] = (ID_reg.instr.mnemonic == "beq" || ID_reg.instr.mnemonic == "bne")
                    ? formatMonitorField(ID_reg.instr.labelTarget)
                    : "-";

        if (ID_reg.instr.type == InstrType::R_TYPE || ID_reg.instr.type == InstrType::SHIFT_TYPE) {
            mon[1]  = getRegisterName(ID_reg.instr.rs);                         // Monitor 5
            mon[2]  = getRegisterName(ID_reg.instr.rt);                         // Monitor 6
            mon[3]  = getRegisterName(WB_reg.instr.rd);                         // Monitor 7
            mon[4]  = formatMonitorField(WB_reg.aluResult);                     // Monitor 8
            mon[5]  = formatMonitorField(cpu.registers[ID_reg.instr.rs]);        // Monitor 9
            EXE_monitor16 = formatMonitorField(cpu.registers[ID_reg.instr.rs]);
            mon[6]  = formatMonitorField(cpu.registers[ID_reg.instr.rt]);        // Monitor 10
            EXE_monitor17 = formatMonitorField(cpu.registers[ID_reg.instr.rt]);
            mon[7]  = "-";                                                     // Monitor 11
            mon[8]  = getRegisterName(ID_reg.instr.rs);                         // Monitor 12
            mon[9]  = getRegisterName(ID_reg.instr.rt);                         // Monitor 13
            mon[10] = getRegisterName(ID_reg.instr.rt);                         // Monitor 14
            mon[11] = getRegisterName(ID_reg.instr.rd);                         // Monitor 15
        }
        else if (ID_reg.instr.type == InstrType::I_TYPE) {
            mon[1]  = getRegisterName(ID_reg.instr.rs);                         // Monitor 5
            mon[2]  = "-";                                                     // Monitor 6
            mon[3]  = getRegisterName(WB_reg.instr.rd);                         // Monitor 7
            mon[4]  = formatMonitorField(WB_reg.aluResult);                     // Monitor 8
            mon[5]  = formatMonitorField(cpu.registers[ID_reg.instr.rs]);        // Monitor 9
            mon[6]  = "-";                                                     // Monitor 10
            mon[7]  = formatMonitorField(ID_reg.instr.immediate);               // Monitor 11
            idMonitor.monitor11 = formatMonitorField(ID_reg.instr.immediate);
            mon[8]  = getRegisterName(ID_reg.instr.rs);                         // Monitor 12
            mon[9]  = getRegisterName(ID_reg.instr.rd);                         // Monitor 13
            mon[10] = getRegisterName(ID_reg.instr.rd);                         // Monitor 14
            mon[11] = "-";                                                     // Monitor 15
        }
        else if (ID_reg.instr.type == InstrType::BRANCH_TYPE) {
            mon[1]  = getRegisterName(ID_reg.instr.rt);                         // Monitor 5
            mon[2]  = getRegisterName(ID_reg.instr.rt);                         // Monitor 6
            mon[3]  = getRegisterName(ID_reg.instr.rs);                         // Monitor 7
            mon[4]  = formatMonitorField(cpu.registers[ID_reg.instr.rs]);        // Monitor 8
            mon[5]  = formatMonitorField(cpu.registers[ID_reg.instr.rd]);        // Monitor 9
            mon[6]  = formatMonitorField(cpu.registers[ID_reg.instr.rt]);        // Monitor 10
            mon[7]  = "-";                                                     // Monitor 11
            mon[8]  = getRegisterName(ID_reg.instr.rs);                        // Monitor 12
            mon[9]  = getRegisterName(ID_reg.instr.rs);                         // Monitor 13
            mon[10] = getRegisterName(ID_reg.instr.rs);                         // Monitor 14
            mon[11] = "-";                                                     // Monitor 15
        }
        else if (ID_reg.instr.type == InstrType::MEM_TYPE) {
            if (ID_reg.instr.mnemonic == "lw") {
                mon[1]  = getRegisterName(ID_reg.instr.rs);                     // Monitor 5
                mon[2]  = getRegisterName(ID_reg.instr.rd);                     // Monitor 6
                mon[3]  = "-";                                                 // Monitor 7
                mon[4]  = "-";                                                 // Monitor 8
                mon[5]  = formatMonitorField(cpu.registersfinal[ID_reg.instr.rs]); // Monitor 9
                idMonitor.monitor9 = formatMonitorField(cpu.registersfinal[ID_reg.instr.rs]);
                mon[6]  = "-";                                                 // Monitor 10
                mon[7]  = formatMonitorField(ID_reg.instr.immediate);           // Monitor 11
                mon[8]  = formatMonitorField(cpu.registersfinal[ID_reg.instr.rt]); // Monitor 12
                idMonitor.monitor12 = formatMonitorField(cpu.registersfinal[ID_reg.instr.rt]);
                mon[9]  = formatMonitorField(ID_reg.instr.rd);                   // Monitor 13
                mon[10] = formatMonitorField(cpu.registersfinal[ID_reg.instr.rs]); // Monitor 14
                mon[11] = "-";                                                 // Monitor 15
            }
            else { // περίπτωση "sw"
                mon[1]  = getRegisterName(ID_reg.instr.rt);                     // Monitor 5
                mon[2]  = getRegisterName(ID_reg.instr.rd);                     // Monitor 6
                mon[3]  = getRegisterName(WB_reg.instr.rd);                     // Monitor 7
                mon[4]  = formatMonitorField(WB_reg.aluResult);                 // Monitor 8
                mon[5]  = formatMonitorField(cpu.registersfinal[ID_reg.instr.rt]); // Monitor 9
                mon[6]  = formatMonitorField(cpu.registersfinal[ID_reg.instr.rd]); // Monitor 10
                idMonitor.monitor10 = formatMonitorField(cpu.registersfinal[ID_reg.instr.rd]);
                mon[7]  = formatMonitorField(ID_reg.instr.immediate);           // Monitor 11
                mon[8]  = getRegisterName(ID_reg.instr.rt);                     // Monitor 12
                mon[9]  = getRegisterName(ID_reg.instr.rd);                     // Monitor 13
                mon[10] = getRegisterName(ID_reg.instr.rd);                     // Monitor 14
                mon[11] = "-";                                                 // Monitor 15
            }
        }
    }
    else {
        // Εάν η εντολή είναι NOP, γεμίζουμε όλα τα 12 πεδία με "-"
        mon.assign(12, "-");
    }
    return mon;
}





vector<string> computeEXMonitors() {
    // Δημιουργούμε vector 4 θέσεων για τα Monitors 16 έως 19
    vector<string> mon(4, "-");
    if (EX_reg.valid && EX_reg.instr.mnemonic != "NOP") {
        if (EX_reg.instr.type == InstrType::R_TYPE || EX_reg.instr.type == InstrType::SHIFT_TYPE) {
            mon[0] = formatMonitorField(cpu.registers[EX_reg.instr.rs]); // Monitor 16
            mon[1] = formatMonitorField(cpu.registers[EX_reg.instr.rt]); // Monitor 17
            mon[2] = formatMonitorField(cpu.registers[EX_reg.instr.rs]); // Monitor 18
            mon[3] = "-";                                               // Monitor 19
        }
        else if (EX_reg.instr.type == InstrType::I_TYPE) {
            mon[0] = formatMonitorField(cpu.registers[EX_reg.instr.rd]);      // Monitor 16
            mon[1] = formatMonitorField(EX_reg.instr.immediate);               // Monitor 17
            mon[2] = formatMonitorField(cpu.registers[EX_reg.instr.rd] + EX_reg.instr.immediate); // Monitor 18
            mon[3] = "-";                                                    // Monitor 19
        }
        else if (EX_reg.instr.type == InstrType::BRANCH_TYPE) {
            // Για branch απλά γεμίζουμε με "-"
            for (int i = 0; i < 4; i++) mon[i] = "-";
        }
        else if (EX_reg.instr.type == InstrType::MEM_TYPE) {
            if (EX_reg.instr.mnemonic == "lw") {
                mon[0] = idMonitor.monitor9;                             // Monitor 16
                exMonitor.monitor16 = formatMonitorField(cpu.registers[EX_reg.instr.rd]);
                mon[1] = formatMonitorField(EX_reg.instr.immediate);       // Monitor 17
                mon[2] = idMonitor.monitor9;                             // Monitor 18
                mon[3] = "-";                                            // Monitor 19
                string sum20 = idMonitor.monitor9 + formatMonitorField(EX_reg.instr.immediate);
                memMonitor.monitor20 = sum20;
            }
            else { // περίπτωση "sw"
                mon[0] = idMonitor.monitor9;                             // Monitor 16
                mon[1] = formatMonitorField(EX_reg.instr.immediate);       // Monitor 17
                mon[2] = formatMonitorField(EX_reg.instr.immediate);       // Monitor 18
                mon[3] = exMonitor.monitor16;                            // Monitor 19
            }
        }
    }
    else {
        // Εάν η εντολή είναι NOP, γεμίζουμε όλα τα 12 πεδία με "-"
        mon.assign(4, "-");
    }
    return mon;
}



vector<string> computeMEMMonitors() {
    // Δημιουργούμε vector με 5 θέσεις, αρχικοποιημένες ως "-"
    vector<string> mon(5, "-");

    if (MEM_reg.valid && MEM_reg.instr.mnemonic != "NOP") {
        if (MEM_reg.instr.type == InstrType::BRANCH_TYPE) {
            // Για branch, διατηρούμε τις τιμές "-"
        }
        else if (MEM_reg.instr.type == InstrType::R_TYPE || MEM_reg.instr.type == InstrType::SHIFT_TYPE) {
            // Monitor 20: -
            // Monitor 21: -
            // Monitor 22: -
            // Monitor 23: Τιμή του καταχωρητή που προκύπτει από το ID (π.χ. cpu.registers[ID_reg.instr.rs])
            mon[3] = formatMonitorField(cpu.registers[ID_reg.instr.rs]);
            // Monitor 24: Όνομα καταχωρητή προορισμού
            mon[4] = getRegisterName(ID_reg.instr.rd);
        }
        else if (MEM_reg.instr.type == InstrType::I_TYPE) {
            mon[3] = formatMonitorField(cpu.registers[MEM_reg.instr.rd+1]); // Monitor 23
            mon[4] = getRegisterName(MEM_reg.instr.rd);                   // Monitor 24
        }
        else if (MEM_reg.instr.type == InstrType::MEM_TYPE) {
            if (MEM_reg.instr.mnemonic == "lw") {
                // Monitor 20: Διεύθυνση μνήμης (ALU result στο MEM)
                mon[0] = memMonitor.monitor20;
                // Monitor 21: (για lw, δεν υπάρχει ανάγνωση δεδομένων εδώ) αφήνουμε "-"
                // Monitor 22: (για sw, εδώ χρησιμοποιούμε το wbMonitor.monitor21, αν και για lw είναι "-"
                mon[2] = wbMonitor.monitor21;
                // Monitor 23: -
                // Monitor 24: Όνομα καταχωρητή προορισμού
                mon[4] = getRegisterName(MEM_reg.instr.rd);
            }
            else { // περίπτωση "sw"
                mon[0] = memMonitor.monitor20;  // Monitor 20: Διεύθυνση μνήμης
                mon[1] = memMonitor.monitor21;  // Monitor 21: Τιμή που διαβάστηκε (π.χ. από προηγούμενο στάδιο)
                // Monitor 22, 23, 24: "-"
            }
        }
    }
    else {
        // Εάν η εντολή είναι NOP, γεμίζουμε όλα τα 12 πεδία με "-"
        mon.assign(5, "-");
    }
    return mon;
}





vector<string> computeWBMonitors() {
    // Δημιουργούμε vector με 2 θέσεις, αρχικοποιημένες ως "-"
    vector<string> mon(2, "-");

    if (WB_reg.valid && WB_reg.instr.mnemonic != "NOP") {
        if (WB_reg.instr.type == InstrType::BRANCH_TYPE) {
            // Για branch, διατηρούμε τις τιμές "-"
        }
        else {
            // Monitor 25: Όνομα προορισμού (destination register)
            mon[0] = getRegisterName(WB_reg.instr.rd);
            // Monitor 26: Τελική τιμή που εγγράφεται στον καταχωρητή (write-back value)
            mon[1] = formatMonitorField(WB_reg.aluResult);
        }
    }
    else {
        // Εάν η εντολή είναι NOP, γεμίζουμε όλα τα 12 πεδία με "-"
        mon.assign(2, "-");
    }
    return mon;
}



// Συνάρτηση που ενσωματώνει όλα τα monitor των σταδίων και τα τυπώνει μαζί:
void printAllMonitors( ofstream &out) {
    vector<string> ifMon = computeIFMonitors();
    vector<string> idMon = computeIDMonitors();
    vector<string> exMon = computeEXMonitors();
    vector<string> memMon = computeMEMMonitors();
    vector<string> wbMon = computeWBMonitors();



    // Τύπωση των monitor για το IF στάδιο:
    for (auto &s : ifMon)
        out << s << "\t";


    // Τύπωση των monitor για το ID στάδιο:
    for (auto &s : idMon)
        out << s << "\t";


    // Τύπωση των monitor για το EX στάδιο:
    for (auto &s : exMon)
        out << s << "\t";


    // Τύπωση των monitor για το MEM στάδιο:
    for (auto &s : memMon)
        out << s << "\t";


    // Τύπωση των monitor για το WB στάδιο:
    for (auto &s : wbMon)
        out << s << "\t";

}

     void pipelineformat()
     {
        if(IF_reg.instr.fullLine=="NOP")
        IF_reg.instr.fullLine="-";

        if(ID_reg.instr.fullLine=="NOP")
        ID_reg.instr.fullLine="-";

        if(EX_reg.instr.fullLine=="NOP")
        EX_reg.instr.fullLine="-";

        if(MEM_reg.instr.fullLine=="NOP")
        MEM_reg.instr.fullLine="-";

        if(WB_reg.instr.fullLine=="NOP")
        WB_reg.instr.fullLine="-";
     }
    //---------------------------
    // Print the state of the current cycle
    //---------------------------
    void printCycleState(int cycle, ofstream &out) {
        out << "-----Cycle " << cycle << "-----\n";
        out << "Registers: \n";
        // Use computed printed PC: starts at 0 and increases by 4 each cycle
        int printedPC = pc-4;
        out << toHex(printedPC) << "\t";
        for (int i = 0; i <  cpu.registersfinal.size()-1; i++) {
            out << toHex(cpu.registersfinal[i]) << "\t";
        }
        out << "\n\n";


        out << "Monitors:\n";
         printAllMonitors(out);


        out << "\n\n";

        out << "Memory State:\n" << memory.getMemoryState() << "\n\n";

        pipelineformat();

        out << "Pipeline Stages:\n";
        out <<IF_reg.instr.fullLine << "\t"<< ID_reg.instr.fullLine << "\t"<< EX_reg.instr.fullLine << "\t"<<MEM_reg.instr.fullLine<<"\t"<< WB_reg.instr.fullLine << "\n\n";
    }

public:
    Simulator() : pc(0), cycleCount(1) { }

    void loadInstructions(const string &filename) {
        ifstream fin(filename);
        if(!fin) {
            cerr << "Cannot open file: " << filename << endl;
            exit(1);
        }
        string line;
        int currentPC = 0;
        while(getline(fin, line)) {
            int before = instructions.size();
            parseLine(line, currentPC);
            int after = instructions.size();
            if(after > before)
                currentPC += 4;
        }
        fin.close();
    }

    // run: main simulation loop; shift pipeline registers and print state for selected cycles.
    void run(const vector<int> &printCycles, const string &outputFile,
             const string &studentName, const string &studentID, clock_t &startclock, clock_t endclock) {
        ofstream fout(outputFile);
        if(!fout) {
            cerr << "Cannot create output file: " << outputFile << endl;
            exit(1);
        }
        fout << "Name: " << studentName << "\n";
        fout << "ID: " << studentID << "\n\n";

        // Initialize pipeline registers as bubbles
        IF_reg = PipelineRegister();
        ID_reg = PipelineRegister();
        EX_reg = PipelineRegister();
        MEM_reg = PipelineRegister();
        WB_reg = PipelineRegister();

        // Fetch initial instruction into IF stage
         IF_stage();


              cycleCount++;

        // Run simulation until pipeline is empty
        while ((IF_reg.valid || ID_reg.valid || EX_reg.valid || MEM_reg.valid || WB_reg.valid) && (sllflagtelos==-1)) {
            WB_stage();
            MEM_stage();
            EX_stage();
            ID_stage();  // branch resolution here sets branchTakenFlag if needed



            // Shift pipeline registers
            WB_reg = MEM_reg;
            MEM_reg = EX_reg;
            EX_reg = ID_reg;
            ID_reg = IF_reg;

            IF_stage();


            // If a branch was taken, flush IF and ID registers and fetch from branch target.
            if (branchTakenFlag) {
                IF_reg = PipelineRegister();
                pc = branchTargetAddress; // update PC to branch target
                IF_stage();               // fetch the branch target instruction
                branchTakenFlag = false;
            }

          if(find(printCycles.begin(), printCycles.end(), cycleCount) != printCycles.end())
           {
                WB_stage();
                printCycleState(cycleCount, fout);
           }

           // Έλεγχος για τερματισμό όταν στο WB υπάρχει "sll zero zero 0"
            if (WB_reg.instr.mnemonic == "sll" &&
                WB_reg.instr.rd == getRegisterNumber("zero") &&
                WB_reg.instr.rt == getRegisterNumber("zero") &&
                WB_reg.instr.immediate == 0) {
                sllflagtelos=1;
            }
            cycleCount++;
        }
        printFinalState(fout);


         // stop timer and output the time
         endclock = clock();
         double duration = double(endclock - startclock) / CLOCKS_PER_SEC;
         double duration_in_ns = duration * 1.0e9;
         fout << duration_in_ns << "ns" << endl;
         fout << endl;
         fout.close();
    }


    void printFinalState(ofstream &out) {
        out << "-----Final State-----\n";
        out << "Registers:\n";
        int printedPC = (cycleCount - 1) * 4;
        out << toHex(pc-4) << "\t";
        for (int i = 0; i < 32; i++) {
            out << toHex(cpu.registers[i]) << "\t";
        }
        out << "\n\nMemory State:\n" << memory.getMemoryState() << "\n\n";
        out << "Total Cycles:\n" << (cycleCount - 1);
        out <<"\n\n";
        out << "Total Execution Time:" << endl;
    }
};

//---------------------------
// main: read user input and run the simulation
//---------------------------
int main(){
    clock_t startclock,endclock;
    int numPrintCycles;
    cout << "Enter the number of cycles you want to print: ";
    cin >> numPrintCycles;
    vector<int> printCycles(numPrintCycles);
    cout << "Enter the cycle numbers (separated by spaces): ";
    for (int i = 0; i < numPrintCycles; i++){
        cin >> printCycles[i];
    }

    //arxizo xronometro
    startclock=clock();

    string studentName = "Iosif Nicolaou";
    string studentID = "UC1060xxxxx";

    Simulator sim;
    sim.loadInstructions("pipe_no_hazard_2025.txt");
    sim.run(printCycles, "lab3_UC10xxxxx_OUTPUT.txt", studentName, studentID,startclock,endclock);

    cout << "Simulation complete.\n";
    return 0;
}
