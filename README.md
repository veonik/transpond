transpond
=========

[![Build Status](https://travis-ci.org/veonik/transpond.svg?branch=master)](https://travis-ci.org/veonik/transpond)

An Arduino application for remotely tracking a model rocket.

This application contains two parts: the transponder module and the handset module.
 


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

```bash
PLATFORMIO_BUILD_FLAGS="-DTRANSMITTER" pio run -e nano
```


#### Build the handset

```bash
PLATFORMIO_BUILD_FLAGS="-DRECEIVER" pio run -e mega
```