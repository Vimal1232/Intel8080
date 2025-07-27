
# Intel8080(Space Invaders)
A custom-built Intel 8080 emulator capable of accurately running **Space Invaders** arcade game. This project includes hardware-specific I/O emulation and graphics output using SDL

## 🕹️ Game Play
<img src="./demo.gif"/>

## 🚀 Features
- ✅ Full Intel 8080 instruction set emulation
- ✅ Accurate memory map and I/O for Space Invaders
- ✅ Shift register emulation for game logic
- ✅ SDL-based pixel rendering 
- ✅ Keyboard input mapped to original arcade controls

## 🗂️ Memory Map

- 0x0000 - 0x1FFF : ROM (4 x 2KB)
- 0x2000 - 0x23FF : RAM
- 0x2400 - 0x3FFF : Video RAM

## 🎮 Controls

| Arcade Control | Key         |
|----------------|-------------|
| Coin Insert    | `C`         |
| Start (P1)     | `1`         |
| Move Left      | `←`         |
| Move Right     | `→`         |
| Fire           | `Spacebar`  |


## 📚 Acknowledgements

- 🧾 **Intel 8080 documentation** 
- 🖥️ **8080 Emulator Manual**
- 🎮 **Computer Archeology**
- 🎨 **SDL library**

