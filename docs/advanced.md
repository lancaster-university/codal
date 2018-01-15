# Advanced

## Architecture

The CODAL runtime is formed of many libraries, the library common to *all* CODAL targets is known as [codal-core](https://github.com/lancaster-university/codal-core). This library contains common drivers, abstractions for common drivers (driver models), the scheduler and eventing mechanisms, central to the CODAL experience.

## Libraries

Libraries are simply collections of C/C++ files to be used in the final binary. CMakeLists.txt define the contents and dependencies of a library. Here's an example for codal-core:

```cmake
include("${CODAL_UTILS_LOCATION}")

include("${CODAL_UTILS_LOCATION}")
RECURSIVE_FIND_DIR(INCLUDE_DIRS "./inc" "*.h")
RECURSIVE_FIND_FILE(SOURCE_FILES "./source" "*.c??")

execute_process(WORKING_DIRECTORY "." COMMAND "git" "log" "--pretty=format:%h" "-n" "1" OUTPUT_VARIABLE git_hash)
execute_process(WORKING_DIRECTORY "." COMMAND "git" "rev-parse" "--abbrev-ref" "HEAD" OUTPUT_VARIABLE git_branch OUTPUT_STRIP_TRAILING_WHITESPACE)

if ("${git_branch}" STREQUAL "master")
    set(CODAL_VERSION_STRING "${CODAL_VERSION_STRING}")
else()
    set(CODAL_VERSION_STRING "${CODAL_VERSION_STRING}-${git_branch}-g${git_hash}")
endif()

set(CODAL_VERSION_FLAGS "-DCODAL_VERSION=\\\"${CODAL_VERSION_STRING}\\\"")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CODAL_VERSION_FLAGS}")

add_library(codal-core
    ${SOURCE_FILES}
)

target_include_directories(codal-core PUBLIC ${INCLUDE_DIRS})
```

There are additional defines and function to help developers write CMakeLists.txt.

`${CODAL_UTILS_LOCATION}` defines the location where utility functions are kept, such as `RECURSIVE_FIND_DIR` and `RECURSIVE_FIND_FILE` useful for finding source and header files.

Before invoking any library `CMakeLists.txt`, codal's CMakeLists.txt defines information about the target and toolchain:
- `CODAL_TARGET_NAME` - The name of the current target.
- `CODAL_OUTPUT_NAME` - The name of the output binary.
- `CODAL_TARGET_PROCESSOR` - The name of the target processor.
- `CODAL_TARGET_CPU_ARCHITECTURE` - The architecture of the processor i.e. CortexM0.
- `TOOLCHAIN` - The toolchain e.g. AVR_GCC, ARM_GCC.

Libraries can be composed however you desire, however we recommend segmenting includes and source files into respective directories named "inc" and "source".

## Targets

Targets are libraries with one addition, a `target.json`.

The CODAL build system uses information in this file to select and configure toolchains, and download dependencies. `target.json` also contains information required to configure libraries, like mbed, as well as provide device specific definitions for codal-core. The `target.json` can also configure a post process task, executed from the root of CODAL.

Below is the `target.json` for the Circuit Playground:

```json
{
    "device":"CIRCUIT_PLAYGROUND",
    "processor":"SAMD21G18A",
    "architecture":"CORTEX_M0_PLUS",
    "toolchain":"ARM_GCC",
    "post_process":"python ./utils/uf2conv.py -o <OUTPUT_HEX_DESTINATION>/<OUTPUT_HEX_NAME>.uf2 -c <OUTPUT_BIN_LOCATION>",
    "generate_bin":true,
    "generate_hex":true,
    "config":{
        "CODAL_TIMESTAMP":"uint64_t",
        "USB_MAX_PKT_SIZE": 64,
        "DEVICE_USB_ENDPOINTS":8,
        "USB_DEFAULT_PID":"0x2402",
        "USB_DEFAULT_VID":"0x03EB",
        "USB_EP_FLAG_NO_AUTO_ZLP":"0x01",
        "DEVICE_SRAM_BASE":"0x20000000",
        "DEVICE_SRAM_END":"0x20008000",
        "DEVICE_STACK_BASE":"DEVICE_SRAM_END",
        "DEVICE_STACK_SIZE":2048,
        "TARGET_DEBUG_CLASS":"NOT_IMPLEMENTED",
        "DEVICE_HEAP_ALLOCATOR":1,
        "DEVICE_TAG":0,
        "SCHEDULER_TICK_PERIOD_US":6000,
        "EVENT_LISTENER_DEFAULT_FLAGS":"MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY",
        "MESSAGE_BUS_LISTENER_MAX_QUEUE_DEPTH":10,
        "USE_ACCEL_LSB":0,
        "DEVICE_DEFAULT_SERIAL_MODE":"SYNC_SLEEP",
        "DEVICE_COMPONENT_COUNT":30,
        "DEVICE_DEFAULT_PULLMODE":"PullMode::None",
        "DEVICE_PANIC_HEAP_FULL":1,
        "DEVICE_DMESG":1,
        "DEVICE_DMESG_BUFFER_SIZE":1024,
        "CODAL_DEBUG":0,
        "DEVICE_USB":1,
        "PROCESSOR_WORD_TYPE":"uint32_t"
    },
    "definitions":"-DCONF_CLOCKS_H_INCLUDED -DGCLK_PERIPHERAL_CLOCK=GCLK_GENERATOR_3 -DDEVICE_ANALOGIN -DDEVICE_ANALOGOUT -DDEVICE_I2C -DDEVICE_I2CSLAVE -DDEVICE_I2C_ASYNCH -DDEVICE_INTERRUPTIN -DDEVICE_PORTIN -DDEVICE_PORTINOUT -DDEVICE_PORTOUT -DDEVICE_RTC -DDEVICE_SERIAL -DDEVICE_SERIAL_ASYNCH -DDEVICE_SERIAL_FC -DDEVICE_SLEEP -DDEVICE_SPI -DDEVICE_PWMOUT -DDEVICE_SPISLAVE -DDEVICE_SPI_ASYNCH -DCONF_CLOCKS_H_INCLUDED -DGCLK_PERIPHERAL_CLOCK=GCLK_GENERATOR_3 -D__SAMD21G18A__ -D__CORTEX_M0PLUS -D__MBED__=1  -DTOOLCHAIN_GCC -DTOOLCHAIN_GCC_ARM -DMBED_OPERATORS -DTARGET_ADAFRUIT_CP_G18A -DI2C_MASTER_CALLBACK_MODE=true -DEXTINT_CALLBACK_MODE=true -DUSART_CALLBACK_MODE=true -DTC_ASYNC=true",
    "cmake_definitions":{
        "MBED_LEGACY_TARGET_DEFINITIONS":"ADAFRUIT_CP_G18A;Atmel;SAM_CortexM0P;SAMD21",
        "MBED_LEGACY_TOOLCHAIN":"GCC_ARM;"
    },
    "cpu_opts":"-mcpu=cortex-m0plus -mthumb",
    "asm_flags":"-fno-exceptions -fno-unwind-tables --specs=nosys.specs",
    "c_flags":"-std=c99 --specs=nosys.specs",
    "cpp_flags":"-std=c++11 -fwrapv -fno-rtti -fno-threadsafe-statics -fno-exceptions -fno-unwind-tables -Wl,--gc-sections -Wl,--sort-common -Wl,--sort-section=alignment",
    "linker_flags":"-Wl,--no-wchar-size-warning,-wrap,main",
    "libraries":[
        {
            "name":"codal-core",
            "url":"https://github.com/lancaster-university/codal-core",
            "branch":"master",
            "type":"git"
        },
        {
            "name":"mbed-classic",
            "url":"https://github.com/lancaster-university/mbed-classic",
            "branch":"new-build-system",
            "type":"git"
        },
        {
            "name":"codal-samd21",
            "url":"https://github.com/lancaster-university/codal-samd21",
            "branch":"master",
            "type":"git"
        },
        {
            "name":"codal-mbed",
            "url":"https://github.com/lancaster-university/codal-mbed",
            "branch":"master",
            "type":"git"
        }
    ]
}
```
