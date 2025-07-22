#pragma once
#include <bitset>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

class Memory {
 public:
  uint8_t *mem;
  uint16_t ROM_END = 0x1FFF;
  uint16_t STACK_START = 0xFF00;

  Memory() { mem = new uint8_t[64 * 1024](); };

  uint8_t read(uint16_t address) { return mem[address]; };

  void write(uint16_t address, uint8_t value) { mem[address] = value; }

  void Load_ROM() {
    std::ifstream file("../TST8080.COM", std::ios::binary);

    if (!file) {
      std::cout << "Can't Open File" << std::endl;
      return;
    }

    file.seekg(0, std::ios::end);
    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    uint16_t address = 0x0100;
    uint8_t byte;

    if (size >= (ROM_END - 0x0100)) {
      std::cout << "File Size too Big Get a Smaller ROM" << std::endl;
      return;
    } else {
      while (file.read(reinterpret_cast<char *>(&byte), 1)) {
        mem[address] = byte;
        address++;
      }

      std::cout << "ROM Loading Done" << std::endl;
    }
  }
};