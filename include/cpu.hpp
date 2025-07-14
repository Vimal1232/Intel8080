#pragma once
#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>

#include <Memory.hpp>
#include <cstdint>

class CPU {
 public:
  Memory& memory;

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

  CPU(Memory& mem) : memory(mem) {
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

  uint8_t fetch() {
    uint8_t opcode = memory.read(PC);
    PC++;
    return opcode;
  }

  void Flag_Set_ALU(uint8_t result8, uint16_t result16, uint8_t operand,
                    uint8_t OriginalA, bool SUB = false) {
    if (SUB) {
      if ((OriginalA & 0x0F) < (operand & 0x0F)) {
        PSW |= 0x10;
      } else {
        PSW &= ~0x10;
      }
    } else {
      if ((OriginalA & 0x0F) + (operand & 0x0F) > 0x0F) {
        PSW |= 0x10;
      } else {
        PSW &= ~0x10;
      }
    }

    // ZERO Flag

    if ((result8) == 0) {
      PSW |= 0x40;
    } else {
      PSW &= ~0x40;
    }

    // Sign Flag

    if ((result8 & 0x80) >> 7 == 0x01) {
      PSW |= 0x80;
    } else {
      PSW &= ~0x80;
    }

    // Parity Flag
    int Count = 0;
    uint8_t Temp = result8;

    for (int i = 0; i < 8; i++) {
      if (Temp & 0x01) {
        Count++;
      }
      Temp >>= 0x01;
    }

    if (Count % 2 == 0) {
      PSW |= 0x04;
    } else {
      PSW &= ~0x04;
    }

    // Carry Flag

    if (SUB) {
      if (operand > OriginalA) {
        PSW |= 0x01;
      } else {
        PSW &= ~0x01;
      }

    } else {
      if (result16 > 0xFF) {
        PSW |= 0x01;
      } else {
        PSW &= ~0x01;
      }
    }
  }

  void Flag_INDC(uint8_t result8, uint8_t auxdata, bool DCR = false) {
    if (DCR) {
      if ((auxdata & 0x0F) < 1) {
        PSW |= 0x10;
      } else {
        PSW &= ~0x10;
      }
    } else {
      if ((auxdata & 0x0F) + 1 > 0x0F) {
        PSW |= 0x10;
      } else {
        PSW &= ~0x10;
      }
    }

    // ZERO Flag
    if ((result8) == 0) {
      PSW |= 0x40;
    } else {
      PSW &= ~0x40;
    }

    // Sign Flag

    if ((result8 & 0x80) >> 7 == 0x01) {
      PSW |= 0x80;
    } else {
      PSW &= ~0x80;
    }

    // Parity Flag
    int Count = 0;
    uint8_t Temp = result8;

    for (int i = 0; i < 8; i++) {
      if (Temp & 0x01) {
        Count++;
      }
      Temp >>= 0x01;
    }

    if (Count % 2 == 0) {
      PSW |= 0x04;
    } else {
      PSW &= ~0x04;
    }
  }

  enum Instruction {
    ANA,
    XRA,
    ORA,
  };

  void Flag_Setting_Logical(uint8_t result, Instruction instruction) {
    // Auxillary FLag

    switch (instruction) {
      case ANA: {
        PSW |= 0x10;
        break;
      }

      case ORA: {
        PSW &= ~0x10;
        break;
      }

      case XRA: {
        PSW &= ~0x10;
        break;
      }
    }

    // Carry Flag Reset
    PSW &= ~0x01;

    // ZERO Flag
    if ((result) == 0) {
      PSW |= 0x40;
    } else {
      PSW &= ~0x40;
    }

    // Sign Flag

    if ((result & 0x80) >> 7 == 0x01) {
      PSW |= 0x80;
    } else {
      PSW &= ~0x80;
    }

    // Parity Flag
    int Count = 0;
    uint8_t Temp = result;

    for (int i = 0; i < 8; i++) {
      if (Temp & 0x01) {
        Count++;
      }
      Temp >>= 0x01;
    }

    if (Count % 2 == 0) {
      PSW |= 0x04;
    } else {
      PSW &= ~0x04;
    }
  }

  void decode(uint8_t opcode) {
    uint8_t* lookup_table[8] = {&B, &C, &D, &E, &H, &L, nullptr, &A};

    switch (opcode) {
      // MOV Opcode + Exception of HLT
      case 0x40 ... 0x7f: {
        uint8_t Src = opcode & 0x07;
        uint8_t Dest = (opcode & 0x38) >> 3;

        if (opcode == 0x76) {
          std::cout << "Will implement Halt Later" << std::endl;
        } else {
          if (Src != 0x06 && Dest != 0x06) {
            *lookup_table[Dest] = *lookup_table[Src];
          } else if (Src == 0x06) {
            uint16_t HL = (static_cast<uint16_t>(H) << 8) | L;
            *lookup_table[Dest] = memory.read(HL);
          } else if (Dest == 0x06) {
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
      case 0x3e: {
        uint8_t Dest = (opcode & 0x38) >> 3;
        uint8_t Byte2Data = memory.read(PC);

        if (Dest != 0x06) {
          *lookup_table[Dest] = Byte2Data;
        } else {
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
      case 0x31: {
        uint8_t RP = (opcode & 0x30) >> 4;
        uint8_t lowOrder = memory.read(PC);
        uint8_t highOrder = memory.read(++PC);
        PC++;
        switch (RP) {
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
      case 0x3a: {
        uint8_t lowOrder = memory.read(PC);
        uint8_t highOrder = memory.read(++PC);
        PC++;

        uint16_t MemLoc = highOrder << 8 | lowOrder;

        uint8_t Data = memory.read(MemLoc);

        A = Data;

        break;
      }

        // Only One Opcode for STA

      case 0x32: {
        uint8_t lowOrder = memory.read(PC);
        uint8_t highOrder = memory.read(++PC);
        PC++;

        uint16_t MemLoc = highOrder << 8 | lowOrder;

        memory.write(MemLoc, A);
        break;
      }

        // Only One Opcode For LHLD

      case 0x2a: {
        uint8_t lowOrder = memory.read(PC);
        uint8_t highOrder = memory.read(++PC);
        PC++;

        uint16_t MemLoc = highOrder << 8 | lowOrder;

        L = memory.read(MemLoc);
        H = memory.read(MemLoc + 1);
        break;
      }

        // Only Single Opcode for SHLD

      case 0x22: {
        uint8_t lowOrder = memory.read(PC);
        uint8_t highOrder = memory.read(++PC);
        PC++;

        uint16_t MemLoc = highOrder << 8 | lowOrder;

        memory.write(MemLoc, L);
        memory.write(MemLoc + 1, H);

        break;
      }

        // 2 Opcode for LDAX

      case 0x0a:
      case 0x1a: {
        uint8_t RP = (opcode & 0x30) >> 4;

        switch (RP) {
          case 0x00: {
            uint16_t MemLoc = B << 8 | C;
            A = memory.read(MemLoc);
            break;
          }

          case 0x01: {
            uint16_t MemLoc = D << 8 | E;
            A = memory.read(MemLoc);
            break;
          }
        }
        break;
      }

        // 2 opcode for STAX

      case 0x02:
      case 0x12: {
        uint8_t RP = (opcode & 0x30) >> 4;

        switch (RP) {
          case 0x00: {
            uint16_t MemLOC = B << 8 | C;
            memory.write(MemLOC, A);
            break;
          }
          case 0x01: {
            uint16_t MemLOC = D << 8 | E;
            memory.write(MemLOC, A);
            break;
          }
        }
        break;
      }

        // Single Opcode For XCHG

      case 0xeb: {
        uint8_t temp = H;

        H = D;
        D = temp;

        temp = L;

        L = E;
        E = temp;

        break;
      }

        // 8 Opcode for ADD

      case 0x80 ... 0x87: {
        uint8_t Src = opcode & 0x07;
        uint8_t Operand;
        uint8_t originalA = A;

        if (Src == 0x06) {
          uint16_t MemLoc = H << 8 | L;
          Operand = memory.read(MemLoc);
        } else {
          Operand = *lookup_table[Src];
        }
        uint16_t Result16 = A + Operand;
        uint8_t Result8 = Result16 & 0xFF;
        A = Result8;

        Flag_Set_ALU(Result8, Result16, Operand, originalA);
        break;
      }

        // 1 Opcode For ADI

      case 0xc6: {
        uint8_t Data = memory.read(PC);
        PC++;

        uint8_t originalA = A;
        uint16_t Result16 = A + Data;
        uint8_t Result8 = Result16 & 0xFF;
        A = Result8;

        Flag_Set_ALU(Result8, Result16, Data, originalA);
        break;
      }

        // 8 Opcodes For ADC

      case 0x88 ... 0x8f: {
        uint8_t src = opcode & 0x07;
        uint8_t Operand;
        uint8_t originalA = A;
        uint8_t carry = PSW & 0x01;

        if (src == 0x06) {
          uint16_t MemLoc = H << 8 | L;
          Operand = memory.read(MemLoc);
        } else {
          Operand = *lookup_table[src];
        }

        uint16_t Result16 = A + Operand + carry;
        uint8_t Result8 = Result16 & 0xFF;
        A = Result8;

        Flag_Set_ALU(Result8, Result16, Operand + carry, originalA);

        break;
      }

        // 1 Opcode for ACI

      case 0xce: {
        uint8_t Data = memory.read(PC);
        PC++;
        uint8_t carry = PSW & 0x01;
        uint8_t originalA = A;

        uint16_t Result16 = A + Data + carry;
        uint8_t Result8 = Result16 & 0xFF;
        A = Result8;

        Flag_Set_ALU(Result8, Result16, Data + carry, originalA);

        break;
      }

        // 8 Opcodes For SUB

      case 0x90 ... 0x97: {
        uint8_t src = opcode & 0x07;
        uint8_t Operand;
        uint8_t originalA = A;

        if (src == 0x06) {
          uint16_t MemLoc = H << 8 | L;
          Operand = memory.read(MemLoc);
        } else {
          Operand = *lookup_table[src];
        }
        uint16_t Result16 = A - Operand;
        uint8_t Result8 = Result16 & 0xFF;
        A = Result8;

        Flag_Set_ALU(Result8, Result16, Operand, originalA, true);

        break;
      }

        // 1 Opcode for SUI

      case 0xd6: {
        uint8_t Data = memory.read(PC);
        PC++;
        uint8_t originalA = A;
        uint16_t Result16 = A - Data;
        uint8_t Result8 = Result16 & 0xFF;
        A = Result8;

        Flag_Set_ALU(Result8, Result16, Data, originalA, true);

        break;
      }

        // 8 Opcodes For SBB

      case 0x98 ... 0x9f: {
        uint8_t src = opcode & 0x07;
        uint8_t Borrow = PSW & 0x01;
        uint8_t Operand;
        uint8_t originalA = A;

        if (src == 0x06) {
          uint16_t MemLoc = H << 8 | L;
          Operand = memory.read(MemLoc);
        } else {
          Operand = *lookup_table[src];
        }

        uint16_t Result16 = A - (Operand - Borrow);
        uint8_t Result8 = Result16 & 0xFF;
        A = Result8;

        Flag_Set_ALU(Result8, Result16, Operand - Borrow, originalA, true);

        break;
      }
        // 1 opcode of SBI

      case 0xde: {
        uint8_t Data = memory.read(PC);
        PC++;
        uint8_t originalA = A;
        uint8_t Borrow = PSW & 0x01;
        uint16_t Result16 = A - (Data - Borrow);
        uint8_t Result8 = Result16 & 0xff;
        A = Result8;

        Flag_Set_ALU(Result8, Result16, Data - Borrow, originalA, true);

        break;
      }

        // 8 Opcodes FOR INR

      case 0x04:
      case 0x14:
      case 0x24:
      case 0x34:
      case 0x0c:
      case 0x1c:
      case 0x2c:
      case 0x3c: {
        uint8_t Dest = (opcode & 0x38) >> 3;
        uint8_t Data;
        uint8_t AUXDATA;

        if (Dest == 0x06) {
          uint16_t MemLoc = H << 8 | L;
          Data = memory.read(MemLoc);
          AUXDATA = Data;
          memory.write(MemLoc, Data + 1);
        } else {
          Data = *lookup_table[Dest];
          AUXDATA = Data;
          *lookup_table[Dest] = Data + 1;
        }
        uint8_t Result = Data + 1;

        Flag_INDC(Result, AUXDATA, true);

        break;
      }

        // 8 opcodes For DCR

      case 0x05:
      case 0x15:
      case 0x25:
      case 0x35:
      case 0x0d:
      case 0x1d:
      case 0x2d:
      case 0x3d: {
        uint8_t dest = (opcode & 0x38) >> 3;
        uint8_t Data;
        uint8_t AUXDATA;

        if (dest == 0x06) {
          uint16_t MemLoc = H << 8 | L;
          uint8_t Data = memory.read(MemLoc);
          AUXDATA = Data;
          memory.write(MemLoc, Data - 1);
        } else {
          Data = *lookup_table[dest];
          AUXDATA = Data;
          *lookup_table[dest] = Data - 1;
        }
        uint8_t Result = Data - 1;

        Flag_INDC(Result, AUXDATA, true);

        break;
      }

        // 4 Opcode For INX

      case 0x03:
      case 0x13:
      case 0x23:
      case 0x33: {
        uint8_t RP = (opcode & 0x30) >> 4;

        switch (RP) {
          case 0x00: {
            uint16_t Pair = B << 8 | C;
            Pair++;
            C = Pair & 0xFF;
            B = (Pair & 0xFF00) >> 8;

            break;
          }
          case 0x01: {
            uint16_t Pair = D << 8 | E;
            Pair++;
            E = Pair & 0xFF;
            D = (Pair & 0xFF00) >> 8;
            break;
          }
          case 0x02: {
            uint16_t Pair = H << 8 | L;
            Pair++;
            L = Pair & 0xFF;
            H = (Pair & 0xFF00) >> 8;
            break;
          }
          case 0x03: {
            SP++;
            break;
          }
        }
        break;
      }

        // 4 Opcode for DCX

      case 0x08:
      case 0x18:
      case 0x28:
      case 0x38: {
        uint8_t RP = (opcode & 0x30) >> 4;

        switch (RP) {
          case 0x00: {
            uint16_t Pair = B << 8 | C;
            Pair--;
            C = Pair & 0xFF;
            B = (Pair & 0xFF00) >> 8;
            break;
          }
          case 0x01: {
            uint16_t Pair = D << 8 | E;
            Pair--;
            E = Pair & 0xFF;
            D = (Pair & 0xFF00) >> 8;
            break;
          }
          case 0x02: {
            uint16_t Pair = H << 8 | L;
            Pair--;
            L = Pair & 0xFF;
            H = (Pair & 0xFF00) >> 8;
            break;
          }
          case 0x03: {
            SP--;
            break;
          }
        }
        break;
      }

        // 4 Opcodes For DAD

      case 0x09:
      case 0x19:
      case 0x29:
      case 0x39: {
        uint8_t HL = H >> 8 | L;
        uint16_t RP = (opcode & 0x30) >> 4;
        uint32_t Result;

        switch (RP) {
          case 0x00: {
            uint16_t Data = B << 8 | C;
            Result = Data + HL;
            break;
          }
          case 0x01: {
            uint16_t Data = D << 8 | E;
            Result = Data + HL;

            break;
          }

          case 0x02: {
            uint16_t Data = HL;
            Result = Data + HL;
            break;
          }

          case 0x03: {
            uint16_t Data = SP;
            Result = Data + HL;
            break;
          }
        }
        L = Result & 0xFF;
        H = (Result & 0xFF00) >> 8;

        if (Result > 0xFFFF) {
          PSW |= 0x01;
        } else {
          PSW &= ~0x01;
        }
        break;
      }

        // 8 Opcode of ANA

      case 0xa0 ... 0xa7: {
        uint8_t Src = opcode & 0x07;
        uint8_t Result;

        if (Src == 0x06) {
          uint16_t MemLoc = H << 8 | L;
          uint8_t Data = memory.read(MemLoc);
          Result = A & Data;
        } else {
          Result = A & *lookup_table[Src];
        }
        A = Result;
        Flag_Setting_Logical(Result, ANA);
        break;
      }

        // 1 opcode for ANI

      case 0xe6: {
        uint8_t Data = memory.read(PC);
        PC++;

        uint8_t Result = A & Data;
        A = Result;

        Flag_Setting_Logical(Result, ANA);

        break;
      }

        // 8 Opcodes For XRA

      case 0xa8 ... 0xaf: {
        uint8_t Src = opcode & 0x07;
        uint8_t Result;
        if (Src == 0x06) {
          uint16_t MemLoc = H << 8 | L;
          uint8_t Data = memory.read(MemLoc);
          Result = A ^ Data;
        } else {
          Result = A ^ *lookup_table[Src];
        }
        A = Result;
        Flag_Setting_Logical(Result, XRA);
        break;
      }

        // 1 Opcode For XRI

      case 0xee: {
        uint8_t Data = memory.read(PC);
        PC++;

        uint8_t Result = A ^ Data;
        A = Result;

        Flag_Setting_Logical(Result, XRA);

        break;
      }

        // 8 Opcodes for ORA

      case 0xb0 ... 0x0b7: {
        uint8_t Src = opcode & 0x07;
        uint8_t Result;

        if (Src == 0x06) {
          uint16_t MemLoc = H << 8 | L;
          uint8_t Data = memory.read(MemLoc);
          Result = A | Data;
        } else {
          Result = A | *lookup_table[Src];
        }
        A = Result;

        Flag_Setting_Logical(Result, ORA);
        break;
      }

      // 1 Opcode for ORI
      case 0xf6: {
        uint8_t Data = memory.read(PC);
        PC++;

        uint8_t Result = A | Data;
        A = Result;

        Flag_Setting_Logical(Result, ORA);

        break;
      }

      // 8 Opcodes For CMP
      case 0xb8 ... 0xbf: {
        uint8_t src = opcode & 0x07;
        uint8_t Operand;

        if (src == 0x06) {
          uint16_t MemLoc = H << 8 | L;
          Operand = memory.read(MemLoc);
        } else {
          Operand = *lookup_table[src];
        }

        uint16_t Result16 = A - Operand;
        uint8_t Result8 = Result16 & 0xFF;

        Flag_Set_ALU(Result8, Result16, Operand, A, true);
        break;
      }

        // 1 opcode for CPI

      case 0xfe: {
        uint8_t Data = memory.read(PC);
        PC++;

        uint16_t Result16 = A - Data;
        uint8_t Result8 = Result16 & 0xFF;

        Flag_Set_ALU(Result8, Result16, Data, A, true);
        break;
      }

      // 1 Opcode For RLC
      case 0x07: {
        uint8_t BitToLow = (A & 0x80) >> 7;
        A = (A << 1) | BitToLow;

        if (BitToLow) {
          PSW |= 0x01;
        } else {
          PSW &= ~0x01;
        }
        break;
      }

        // 1 Opcode For RRC

      case 0x0f: {
        uint8_t BitToHigh = A & 0x01;
        A = (A >> 1) | (BitToHigh << 7);

        if (BitToHigh) {
          PSW |= 0x01;
        } else {
          PSW &= ~0x01;
        }
        break;
      }

        // 1 opcode for RAL

      case 0x17: {
        uint8_t CarryValue = PSW & 0x01;
        uint8_t BitToCarry = (A & 0x80) >> 7;

        A = (A << 1) | CarryValue;

        if (BitToCarry) {
          PSW |= 0x01;
        } else {
          PSW &= ~0x01;
        }
        break;
      }

        // 1 Opcode for RAR

      case 0x1f: {
        uint8_t CarryValue = PSW & 0x01;
        uint8_t BitToCarry = A & 0x01;

        A = (A >> 1) | (CarryValue << 7);

        if (BitToCarry) {
          PSW |= 0x01;
        } else {
          PSW &= ~0x01;
        }
        break;
      }

      // 1 opcode for CMA
      case 0x2f: {
        A = ~A & 0xff;
        break;
      }

      // 1 opcode for CMC
      case 0x3f: {
        PSW ^= 0x01;
        break;
      }

      // 1 Opcode for STC
      case 0x37: {
        PSW |= 0x01;
        break;
      }

        // 1 Opcode For JMP

      case 0xc3: {
        uint8_t Lowbits = memory.read(PC);
        uint8_t HighBits = memory.read(++PC);

        uint16_t MemLoc = HighBits << 8 | Lowbits;

        PC = MemLoc;

        break;
      }

        // 8 opcodes For JUMP Conditions

      case 0xc2:
      case 0xD2:
      case 0xE2:
      case 0xf2:
      case 0xca:
      case 0xda:
      case 0xea:
      case 0xfa: {
        uint8_t CCC = (opcode & 0x38) >> 3;
        uint8_t LowBits = memory.read(PC);
        uint8_t HighBits = memory.read(++PC);
        uint16_t MemLoc = HighBits << 8 | LowBits;
        switch (CCC) {
          case 0x00: {
            if ((PSW & 0x40) == 0) {
              PC = MemLoc;
            }
            break;
          }

          case 0x01: {
            if ((PSW & 0x40) != 0) {
              PC = MemLoc;
            }
            break;
          }
          case 0x02: {
            if ((PSW & 0x01) == 0) {
              PC = MemLoc;
            }
            break;
          }
          case 0x03: {
            if ((PSW & 0x01) != 0) {
              PC = MemLoc;
            }
            break;
          }

          case 0x04: {
            if ((PSW & 0x04) == 0) {
              PC = MemLoc;
            }

            break;
          }
          case 0x05: {
            if ((PSW & 0x04) != 0) {
              PC = MemLoc;
            }
            break;
          }

          case 0x06: {
            if ((PSW & 0x80) == 0) {
              PC = MemLoc;
              break;
            }
          }
          case 0x07: {
            if ((PSW & 0x80) != 0) {
              PC = MemLoc;
              break;
            }
          }
        }
        break;
      }
      




    }
  }
};