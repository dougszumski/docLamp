docLamp by Doug Szumski
=======================

An RGB LED lamp project using feedback from an ADXL345 accelerometer to control the colour.

The cartesian vector from the ADXL345 is smoothed using a stack-based averaging approach and then converted to spherical polars using look up tables. This yields a stable measurement of both theta and phi, with an angular resolution of better than 0.25 degrees and a refresh rate fast enough to make the lamp highly resoponsive to the users motions. 

TODO:

* Clean up code!! 
* Fix double tap detection for mode selection (currently unreliable)
* Add schematic

Files:

docLamp.c   : Main program

ADXL345.h   : Accelerometer routines (including double tap):
ADXL345.c

hConfig.h   : Hardware specific routines for ATmega
hConfig.c

i2cmaster.h : I2C master library by Peter Fleury
twimaster.c

uart.h      : UART library by Peter Fleury
uart.c

lut.c       : Lookup tables for trig functions
lutGen.py   : Python script for generating the above

sample.h    : Filtering routine for acceleration vectors
sample.c 
teststack.c : Test for above

Significant contributions:

Will J. Szumski for writing the sampling routine
Peter Fleury for providing excellent TWI / UART libraries
