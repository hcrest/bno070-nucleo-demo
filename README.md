# SensorHub Sample Application for Nucleo STM32F401RE / STM32F411RE

This repository contains the Nucleo version of the SensorHub sample
application.  It runs on the ST STM32F401RE or STM32F411RE Nucleo eval
boards.

This code runs on an ST STM32F401RE or STM32F411RE Nucleo eval
board connected to the Hillcrest BNO070 Development Board. 

The code in this project uses the SensorHub driver from the companion
project, sh1-mcu-driver.  The instructions below describe how to check
out both repositories and build the code.  If you received the code as
a zip file, the file contains source code for both of the necessary
projects.

## Requirements

* IAR Embedded Workbench for ARM (EWARM) version 7.4
* STM32F411 or STM32F401 Nucleo board
* Hillcrest BNO070 Development board

## Setup

* The code for this project should have been supplied as a zip file
  from Hillcrest.
* After unzipping the file, there will be a folder, sh1, containing two
  subfolders, sh1-mcu-driver and sh1-example-nucleo.
* sh1/sh1-example-nucleo/EWARM contains the IAR Embedded Workbench
  project workspace.

## Building the Code
* Use IAR EWARM to open the workspace, sh1/sh1-example-nucleo/EWARM/Project.eww
* Run Project -> Rebuild All to compile the project.

## Running the Application

* Mount the shield board on the Nucleo platform.
* Connect the Nucleo board to the development PC via USB.
* In IAR EWARM, execute Project -> Download and Debug.
* Once the debugger is ready, click the Go button.

The application should print the SH-1 version numbers, then start
reading and printing Rotation Vectors from the SensorHub.  (Output
appears on the virtual com port of the Nucleo board.  Use any terminal
program to view it using baud rate 115200, 8 bits, No Parity.)

```
Part 10003251 : Version 1.8.4 Build 415
Part 10003210 : Version 0.11.0 Build 176
Part 10003254 : Version 3.5.1 Build 162
Part 10003171 : Version 3.5.3 Build 251
Rotation Vector: t:2.244 r:0.153 i:-0.126 j:-0.020 k:-0.980 (acc: 179.987 [deg])
Rotation Vector: t:2.254 r:0.153 i:-0.126 j:-0.020 k:-0.980 (acc: 179.987 [deg])
Rotation Vector: t:2.264 r:0.153 i:-0.126 j:-0.020 k:-0.980 (acc: 179.987 [deg])
Rotation Vector: t:2.275 r:0.153 i:-0.126 j:-0.020 k:-0.980 (acc: 179.987 [deg])
Rotation Vector: t:2.284 r:0.153 i:-0.126 j:-0.020 k:-0.980 (acc: 179.987 [deg])
.
.
.
```

