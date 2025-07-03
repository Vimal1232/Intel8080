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

    uint8_t fetch()
    {
        uint8_t opcode = memory.read(PC);
        PC++;
        return opcode;
    }

    void decode(uint8_t opcode)
    {

        uint8_t *lookup_table[8] = {&B, &C, &D, &E, &H, &L, nullptr, &A};

        switch (opcode)
        {
        // MOV Opcode + Exception of HLT
        case 0x40 ... 0x7f:
            uint8_t Src = opcode & 0x07;
            uint8_t Dest = (opcode & 0x38) >> 3;

            if (opcode == 0x76)
            {
                std::cout << "Will implement Halt Later" << std::endl;
            }
            else
            {
                if (Src != 0x06 && Dest != 0x06)
                {
                    *lookup_table[Dest] = *lookup_table[Src];
                }
                else if (Src == 0x06)
                {
                    uint16_t HL = (static_cast<uint16_t>(H) << 8) | L;
                    *lookup_table[Dest] = memory.read(HL);
                }
                else if (Dest == 0x06)
                {
                    uint16_t HL = (static_cast<uint16_t>(H) << 8) | L;
                    memory.write(HL, *lookup_table[Src]);
                }
            }

            break;

            // Different OpCodes but Serve Same Purpose of MVI

        case 0x06:
        case 0x0e:
        case 0x16:
        case 0x1e:
        case 0x26:
        case 0x2e:
        case 0x36:
        case 0x3e:
            uint8_t Dest = (opcode & 0x38) >> 3;
            uint8_t Byte2Data = memory.read(PC);

            if (Dest != 0x06)
            {
                *lookup_table[Dest] = Byte2Data;
            }
            else
            {
                uint16_t HL = (static_cast<uint16_t>(H) << 8) | L;
                memory.write(HL, Byte2Data);
            }

            PC++;

            break;
        }
    }
};