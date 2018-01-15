# Installation

## Automatic installation.

This software has its grounding on the founding principles of [Yotta](https://www.mbed.com/en/platform/software/mbed-yotta/), the simplest install path would be to install their tools via their handy installer.

## Docker

A [docker image](https://hub.docker.com/r/jamesadevine/codal-toolchains/) is available that contains toolchains used to build codal targets. A wrapper [Dockerfile](https://github.com/lancaster-university/codal-docker) is available that can be used to build your project with ease.

Then follow the build steps listed below.

## Manual installation

1. Install `git`, ensure it is available on your platforms path.
2. Install the relevant compilation toolchain for your desired platform:
    - `arm-none-eabi-*` command line utilities for ARM based devices
    - `avr-gcc`, `avr-binutils`, `avr-libc` for AVR based devices
    - `xtensa-esp32-*` for xtensa based devices.
3. Install [CMake](https://cmake.org)(Cross platform make), this is the entirety of the build system.
    5. If on Windows, install ninja.
4. Install `Python 2.7` (if you are unfamiliar with CMake), python scripts are used to simplify the build process.
5. Clone this repository

## Building
- Generate or create a `codal.json` file
    - `python build.py ls` lists all available targets
    - `python build.py <target-name>` generates a codal.json file for a given target
- In the root of this repository type `python build.py` the `-c` option cleans before building.
    - If you are not using python:
        - Windows:
            1. In the root of the repository make a build folder.
            2. `cd build`
            3. `cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=RelWithDebInfo`
            4. `ninja`
        - Mac:
            1. In the root of the repository make a build folder.
            2. `cd build`
            3. `cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo`
            4. `make`

- The hex file will be placed at the location specified by `codal.json`, by default this is the root.