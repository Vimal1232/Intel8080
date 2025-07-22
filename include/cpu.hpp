#pragma once
#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>

#include <Memory.hpp>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <iostream>

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

  bool Halt = false;

  CPU(Memory& mem) : memory(mem) {
    B = 0x0;
    C = 0x0;
    D = 0x0;
    E = 0x0;
    H = 0x0;
    L = 0x0;
    A = 0x0;

    PSW = 0x02;

    PC = 0x0100;
    SP = 0xFFFF;
  }

  uint8_t fetch() {
    uint8_t opcode = memory.read(PC);
    PC++;
    return opcode;
  }

  void Flag_Set_ALU(uint8_t result8, uint16_t result16, uint16_t operand,
                    uint8_t OriginalA, bool SUB = false) {
    if (SUB) {
      if ((OriginalA & 0x0F) < (operand & 0x0F)) {
        PSW |= 0x10;
      } else {
        PSW &= ~0x10;
      }
    } else {
      if (((OriginalA & 0x0F) + (operand & 0x0F)) > 0x0F) {
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

    if ((result8 & 0x80)) {
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
      if (OriginalA < operand) {
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
          Halt = true;
          std::cout << "CPU Halted at PC: 0x" << std::hex << PC << std::endl;
        } else {
          if (Dest == 0x06) {
            uint16_t MemLoc = (H << 8) | L;
            if (Src == 0x06) {
              Halt = true;
              std::cout << "CPU HALTED" << std::endl;
            } else {
              memory.write(MemLoc, *lookup_table[Src]);
            }
          } else if (Src == 0x06) {
            uint16_t MemLoc = (H << 8) | L;
            *lookup_table[Dest] = memory.read(MemLoc);
          } else {
            *lookup_table[Dest] = *lookup_table[Src];
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

        Flag_Set_ALU(Result8, Result16, (uint16_t)Operand + carry, originalA);

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

        Flag_Set_ALU(Result8, Result16, (uint16_t)Data + carry, originalA);

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

        uint16_t Result16 = A - Operand - Borrow;
        uint8_t Result8 = Result16 & 0xFF;
        A = Result8;

        Flag_Set_ALU(Result8, Result16, (uint16_t)Operand + Borrow, originalA,
                     true);

        break;
      }
        // 1 opcode of SBI

      case 0xde: {
        uint8_t Data = memory.read(PC);
        PC++;
        uint8_t originalA = A;
        uint8_t Borrow = PSW & 0x01;
        uint16_t Result16 = A - Data - Borrow;
        uint8_t Result8 = Result16 & 0xff;
        A = Result8;

        Flag_Set_ALU(Result8, Result16, (uint16_t)Data + Borrow, originalA,
                     true);

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

        Flag_INDC(Result, AUXDATA);

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
            uint16_t BC = (B << 8) | C;
            BC++;
            B = (BC & 0xFF00) >> 8;
            C = BC & 0x00FF;
            break;

            break;
          }
          case 0x01: {
            uint16_t DE = (D << 8) | E;
            DE++;
            D = (DE & 0xFF00) >> 8;
            E = DE & 0x00FF;
            break;

            break;
          }
          case 0x02: {
            uint16_t HL = (H << 8) | L;
            HL++;
            H = (HL & 0xFF00) >> 8;
            L = HL & 0x00FF;
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

      case 0x0b:
      case 0x1b:
      case 0x2b:
      case 0x3b: {
        uint8_t RP = (opcode & 0x30) >> 4;

        switch (RP) {
          case 0x00: {
            uint16_t BC = (B << 8) | C;
            BC--;
            B = (BC & 0xFF00) >> 8;
            C = BC & 0x00FF;
            break;
          }
          case 0x01: {
            uint16_t DE = (D << 8) | E;
            DE--;
            D = (DE & 0xFF00) >> 8;
            E = DE & 0x00FF;
            break;
          }
          case 0x02: {
            uint16_t HL = (H << 8) | L;
            HL--;
            H = (HL & 0xFF00) >> 8;
            L = HL & 0x00FF;
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
        uint16_t RP = (opcode & 0x30) >> 4;

        switch (RP) {
          case 0x00: {
            uint16_t Data = (B << 8) | C;
            uint16_t HL = (H << 8) | L;
            uint32_t Result = HL + Data;

            if (Result > 0xFFFF) {
              PSW |= 0x01;
            } else {
              PSW &= ~0x01;
            }
            uint16_t Result16 = Result & 0xffff;
            H = (Result16 & 0xFF00) >> 8;
            L = Result16 & 0x00FF;

            break;
          }
          case 0x01: {
            uint16_t Data = (D << 8) | E;
            uint16_t HL = (H << 8) | L;
            uint32_t Result = HL + Data;

            if (Result > 0xFFFF) {
              PSW |= 0x01;
            } else {
              PSW &= ~0x01;
            }
            uint16_t Result16 = Result & 0xffff;
            H = (Result16 & 0xFF00) >> 8;
            L = Result16 & 0x00FF;

            break;
          }

          case 0x02: {
            uint16_t HL = (H << 8) | L;
            uint32_t Result = HL + HL;
            if (Result > 0xFFFF) {
              PSW |= 0x01;
            } else {
              PSW &= ~0x01;
            }
            uint16_t Result16 = Result & 0xffff;
            H = (Result16 & 0xFF00) >> 8;
            L = Result16 & 0x00FF;

            break;
          }

          case 0x03: {
            uint16_t Data = SP;
            uint16_t HL = (H << 8) | L;
            uint32_t Result = HL + Data;

            if (Result > 0xFFFF) {
              PSW |= 0x01;
            } else {
              PSW &= ~0x01;
            }
            uint16_t Result16 = Result & 0xffff;
            H = (Result16 & 0xFF00) >> 8;
            L = Result16 & 0x00FF;

            break;
          }
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
          uint16_t MemLoc = (H << 8) | L;
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

      case 0xb0 ... 0xb7: {
        uint8_t Src = opcode & 0x07;
        uint8_t Result;

        if (Src == 0x06) {
          uint16_t MemLoc = (H << 8) | L;
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
          uint16_t MemLoc = (H << 8) | L;
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
        // 8 opcodes For JUMP Conditions

      case 0xc3:
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
        uint8_t HighBits = memory.read(PC + 1);
        uint16_t MemLoc = (HighBits << 8) | LowBits;

        if (opcode == 0xc3) {
          PC = MemLoc;
        } else {
          switch (CCC) {
            case 0x00: {
              if ((PSW & 0x40) == 0) {
                PC = MemLoc;
              } else {
                PC += 2;
              }
              break;
            }

            case 0x01: {
              if ((PSW & 0x40) != 0) {
                PC = MemLoc;
              } else {
                PC += 2;
              }
              break;
            }
            case 0x02: {
              if ((PSW & 0x01) == 0) {
                PC = MemLoc;
              } else {
                PC += 2;
              }
              break;
            }
            case 0x03: {
              if ((PSW & 0x01) != 0) {
                PC = MemLoc;
              } else {
                PC += 2;
              }
              break;
            }

            case 0x04: {
              if ((PSW & 0x04) == 0) {
                PC = MemLoc;
              } else {
                PC += 2;
              }

              break;
            }
            case 0x05: {
              if ((PSW & 0x04) != 0) {
                PC = MemLoc;
              } else {
                PC += 2;
              }
              break;
            }

            case 0x06: {
              if ((PSW & 0x80) == 0) {
                PC = MemLoc;
              } else {
                PC += 2;
              }
              break;
            }
            case 0x07: {
              if ((PSW & 0x80) != 0) {
                PC = MemLoc;
              } else {
                PC += 2;
              }
              break;
            }
          }
        }
        break;
      }

        // 1 Opcode for CALL
        // 8 Opcodes for Condition CALLs

      case 0xcd:
      case 0xcc:
      case 0xdc:
      case 0xec:
      case 0xfc:
      case 0xc4:
      case 0xd4:
      case 0xe4:
      case 0xf4: {
        uint8_t CCC = (opcode & 0x38) >> 3;

        uint8_t LowerBit = memory.read(PC);
        uint8_t HigherBit = memory.read(PC + 1);
        uint16_t MemLoc = (HigherBit << 8) | LowerBit;

        uint16_t ProgramUpgrade = PC + 2;

        uint8_t LowOrderBit = ProgramUpgrade & 0x00FF;
        uint8_t HighOrderBit = (ProgramUpgrade & 0xFF00) >> 8;
        if (opcode == 0xcd) {
          PC = MemLoc;
          SP--;
          memory.write(SP, HighOrderBit);
          SP--;
          memory.write(SP, LowOrderBit);

        } else {
          switch (CCC) {
            case 0x00: {
              if ((PSW & 0x40) == 0) {
                PC = MemLoc;
                SP--;
                memory.write(SP, HighOrderBit);
                SP--;
                memory.write(SP, LowOrderBit);
              } else {
                PC += 2;
              }
              break;
            }

            case 0x01: {
              if ((PSW & 0x40) != 0) {
                PC = MemLoc;
                SP--;
                memory.write(SP, HighOrderBit);
                SP--;
                memory.write(SP, LowOrderBit);
              } else {
                PC += 2;
              }
              break;
            }
            case 0x02: {
              if ((PSW & 0x01) == 0) {
                PC = MemLoc;
                SP--;
                memory.write(SP, HighOrderBit);
                SP--;
                memory.write(SP, LowOrderBit);
              } else {
                PC += 2;
              }
              break;
            }
            case 0x03: {
              if ((PSW & 0x01) != 0) {
                PC = MemLoc;
                SP--;
                memory.write(SP, HighOrderBit);
                SP--;
                memory.write(SP, LowOrderBit);
              } else {
                PC += 2;
              }
              break;
            }

            case 0x04: {
              if ((PSW & 0x04) == 0) {
                PC = MemLoc;
                SP--;
                memory.write(SP, HighOrderBit);
                SP--;
                memory.write(SP, LowOrderBit);
              } else {
                PC += 2;
              }

              break;
            }
            case 0x05: {
              if ((PSW & 0x04) != 0) {
                PC = MemLoc;
                SP--;
                memory.write(SP, HighOrderBit);
                SP--;
                memory.write(SP, LowOrderBit);
              } else {
                PC += 2;
              }
              break;
            }

            case 0x06: {
              if ((PSW & 0x80) == 0) {
                PC = MemLoc;
                SP--;
                memory.write(SP, HighOrderBit);
                SP--;
                memory.write(SP, LowOrderBit);
              } else {
                PC += 2;
              }
              break;
            }
            case 0x07: {
              if ((PSW & 0x80) != 0) {
                PC = MemLoc;
                SP--;
                memory.write(SP, HighOrderBit);
                SP--;
                memory.write(SP, LowOrderBit);
              } else {
                PC += 2;
              }
              break;
            }
          }
        }

        break;
      }

        // 1 Opcode for RET
        // 8 Opcodes For RET Conditions

      case 0xc9:
      case 0xc8:
      case 0xd8:
      case 0xe8:
      case 0xf8:
      case 0xc0:
      case 0xd0:
      case 0xe0:
      case 0xf0: {
        uint8_t CCC = (opcode & 0x38) >> 3;

        uint8_t LowOrderBit = memory.read(SP);
        uint8_t HighOrderBit = memory.read(SP + 1);
        uint16_t MemLoc = (HighOrderBit << 8) | LowOrderBit;

        if (opcode == 0xc9) {
          PC = MemLoc;
          SP += 2;

        } else {
          switch (CCC) {
            case 0x00: {
              if ((PSW & 0x40) == 0) {
                PC = MemLoc;
                SP += 2;
              }
              break;
            }

            case 0x01: {
              if ((PSW & 0x40) != 0) {
                PC = MemLoc;
                SP += 2;
              }
              break;
            }
            case 0x02: {
              if ((PSW & 0x01) == 0) {
                PC = MemLoc;
                SP += 2;
              }
              break;
            }
            case 0x03: {
              if ((PSW & 0x01) != 0) {
                PC = MemLoc;
                SP += 2;
              }
              break;
            }

            case 0x04: {
              if ((PSW & 0x04) == 0) {
                PC = MemLoc;
                SP += 2;
              }

              break;
            }
            case 0x05: {
              if ((PSW & 0x04) != 0) {
                PC = MemLoc;
                SP += 2;
              }
              break;
            }

            case 0x06: {
              if ((PSW & 0x80) == 0) {
                PC = MemLoc;
                SP += 2;
              }
              break;
            }
            case 0x07: {
              if ((PSW & 0x80) != 0) {
                PC = MemLoc;
                SP += 2;
              }
              break;
            }
          }
        }

        break;
      }

        // 8 opcodes For RST

      case 0xc7:
      case 0xd7:
      case 0xe7:
      case 0xf7:
      case 0xcf:
      case 0xdf:
      case 0xef:
      case 0xff: {
        uint16_t ProgramUpdgrade = PC;
        uint8_t Lowbits = ProgramUpdgrade & 0xFF;
        uint8_t HighBits = (ProgramUpdgrade & 0xFF00) >> 8;

        uint8_t NNN = (opcode & 0x38) >> 3;

        memory.write(SP - 1, HighBits);
        memory.write(SP - 2, Lowbits);

        SP -= 2;

        PC = 8 * NNN;

        break;
      }

      case 0xe9: {
        PC = H << 8 | L;

        break;
      }

        // 3  opcodes FOR PUSH register Pairs

      case 0xc5:
      case 0xd5:
      case 0xe5: {
        uint8_t RP = (opcode & 0x30) >> 4;

        switch (RP) {
          case 0x00: {
            SP--;
            memory.write(SP, B);
            SP--;
            memory.write(SP, C);

            break;
          }
          case 0x01: {
            SP--;
            memory.write(SP, D);
            SP--;
            memory.write(SP, E);

            break;
          }

          case 0x02: {
            SP--;
            memory.write(SP, H);
            SP--;
            memory.write(SP, L);

            break;
          }
        }
        break;
      }

        // PUSH For PSW

      case 0xf5: {
        SP--;
        memory.write(SP, A);
        SP--;
        memory.write(SP, PSW);

        break;
      }

        // POP for RPs

      case 0xc1:
      case 0xd1:
      case 0xe1: {
        uint8_t RP = (opcode & 0x30) >> 4;

        switch (RP) {
          case 0x00: {
            C = memory.read(SP);
            SP++;
            B = memory.read(SP);
            SP++;
            break;
          }
          case 0x1: {
            E = memory.read(SP);
            SP++;
            D = memory.read(SP);
            SP++;
            break;
          }
          case 0x2: {
            L = memory.read(SP);
            SP++;
            H = memory.read(SP);
            SP++;

            break;
          }
        }

        break;
      }

        // POP for PSW

      case 0xf1: {
        PSW = memory.read(SP);
        SP++;
        A = memory.read(SP);
        SP++;
        break;
      }

        // XTHL

      case 0xe3: {
        uint8_t temp = memory.read(SP);
        uint8_t tempH = memory.read(SP + 1);

        memory.write(SP, L);
        memory.write(SP + 1, H);

        L = temp;
        H = tempH;

        break;
      }

        // SPHL

      case 0xf9: {
        SP = (H << 8) | L;
        break;
      }

      case 0x00: {
        break;
      }

      case 0xdb: {
        uint8_t port = memory.read(PC);
        PC++;

        if (port == 0x00 || port == 0x01) {
          A = 0xFF;
        } else {
          A = 0x00;
        }
        break;
      }

      case 0xd3: {
        uint8_t port = memory.read(PC);
        uint8_t data = A;
        PC++;
        if (port == 0x00 || port == 0x01) {
          if (data >= 32 && data <= 126) {
            std::cout << static_cast<char>(data);
          } else if (data == 13) {
            std::cout << '\n';
          } else if (data == 10) {
            std::cout << '\n';
          }
          std::cout.flush();
        }
        break;
      }
        // Case DAA;

      case 0x27: {
        uint8_t correction = 0;
        bool carry = false;
        if ((A & 0x0F) > 9 || (PSW & 0x10)) {
          correction += 0x06;
        }
        if (((A & 0xF0) >> 4) > 9 || (PSW & 0x01) ||
            (((A & 0xF0) >> 4) >= 9 && (A & 0x0F) > 9)) {
          correction += 0x60;
          carry = true;
        }
        uint8_t originalA = A;
        A += correction;

        if (((originalA & 0x0F) + (correction & 0x0F)) > 0x0F) {
          PSW |= 0x10;
        } else {
          PSW &= ~0x10;
        }

        if (carry) {
          PSW |= 0x01;
        } else {
          PSW &= ~0x01;
        }
        if (A == 0) {
          PSW |= 0x40;
        } else {
          PSW &= ~0x40;
        }
        if (A & 0x80) {
          PSW |= 0x80;
        } else {
          PSW &= ~0x80;
        }
        int count = 0;
        uint8_t temp = A;
        for (int i = 0; i < 8; i++) {
          if (temp & 1) count++;
          temp >>= 1;
        }
        if (count % 2 == 0) {
          PSW |= 0x04;
        } else {
          PSW &= ~0x04;
        }

        break;
      }

      default: {
        std::cout << "Unknown Opcode: 0x" << std::hex << std::setfill('0')
                  << std::setw(2) << static_cast<int>(opcode) << " at PC: 0x"
                  << std::setw(4) << (PC - 1) << std::dec << "\n";
        break;
      }
    }
  }

  void handle_BDOS_call() {
    uint8_t low = memory.read(SP);
    uint8_t high = memory.read(SP + 1);
    uint16_t MemLoc = (high << 8) | low;
    if (MemLoc == 0x00) {
      Halt = true;
    }
    uint8_t function = C;
    switch (function) {
      case 2: {
        char ch = static_cast<char>(E);
        if (ch >= 32 && ch <= 126) {
          std::cout << ch;
        } else if (ch == 13 || ch == 10) {
          std::cout << '\n';
        }
        std::cout.flush();
        break;
      }
      case 9: {
        uint16_t addr = (D << 8) | E;
        char ch;
        while ((ch = memory.read(addr)) != '$') {
          if (ch >= 32 && ch <= 126) {
            std::cout << ch;
          } else if (ch == 13 || ch == 10) {
            std::cout << '\n';
          }
          addr++;
        }
        std::cout.flush();
        break;
      }
      default: {
        break;
      }
    }
    PC = MemLoc;
    SP += 2;
  }

  void cycle() {
    if (PC == 0x0005) {
      handle_BDOS_call();
      return;
    }
    uint8_t opcode = fetch();

    decode(opcode);
  }

  void run() {
    while (!Halt) {
      cycle();
    }
  }
};
