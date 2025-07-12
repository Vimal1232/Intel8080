#pragma once
#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>
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

        PSW = 00000010;

        PC = 0x0000;
        SP = 0xFFFF;
    }

    uint8_t fetch()
    {
        uint8_t opcode = memory.read(PC);
        PC++;
        return opcode;
    }


    void Flag_Set_ADD(uint8_t result8 , uint16_t result16 , uint8_t operand , uint8_t OriginalA , bool ADC = false , uint8_t Carry = 0 ){

            if(ADC){
                if((OriginalA & 0x0F) + (operand &0x0F) + Carry > 0x0F ){
                 PSW |= 0x10;
            } else {
                PSW &= ~0x10;
            }

            } else {
                 //Auxillary Carry for ADD

          if((OriginalA & 0x0F) + (operand &0x0F) > 0x0F ){
                 PSW |= 0x10;
            } else {
                PSW &= ~0x10;
            }
            
            }
       
          

            // ZERO Flag

            if((result8) == 0){
                PSW |= 0x40;
            } else {
                PSW &= ~0x40;
            }

            // Sign Flag

            if((result8 & 0x80)>> 7 == 0x01){
                PSW |= 0x80;
            } else {
                PSW &= ~0x80;
            }

            // Parity Flag
            int Count = 0;
            uint8_t Temp = result8;

            for (int i = 0 ; i < 8; i++) {
                if(Temp & 0x01){
                    Count++;
                }
                Temp >>= 0x01;
            }

            if(Count % 2 == 0){
                PSW |= 0x04;
            } else {
                PSW &= ~0x04;
            }

            // Carry Flag


            if(result16 > 0xFF){
                PSW |= 0x01;
            } else{
                PSW &= ~0x01;
            }


    }

    void decode(uint8_t opcode)
    {

        uint8_t *lookup_table[8] = {&B, &C, &D, &E, &H, &L, nullptr, &A};

        switch (opcode)
        {
        // MOV Opcode + Exception of HLT
        case 0x40 ... 0x7f:
        {
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
        }

        // Different OpCodes but Serve Same Purpose of MVI

        case 0x06:
        case 0x0e:
        case 0x16:
        case 0x1e:
        case 0x26:
        case 0x2e:
        case 0x36:
        case 0x3e:
        {
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

            // Different Opcode Serves Same Purpose OF LXI

        case 0x01:
        case 0x11:
        case 0x21:
        case 0x31:
        {
            uint8_t RP = (opcode & 0x30) >> 4;
            uint8_t lowOrder = memory.read(PC);
            uint8_t highOrder = memory.read(++PC);
            PC++;
            switch (RP)
            {
            case 0x00:
                B = highOrder;
                C = lowOrder;
                break;

            case 0x01:

                D = highOrder;
                E = lowOrder;
                break;

            case 0x02:

                H = highOrder;
                L = lowOrder;
                break;

            case 0x03:

                SP = highOrder << 8 | lowOrder;

                break;
            }

            break;
        }

        // For LDA There is only one Opcode
        case 0x3a:
        {
            uint8_t lowOrder = memory.read(PC);
            uint8_t highOrder = memory.read(++PC);
            PC++;

            uint16_t MemLoc = highOrder << 8 | lowOrder;

            uint8_t Data = memory.read(MemLoc);

            A = Data;

            break;
        }

            // Only One Opcode for STA

        case 0x32:
        {
            uint8_t lowOrder = memory.read(PC);
            uint8_t highOrder = memory.read(++PC);
            PC++;

            uint16_t MemLoc = highOrder << 8 | lowOrder;

            memory.write(MemLoc, A);
            break;
        }

        // Only One Opcode For LHLD 

        case 0x2a:{
            uint8_t lowOrder = memory.read(PC);
            uint8_t highOrder = memory.read(++PC);
            PC++;

            uint16_t MemLoc = highOrder << 8 | lowOrder;

            L = memory.read(MemLoc);
            H = memory.read(MemLoc +1);
           break;
        }

        // Only Single Opcode for SHLD 

        case 0x22:{
            uint8_t lowOrder = memory.read(PC);
            uint8_t highOrder = memory.read(++PC);
            PC++;

            uint16_t MemLoc = highOrder << 8 | lowOrder;

            memory.write(MemLoc , L);
            memory.write(MemLoc +1 , H);


            break;

        }

        // 2 Opcode for LDAX

        case 0x0a:
        case 0x1a:{
            uint8_t RP = (opcode & 0x30) >> 4;

            switch(RP){
                case 0x00:{
                    uint16_t MemLoc = B << 8 | C ;
                    A = memory.read(MemLoc);
                    break;
                }

                case 0x01:{
                     uint16_t MemLoc = D << 8 | E;
                    A = memory.read(MemLoc);
                    break;
                }
            }
            break;
        }

        // 2 opcode for STAX

        case 0x02:
        case 0x12:{
            uint8_t RP = (opcode & 0x30) >> 4;

            switch (RP) {
            case 0x00:{
              uint16_t MemLOC = B << 8 | C;
              memory.write(MemLOC,A);
              break;
             }
             case 0x01:{
              uint16_t MemLOC = D << 8 | E;
              memory.write(MemLOC,A);
              break;
             }
            }
            break;
        }


        // Single Opcode For XCHG

        case 0xeb:{
            uint8_t temp = H;

            H = D;
            D = temp;

            temp = L;

            L = E;
            E = temp;

            break;
        }

        // 8 Opcode for ADD

        case 0x80 ... 0x87:{
            uint8_t Src = opcode & 0x07;
            uint8_t Operand;
            uint8_t originalA = A;

            if(Src == 0x06){
                uint16_t MemLoc = H << 8 | L;
                 Operand = memory.read(MemLoc);
            } else {
                Operand = *lookup_table[Src];
            }
            uint16_t Result16 = A + Operand;
            uint8_t Result8 = Result16 & 0xFF;
            A = Result8;

            Flag_Set_ADD(Result8, Result16, Operand, originalA);
            break;
        }

        // 1 Opcode For ADI

        case 0xc6:{
            uint8_t Data = memory.read(PC);
            PC++;

            uint8_t originalA = A;
            uint16_t Result16 = A + Data;
            uint8_t Result8 = Result16 & 0xFF;
            A = Result8;

            Flag_Set_ADD(Result8, Result16, Data, originalA);
            break;

        }


        // 8 Opcodes For ADC

        case 0x88 ... 0x8f:{
            uint8_t src = opcode & 0x07;
            uint8_t Operand;
            uint8_t originalA = A;
            uint8_t carry = PSW & 0x01;

            if(src == 0x06){
                uint16_t MemLoc = H << 8 | L;
                Operand = memory.read(MemLoc);
            } else {
                Operand = *lookup_table[src];
            }

            uint16_t Result16 = A + Operand + carry;
            uint8_t Result8 = Result16 & 0xFF;
            A = Result8;

            Flag_Set_ADD(Result8, Result16,  Operand + carry,  originalA , true,  carry);

            break;
        }













        }
    }
};