#pragma once

#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <memory>

#include "auxlib/Types.h"

#include "Memory.h"

class Transputer {
  public:
    Transputer():
      hasPath(false),
      memPtr(std::make_unique<WriteableMemory>()) {
        reset();
    }

    Transputer(const char *filePath):
      Transputer() {
        loadProgram(filePath);
    }

    void reset() {
        tickCount = 0;

        I = 0;
        W = 0;
        O = 0;
        A = 0;
        B = 0;
        C = 0;

        memPtr->clearMemory();
    }

    void loadProgram(const char *filePath) {
        const int fd = open(filePath.c_str(), O_RDONLY);
        
        struct stat st;
        int retCode = fstat(fd, &st);

        if (retCode == 0) {
            off_t fileSize = st.st_size;
            instrBuf.resize(fileSize);
            read(fd, &instrBuf[0], fileSize);
        } else {
            throw BException("Cannot load program from file %s - "
                             "fstat failed with return code %d",
                             filePath, retCode);
        }

        close(fd);

        hasPath = true;
        instrPath = std::string(filePath);
    }

    void tick() {
        const u8 instr = instrBuf[I];
        doInstr(instr);
        tickCount++;
    }

    void dumpState(bool dumpMemory = false) {
        std::cerr << "===== REGISTERS =====\n";

        std::cerr << "I = " << I << "\n"
                  << "W = " << W << "\n"
                  << "O = " << O << "\n"
                  << "A = " << A << "\n"
                  << "B = " << B << "\n"
                  << "C = " << C << "\n";

        if (dumpMemory) {
            std::cerr << "===== MEMORY =====\n";
            memPtr->dumpContents();
        }
    }

  private:
    bool hasPath;
    std::string instrPath;    
    std::vector<u8> instrBuf;
    std::unique_ptr<WriteableMemory> memPtr;

    // Registers.
    u32 I = 0;
    u32 W = 0;
    u32 O = 0;
    u32 A, B, C = 0;

    u64 tickCount = 0;

    void doInstr(const u8 instrCode);
    void doOp(const u8 opCode);

    void advanceInstr() { I += 1; }
};
