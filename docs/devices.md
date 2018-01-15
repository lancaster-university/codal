# Devices

A number of devices (or targets) are currently supported in CODAL, to obtain a full list of targets type:

```
python build.py ls
```

To generate the `codal.json` for a target listed by the ls command, please run:

```
python build.py <target-name>
```
Please note you may need to remove the libraries folder if your previous build relied on similar dependencies.


# Configuration

Below is an example of how to configure codal to build the [codal-circuit-playground](https://github.com/lancaster-university/codal-circuit-playground) target, example applications will automatically be loaded into the "source" folder:

```json
{
    "target":{
        "name":"codal-circuit-playground",
        "url":"https://github.com/lancaster-university/codal-circuit-playground",
        "branch":"master",
        "type":"git"
    }
}
```

For more targets, read the targets section below.

## Configuration Options

If you would like to override or define any additional configuration options (`#define's`) that are used by the supporting libraries, the codal build system allows the addition of a config field in `codal.json`:

```json
{
    "target":{
        "name":"codal-circuit-playground",
        "url":"https://github.com/lancaster-university/codal-circuit-playground",
        "branch":"master",
        "type":"git"
    },
    "config":{
        "NUMBER_ONE":1
    },
    "application":"source",
    "output_folder":"."
}
```

The above example will translate `"NUMBER_ONE":1` into: `#define NUMBER_ONE     1` and force include it during compilation. You can also specify alternate application or output folders.


# Targets

## Arduino Uno

This target specifies the arduino uno which is driven by an atmega328p.

### codal.json specification
```json
{
    "target":{
        "name":"codal-arduino-uno",
        "url":"https://github.com/lancaster-university/codal-arduino-uno",
        "branch":"master",
        "type":"git"
    }
}

```
This target depends on:

* [codal-core](https://github.com/lancaster-university/codal-core) provides the core CODAL abstractions
* [codal-atmega328p](https://github.com/lancaster-university/codal-atmega328p) implements basic CODAL components (I2C, Pin, Serial, Timer)

## BrainPad

This target specifies the BrainPad which is driven by a STM32F.

### codal.json specification
```json
{
    "target":{
        "name":"codal-brainpad",
        "url":"https://github.com/lancaster-university/codal-brainpad",
        "branch":"master",
        "type":"git"
    }
}
```
This target depends on:

* [codal-core](https://github.com/lancaster-university/codal-core) provides the core CODAL abstractions
* [codal-mbedos](https://github.com/lancaster-university/codal-mbed) implements required CODAL basic components (Timer, Serial, Pin, I2C, ...) using Mbed

## Circuit Playground

This target specifies the circuit playground which is driven by a SAMD21.

### codal.json specification
```json
{
    "target":{
        "name":"codal-circuit-playground",
        "url":"https://github.com/lancaster-university/codal-circuit-playground",
        "branch":"master",
        "type":"git"
    }
}

```
This target depends on:

* [codal-core](https://github.com/lancaster-university/codal-core) provides the core CODAL abstractions
* [codal-mbed](https://github.com/lancaster-university/codal-mbed) implements required CODAL basic components (Timer, Serial, Pin, I2C, ...) using Mbed
* [codal-samd21](https://github.com/lancaster-university/codal-samd21) implements SAMD21-specific components (such as USB)
* [mbed-classic](https://github.com/lancaster-university/mbed-classic) is a fork of mbed, used by codal-mbed
