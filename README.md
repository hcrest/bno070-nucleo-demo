# BNO070 Example Application for Nucleo STM32F411RE

The BNO070 Sensor Hub [Demo app](https://github.com/hcrest/bno070-nucleo-demo) is a set of example code designed to
run on the STM32F411 Nucleo platform included with the BNO070
Development Kit.  This software provides a simple example for
obtaining motion data and configuring the BNO070 and is designed to
work with the [BNO070 Driver](https://github.com/hcrest/bno070-driver).

Further information about the BNO070 can be found on the website for [Hillcrest Labs\(http://hillcrestlabs.com/products/bno070).

## Requirements

* STM32F411 Nucleo board

* Hillcrest BNO070 Development board

Note: The Hillcrest BNO070 Development kit includes all of these components.

Before attempting to build the demo project, install the IAR Embedded
Workbench ARM tools and Nucleo USB drivers according to instruction
with the STM32F411 Nucleo kit.

## Preparing the Software

The demo project is based on two sets of code: the BNO070 driver and
an application that uses it.  Both of these need to be installed in
order to build the project.  If you are familiar with Git, you can use
git clone to obtain the source code.  Otherwise, you can download the
source as two zip files from the Hillcrest github account.  Detailed
instructions for each of these options are provided here.

### Preparing the Project using Git

If you are familiar with git, the easiest way to set up the source code is to run this command:

git clone --recursive git@github.com:hcrest/bno070-nucleo-demo.git

Be sure to use the --recursive option.  This tells git to also retrieve the bno070-driver project in the appropriate subdirectory.

After running this command, a subdirectory named bno070-nucleo-demo
will contain all the source code and project files.

### Preparing the Project downloading zip files from Github

The demo software can also be obtained as two zip files.  

* First, download the two zip files.

  * With a browser, visit https://github.com/hcrest/bno070-nucleo-demo
  
  * Click the "Clone or download" button, then click "Download ZIP"
  
  * The downloaded file will be named bno070-nucleo-demo-master.zip.
  
  * Next, browse to https://github.com/hcrest/bno070-driver
  
  * Again, click the "Clone or download" button, then click "Download ZIP"
  
  * The downloaded file will be named bno070-driver-master.zip

* Next, extract the contents of the zip files.

  * Select bno070-nucleo-demo-master.zip, right click and select "Extract All"
  
  This creates a new folder named bno070-nucleo-demo-master.  Within
  it is ANOTHER folder also named bno070-nucleo-demo-master.

  * Rename this inner folder as bno070-nucleo-demo.
	
  * Move it to the location where you want the project to reside.  For
    brevity, I'll assume this is c:\bno070-nucleo-demo\
  
  * Select the downloaded bno070-driver-master.zip, right click and
    select "Extract All"

  This creates a new folder named bno070-driver-master.  Within it
    is ANOTHER folder also named bno070-driver-master.

  * Rename this inner folder as bno070-driver.

  * Move it under the Drivers folder of the bno070-nucleo-demo
    workspace created previously.  This would be
    c:\bno070-nucleo-demo\Drivers\bno070-driver\, for example.

When finished, the project tree should look like this:

```
bno070-nucleo-demo
|  README.md
|  ...
+-- Drivers
|   +-- bno070-driver
|   |      bno070.c
|   |      ...
|   +-- CMSIS
|   |      ...
|   +-- STM32F4xx_HAL_Driver
|          ...
|
+-- EWARM
+-- Hillcrest
+-- Inc
+-- Middlewares
+-- Src
```

## Building the Code

* Use IAR EWARM to open the workspace file,
  c:\bno070-nucleo-demo\EWARM\Project.eww

* Select Project -> Rebuild All to compile the project.

## Running the Application

* Mount the shield board on the Nucleo platform.

* Connect the Nucleo board to the development PC via USB.

* Open a terminal emulator window on the Nucleo's COM port using
  115200 bits per second, 8 data bits, 1 stop bit, no parity.

* In IAR EWARM, execute Project -> Download and Debug.

* Once the debugger is ready, click the Go button.

The application should print the SH-1 version numbers, then start
reading and printing Rotation Vectors from the SensorHub.

```
SH-1 Demo App : Version 1.1.1
SH-1 Driver   : Version 1.1.1
Part 10003251 : Version 1.8.4 Build 415
Part 10003210 : Version 0.11.0 Build 176
Part 10003254 : Version 3.5.1 Build 162
Part 10003171 : Version 3.5.3 Build 251
Rotation Vector: t:0.198862 r:0.188 i:0.000 j:0.008 k:-0.982 (acc: 22.703 [deg])
Rotation Vector: t:0.208723 r:0.188 i:0.000 j:0.008 k:-0.982 (acc: 22.703 [deg])
Rotation Vector: t:0.218584 r:0.188 i:0.000 j:0.008 k:-0.982 (acc: 22.703 [deg])
Rotation Vector: t:0.228450 r:0.188 i:0.000 j:0.008 k:-0.982 (acc: 22.703 [deg])
.
.
.
```

