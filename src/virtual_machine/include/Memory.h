#pragma once

#include <cassert>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <string>

#include "auxlib/BException.h"
#include "auxlib/Types.h"

class Memory {
  public:
    u32 __attribute__((always_inline))
    getSize() { 
        return memData.size();
    }

    void __attribute__((always_inline)) 
    checkByteAccess(const u32 byteIdx) {
        if (byteIdx >= getSize())
            throw BException("Attempted to read byte from memory at position "
                             "%lu, which is out of bounds.", byteIdx);
    }

    void __attribute__((always_inline))
    checkWordAccess(const u32 addr) {
        if (addr >= getSize() - 3)
            throw BException("Attempted to read word from memory at position "
                             "%lu, which is out of bounds.", addr);

        if ((addr & 3) != 0)
            throw BException("Attempted to read word from memory at position "
                             "%lu, which is not aligned.", addr);
    }

    void dumpContents() {
        for (u32 i = 0; i < )
    }

  protected:
    std::vector<u8> memData;
}

class ReadableMemory: public Memory {
  public:
    ReadableMemory = default;

    u8 readByte(const u32 byteIdx) {
        checkByteAccess(byteIdx);

        return memData[byteIdx]; 
    }

    u32 readWord(const u32 addr) {
        checkWordAccess(addr);

        u32 word = 0;
        for (int i = 0; i < 4; ++i) {
            word |= getByte(addr + i);
            word <<= 8;
        } 

        return word;
    }  
};

class WriteableMemory: public ReadableMemory {
  public:
    WriteableMemory = default;

    WriteableMemory(const int N):
      memData(std::vector<u8>(N, 0)) {}
      
    void writeByte(const u32 byteIdx, const u8 byte) { 
        checkByteAccess(byteIdx);

        mem[byteIdx] = byte;
    }

    void setWord(const u32 addr, u32 word) {
        checkWordAccess(addr);

        for (int i = 0; i < 4; ++i) {
            mem[addr + i] = word & 0xFF;
            word >>= 8;
        }
    }

    void clearMemory() {
        for (u8 &byte: memData) {
            byte = 0x0;
        }
    }
};
