Recompiling Firmware:
---------------------

All boards shipped on or after February 20, 2017 come **pre-installed** with the Arduino Zero/M0 bootloader and the Arduino-based code. This means that you no longer need a JTAG programmer or Atmel Studio to customize the code on the SAMD21 microcontroller on the board. 

In addition, the new Arduino-based firmware now **streams over USB** the same data sent to the Raspberry Pi's UART pins. This now means that the HealthyPi works not just with a Raspberry Pi, but any Windows/MAC/Linux machine that supports Java and Processing.
 
However, if you would like to edit the firmware, here's the steps to do that in Atmel Studio 7:

* **Atmel Studio 7** :
Please note that Atmel Studio 7 is available only for Windows and does not support Mac and Linux . This is the IDE software that can do step & memory debugging and its only for Windows. Also you have to make an account on Atmel's site. The IDE can be downloaded from [Atmel Studio's Official Website](http://www.atmel.com/tools/atmelstudio.aspx)

**Load the Healthy Pi Source Code**

* **Step 1 :** Start by launching Atmel Studio 7.

* **Step 2 :** Open the Project file in the firmware folder of this git respitory

* **Step 3 :** You'll see the following, where the sketch is in a window, you can edit the code here if you like.

**Debugger**

You need a debugger to load the code into the SAMD21G18A. Atmel-ICE will help get from here http://www.atmel.com/tools/atatmel-ice.aspx

**Step 1 : Set Up and Check Interface**
* Attach the chip & debugger

**Step 2 : Identify Interface**
* OK now you have your debugger plugged in, its good to check that it works, select Device Programming.
* Choose "Device Programmer" from "Tools" Menu
* Programming window will be open.
* Select the programmer from the drop down box. Select interface as “SWD”. Click on “Apply”.
* Now read the Power and Device ID. If you are able to read the Device ID then the interface is fine.

**Step 3 : Build & Start Debugging**
* Ok close out the modal programming window and build the program. If there are no error, then your build will be successful.

**Step 4 : Run & Upload**
* You will see a window appearing to select a debugging tool. Select the tool and interface.
* Once done, go back and Re-run Program without debugging.