#pragma once

#include <array>
#include <algorithm>
#include <charconv>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <stdexcept>
#include <variant>
#include <vector>

#include "auxlib/BException.h"
#include "auxlib/Map.h"
#include "auxlib/Types.h"

using namespace std::literals::string_view_literals;

struct Label;
class Instruction;

typedef std::variant<Label*, Instruction*> Line;

class Assembler {
  friend class Instruction;
  friend class Label;

  public: 
    static constexpr u8 MAX_LINE_LENGTH = 255;

    Assembler(const char *_fileIn, const char *_fileOut):
      fileIn(_fileIn),
      fileOut(_fileOut) {}

    /// Run the assembler on the given input and output files.
    void run();

    /// Get a pointer to the Label object described by @labelText.
    Label* getLabel(const std::string& labelText) { 
        return &allLabels.at(labelText);
    }

    /// Generate the proper prefix sequence for applying the immediate value
    /// @vImm to the instruction @instrCode, storing the result in @instrBuf.
    static void genPrefixSeq(
      const u8 instrCode, 
      const u32 vImm, 
      std::vector<u8>& instrBuf) {
        if (vImm >= 16) genPrefixSeq(pfixCode, vImm >> 4, instrBuf);
        else if (vImm < 0) genPrefixSeq(nfixCode, (~vImm) >> 4, instrBuf);
        instrBuf.push_back(instrCode | (vImm & 0xF));
    }

  private:
    std::ifstream fileIn;
    std::ofstream fileOut;

    std::vector<Instruction> allInstr;
    std::map<std::string, Label> allLabels;
    std::vector<Line> allLines;

    /// Maps instructions to their respective codes, available at compile time
    /// as a linear search for optimal performance.
    static constexpr auto instrMap = Map<std::string_view, u8, 16>{{
      std::array<std::pair<std::string_view, u8>, 16> {{
        {"pfix"sv,  0x0}, // Prefix.
        {"nfix"sv,  0x1}, // Negative prefix.
        {"opr"sv,   0x2}, // Operate.
        {"ldl"sv,   0x3}, // Load local.
        {"stl"sv,   0x4}, // Store local.
        {"ldlp"sv,  0x5}, // Load local pointer.
        {"ldc"sv,   0x6}, // Load constant.
        {"adc"sv,   0x7}, // Add constant.
        {"eqc"sv,   0x8}, // Equals constant.
        {"j"sv,     0x9}, // Jump.
        {"cj"sv,    0xA}, // Conditional jump.
        {"ldnl"sv,  0xB}, // Load not local.
        {"stnl"sv,  0xC}, // Store not local.
        {"ldnlp"sv, 0xD}, // Load not local pointer.
        {"call"sv,  0xE}, // Call.
        {"ajw"sv,   0xF}, // Adjust workspace.
      }}
    }};


    /* ========== Common Instruction Codes ========== */
    static constexpr u8 pfixCode = instrMap.at("pfix");
    static constexpr u8 nfixCode = instrMap.at("nfix");
    static constexpr u8 oprCode  = instrMap.at("opr");
    static constexpr u8 jCode    = instrMap.at("j");
    static constexpr u8 cjCode   = instrMap.at("cj");
    static constexpr u8 noopCode = (oprCode << 4);

    // Maps operations to their respective codes, available at compile time
    // through a linear search. This allows the compiler to optimize.
    static constexpr auto operMap = Map<std::string_view, u8, 14>{{
      std::array<std::pair<std::string_view, u8>, 14> {{
        {"noop"sv, 0x0}, // No-op.
        {"rev"sv,  0x1}, // Reverse.
        {"eqz"sv,  0x2}, // Equals zero.
        {"gt"sv,   0x3}, // Greater than.
        {"and"sv,  0x4}, // And.
        {"or"sv,   0x5}, // Or.
        {"xor"sv,  0x6}, // Xor.
        {"and"sv,  0x7}, // And.
        {"sub"sv,  0x8}, // Subtract.
        {"mul"sv,  0x9}, // Multiply.
        {"div"sv,  0xA}, // Divide.
        {"mod"sv,  0xB}, // Modulo.
        {"shl"sv,  0xC}, // Shift left.
        {"shr"sv,  0xD}, // Shift right. 
      }}
    }};

    /// First pass over the file. Fetch all labels and store them inside
    /// the @allLabels map.
    void scanLabels();

    /// Second pass over the file. Extracts instructions and links jumps
    /// with their label.
    void scanInstructions();

