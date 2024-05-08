# noise-nugget-c-sdk
C/C++ Software Development Kit for the Noise Nugget and PGB-1:

 * **PGB-1** is a pocket-size groovebox with open-source firmware and designed
              for customization. PGB-1 is available on
              [CrowdSupply](https://www.crowdsupply.com/wee-noise-makers/wee-noise-makers-pgb-1).

 * **Noise Nugget** is the audio synthsis and processing board at the heart of
     PGB-1.

## Installing a new firmware on the PGB-1

Warning: This operation will replace the stock firmware and may erase projects
nd samples saved on the PGB-1.

PGB-1 firmware are provided in a `.uf2` file. UF2 files can be dropped into
virtual disk drive provided by the device over USB. Here's the procedure to
install a UF2 firmware:

1. Turn-of PGB-1
2. With a USB cable, plug PGB-1 to your computer
3. With a paper clip, hold down the BOOT button at the bottom right of the
   front pannel
4. While the BOOT button is down, turn on PGB-1. A new USB drive called
   `RPI-RP2` should show up on your computer.
5. Drag and drop the UF2 file in the `RPI-RP2` drive.
6. That's it! PGB-1 should reboot with the new firmware running.

## Building and trying the example project

1. Install CMake, and GCC cross compiler
   ```
   sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
   ```

2. Build using CMake
   ```
   mkdir build
   cd build
   cmake ../ -DPICO_SDK_FETCH_FROM_GIT=on # This step will take several minutes...
   make -j
   ```

3. Program the example on PGB-1: Take the
   `examples/braids_pocket/braids_pocket.uf2` firmware file, and follow the
   "Installing a new firmware on the PGB-1" procedure above.


## Quick-start your own project

### Install CMake, and GCC cross compiler

```
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

### Set up dependencies

1. `git clone` the Raspberry Pi Pico SDK repository:
   ```
   git clone --recursive https://github.com/raspberrypi/pico-sdk
   ```

2. `git clone` the Noise Nugget SDK repository:
   ```
   git clone --recursive https://github.com/wee-noise-makers/noise-nugget-c-sdk
   ```

### Set up project directory and files:

1. Create and enter your project directory
   ```
   mkdir my_project
   cd my_project
   ```

2. Copy `pico_sdk_import.cmake` from the Pico SDK into your project
   directory.
   ```
   cp ../pico-sdk/external/pico_sdk_import.cmake .
   ```

3. Create a `CMakeLists.txt` file:
   ```cmake
   cmake_minimum_required(VERSION 3.12)

   # Pull in PICO SDK (must be before project)
   include(pico_sdk_import.cmake)

   # Init PICO SDK (must be before find_package(NOISE_NUGGET REQUIRED))
   pico_sdk_init()

   project(my_project C CXX ASM)
   set(CMAKE_C_STANDARD 11)
   set(CMAKE_CXX_STANDARD 17)

   # Find the Noise Nugget SDK
   find_package(NOISE_NUGGET REQUIRED)

   # Set up your project and sources
   noise_nugget_executable(
     my_project
     main.c
   )
   ```

4. Create a `main.c` source file:
   ```C
   #include "pgb1.h"
   void main(void) {
       screen_init();
       screen_print(0, 0, "Hello PGB-1!");
       screen_update();
       while (1)
       {
           continue;
       }
   }
   ```

### Build

1. Create and enter the build directory
   ```
   mkdir build
   cd build
   ```

2. Configure and build the project
   ```
   cmake .. -DPICO_SDK_PATH=${PWD}/../../pico-sdk/ -DNOISE_NUGGET_DIR=${PWD}/../../noise-nugget-c-sdk/
   make -j
   ```

### Program and run the project on PGB-1

Take the `my_project.uf2` firmware file, and follow the "Installing a new
firmware on the PGB-1" procedure above.
