transpond
=========

[![Build Status](https://travis-ci.org/veonik/transpond.svg?branch=master)](https://travis-ci.org/veonik/transpond)

An Arduino application for remotely tracking a model rocket.

This application contains two parts: the transponder module and the handset module.

The transponder connects to a few different sensors and is installed in the payload section of the model rocket.

The handset uses a 2.8" Touchscreen LCD shield to interact with and display sensor readings from the transponder.


Installation
------------

Clone the repository.

```bash
git clone --recursive https://github.com/veonik/transpond
```


Building
--------

transpond uses platformio for managing builds.


#### Build the transponder

The transponder application runs on Arduino Uno, Nano, or Teensy 3.5.

To build the transponder for Arduino, use the `nano` environment:

```bash
pio run -e nano
```

To build the transponder for Teensy 3.5, use the `teensy` environment:

```bash
pio run -e teensy
```


#### Build the handset

The handset is designed to run on Arduino Mega2560. To build the handset application:

```bash
pio run -e mega
```