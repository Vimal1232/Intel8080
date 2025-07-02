#pragma once
#include <cstdint>
#include <Memory.hpp>

class CPU
{
public:
    Memory &memory;

    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;
    uint8_t A;

    uint8_t PSW;

    uint16_t PC;
    uint16_t SP;

    CPU(Memory &mem) : memory(mem)
    {
        B = 0x0;
        C = 0x0;
        D = 0x0;
        E = 0x0;
        H = 0x0;
        L = 0x0;
        A = 0x0;

        PSW = 0x0;

        PC = 0x0000;
        SP = 0xFFFF;
    }


    uint8_t fetch(){
      return memory.read(PC);
    }

    void decode(uint8_t opcode){
        

    }




};