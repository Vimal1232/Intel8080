#pragma once
#include <_types/_uint16_t.h>

#include <bitset>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ratio>
#include <sstream>

class Memory {
 public:
  uint8_t *mem;
  uint16_t ROM_END = 0x2000;

  Memory() { mem = new uint8_t[64 * 1024](); };

  uint8_t read(uint16_t address) { return mem[address]; };

  void write(uint16_t address, uint8_t value) { mem[address] = value; }

  void Load_ROM() {
    std::ifstream file("../invaders.h", std::ios::binary);
    std::ifstream file2("../invaders.g", std::ios::binary);
    std::ifstream file3("../invaders.f", std::ios::binary);
    std::ifstream file4("../invaders.e", std::ios::binary);

    if (!file) {
      std::cout << "Can't Open File" << std::endl;
      return;
    }
    uint16_t addressH = 0x00;
    uint16_t addressG = 0x800;
    uint16_t addressF = 0x1000;
    uint16_t addressE = 0x1800;
    uint8_t byte;

    while (file.read(reinterpret_cast<char *>(&byte), 1)) {
      mem[addressH++] = byte;
    }
    while (file2.read(reinterpret_cast<char *>(&byte), 1)) {
      mem[addressG++] = byte;
    }
    while (file3.read(reinterpret_cast<char *>(&byte), 1)) {
      mem[addressF++] = byte;
      ;
    }
    while (file4.read(reinterpret_cast<char *>(&byte), 1)) {
      mem[addressE++] = byte;
    }

    std::cout << "ROM Loading Done" << std::endl;
    // std::cout << "The Last Address" << std::hex << "0x" << addressF
    //           << std::endl;
    std::cout << "VRAM from 2400 - 3FFF" << std::endl;
  }
};