    /// Third pass over the file. Calculates the current byte offset in
    /// the executable as it goes along, updates the offset for labels.
    void calculateOffsets();

    /// Assemble and output all instructions.
    void assemble();
};

/// Labels hold references to places in the assembly file.
struct Label {
    const std::string text;
    u32 offset;

    Label(const std::string& _text):
      text(_text) {
        offset = 0;
    }
};

/// Holds an instruction ready for assembly. All instructions hold an 8-bit
/// code, which encodes their meaning. By structure, they can be further 
/// subdivided into:
/// |  
/// * Immediate value instructions:
/// |   These perform some general function with a given 32-bit immediate 
/// |   value.
/// |   e.g. stl, pfix, call, ldnlp
/// * Operate instructions:
/// |   These perform a (generally arithmetic) function on the registers, 
/// |   specified by the 8-bit opCode.
/// |   e.g. opr noop, opr add, opr shl
/// * Jumps:
///     These contain a pointer to a label, which is transformed into 
///     an offset at assembly time.
///     e.g. j, cj
///
/// Additionally, we store the estimated size of the assembled instruction
/// alongside it. This is useful in the case of optimizing jumps.
class Instruction {
  friend class Assembler;

  public:
    u8 instrCode;
    u8 estSize;
    union {
        u8 opCode;
        u32 imm;
        Label *labelPtr;
    } instrVal;

    /// We only permit the construction of instructions from string_views,
    /// which point to the description and argument of the instruction
    /// respectively.
    explicit Instruction(
      const std::string& instrDesc, 
      const std::string& instrRest) {
        try {
            estSize = 0;
            instrCode = Assembler::instrMap.at(instrDesc);
            if (isJump()) instrVal.labelPtr = assemblerPtr->getLabel(instrRest);
            else if (hasImmediate()) instrVal.imm = std::stoi(instrRest);
            else instrVal.opCode = Assembler::operMap.at(instrRest);
        } catch(const std::exception& e) {
            std::cerr << e.what() << "\n";
            throw BException(
                "Cannot assemble instruction '%s %s' - parsing of "
                "instrDesc and instrVal failed.",
                instrDesc.c_str(), instrRest.c_str());
        }
    }

    /// Assemble an instruction at a given offset and produce a vector of
    /// bytes.
    std::vector<u8> assemble(const u32 atOffset) const {
        std::vector<u8> assembledInstr;

        if (hasImmediate()) { 
            const u32 imm = instrVal.imm;
            Assembler::genPrefixSeq(instrCode, imm, assembledInstr);
        } else if (isOperation()) {
            const u8 opCode = instrVal.opCode;
            Assembler::genPrefixSeq(instrCode, opCode, assembledInstr);
        } else {
            const u32 labelOffset = instrVal.labelPtr->offset;
            const u32 deltaOffset = atOffset - labelOffset;
            Assembler::genPrefixSeq(instrCode, deltaOffset, assembledInstr);
            
            // Assembling jump instructions is tricky as we are encoding the
            // jump length with pfix / nfix instructions. With interlocking 
            // jumps, the number of instructions required to encode one jump can
            // depend in a circular way on the number of instructions required 
            // to encode other jumps.
            //
            // The cleanest compromise is to encode each jump with a fixed 
            // number of instructions, and pad to this number with no-ops. 
            // Eight is the simplest choice here, since it allows jumps in the 
            // range [-2^31, 2^31 - 1].
            for (size_t i = assembledInstr.size(); i < 8; i++) {
                assembledInstr.push_back(Assembler::noopCode);
            }
        }

        return assembledInstr;
    }

    /// Computes the size, in bytes, that the instruction will have once fully
    /// assembled. Provides an upper bound instead for jumps.
    u8 estimateSize() {
        if (isJump()) { return 8; }
        else if (isOperation()) { return 1 + (instrVal.opCode >= 16); }
        else { return 1 + (32 - __builtin_clz(instrVal.imm)) / 4; }
    }

    /// Returns true if the instruction has an immediate value.
    bool hasImmediate() const { 
        return !isJump() && !isOperation(); 
    }
    
    /// Returns true if the instruction is an operation.
    bool isOperation() const { 
        return instrCode == Assembler::oprCode; 
    }
    
    /// Returns true if the instruction is a jump.
    bool isJump() const { 
        return instrCode == Assembler::jCode || instrCode == Assembler::cjCode; 
    }

  private:
    inline static std::unique_ptr<Assembler> assemblerPtr = nullptr;
};
