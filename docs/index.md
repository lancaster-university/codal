#Introduction

The CODAL runtime provides an easy to use environment for programming a number of devices
in the C/C++ language, written by Lancaster University. It contains device drivers for hardware capabilities supported on a number devices,
and also a suite of runtime mechanisms to make programming an embedded device easier and more flexible. These
range from control of the LED matrix display to peer-to-peer radio communication.

In addition to supporting development in C/C++, the runtime is also designed specifically to support
higher level languages provided by our partners that target physical computing and computer science education. It is currently used as a support library for [Microsoft MakeCode](https://makecode.com)

On these pages you will find guidance on how to get started with CODAL, including setting up and configuring your development environment.

#Getting Started

1. [Install](installation.md) the required tools.
2. Select your target.
3. **PROGRAM!**

Here is some sample code Circuit Playground to get you started:

```cpp
#include "CircuitPlayground.h"

CircuitPlayground cplay;

int main()
{
    while(1)
    {
        cplay.io.led.setDigitalValue(1);
        cplay.sleep(500);
        cplay.io.led.setDigitalValue(1);
        cplay.sleep(500);
    }

    release_fiber();
}
```

### What is cplay?

`cplay` is an instance of the CircuitPlayground class, a software model of the Circuit Playground hardware. This abstraction provides a really simple way to interact with the various components of the device itself.

This simplicity can be seen with this line of code:

```cpp
cplay.io.led.setDigitalValue(1);
```

This line sets the LED indicator on the device.


### What is a fiber and why do we release it?

Fibers are lightweight threads used by the runtime to perform operations asynchronously.

The function call `release_fiber();` is recommended at the end of main to release the main fiber, and enter
the scheduler indefinitely as you may have other fibers running elsewhere in the code.
It also means that the processor will enter a power efficient sleep if there are
no other processes running.

If this line is omitted, your program will cease all execution.
