## How to use the sensors?

The Healthy Pi v3 board itself has provisions for biometric sensors that can detect human vital signs. Using these sensors in an accurate way can play a crucial role in getting the correct readings. Let’s take a closer look.

1) SPO2 probe

The Spo2 probe also known as the Saturation of Peripheral Oxygen is a handy probe that can help to detect the level of Oxygen in the blood.

Tip: 
* Gently open the probe and place your index finger inside.
* To ensure that the finger is placed in the middle of the probe.
* Correct placement of the finger can ensure accurate readings.
* You can also ensure that the probe is not exposed to excess light. It can be kept in the dark.


2) ECG Electrodes

ECG or Electrocardiogram helps to measure the electrical activity of the heart. To measure this a set of electrodes are placed on different parts of the body.

![ecg cable connection](images//ecg cable connection.jpg]

* Red - Attach the red electrode to the right side of your chest.
* Blue - Attach the blue electrode to the left side of your chest. 
* Black - Attach the black electrode to the right side of your lower waist.

3) Temperature sensor

The MAX 30205 body temperature sensor helps to detect the body temperature through the skin using the sensor.

Tip:
* Hold the temperature sensor using the index finger and the thumb and press gently. You can find the values changing in the lower section of the screen.

## Powering up the Healthy Pi

Healthy Pi offers some very flexible options when it comes to its use as well as its applications. The very basic step to get started with the board is by powering it up. The board needs a charge of 5V to get powered up. There are some options to powering up your board. They are:-

* Using a Battery HAT

    The board can be attached with a Battery HAT. The basic 5V can be provided by the Battery HAT that is also rechargeable through the USB port.

* Using Power banks

    The board can also be powered using Power banks that are actually easier and handy in carrying around.

* Using USB cables

     The board can be connected to any system/device using the USB cable. This would power up the board in standalone operations.

## Troubleshooting

### With the Display
| Issue                                                | Solution                                                                            |
|------------------------------------------------------|-------------------------------------------------------------------------------------|
| If everything is assembled but the display is blank? | Check the “Y” cable provided.                                                       |
|                                                      | Ensure that the boards are plugged in.                                              |
|                                                      | There may be loose connection in the connecting cable from display to Raspberry Pi3 |
| Raspberry Pi is powering up but still rebooting?     | There might be insufficient power. Check the power source.                          |
|                                                      | The sd card may be corrupted( Check for preloaded software)                         |
|                                                      | Use the power adapter provided in the kit.                                          |
| Raspberry Pi booted but UI not starting up?          | The operating system might not be upgraded.                                         |
| UI is stable but, nothing is plotted?                | Healthy Pi board is not properly plugged in.                                        |
|                                                      | Firmware may be corrupted, needs to be reloaded.                                    |
| The vital parameters are noisy. Why?                 | Check the power source, use the adapter provided.                                   |
|                                                      | The sensors need to be properly connected to the Healthy Pi board.                  |
|                                                      | The sensors need to be securely attached to the body.                               |


### Healthy Pi with the Raspberry Pi

| Issue                                                          | Solution                                                                                                |
|----------------------------------------------------------------|---------------------------------------------------------------------------------------------------------|
| Which OS needs to be used?                                     | The Raspbian software with the latest release.                                                          |
| How can you set up the UI?                                     | Install the OS, enter the following in the terminal                                                     |
|                                                                | curl -sS http://pi.protocentral.com/pi.sh | sudo bash                                                   |
| Why does error occur once the command runs?                    | Ensure that it runs in the Superuser mode.                                                              |
| What if the UI opens, the screen is blank and no plotting?     | Check the Healthy Pi board connection. If the firmware is outdated or corrupted, reload it.             |
|                                                                | Healthy Pi board communicates with Raspberry Pi OSM serial interface, ensure the serial lines are free. |
|                                                                | ( Note: On the display screen, observe the temperature values. If there are changes then it is working) |
| What if the signal quality is bad or the vitals are incorrect? | Check the power adapter, use an isolated power supply to power up the Hpi.                              |
|                                                                | Check the sensor connection         
 
### Standalone Healthy Pi with Desktop

| Issue                                                        | Solution                                                                                                           |
|--------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------|
| The board is plugged in but the USB is not getting detected. | Healthy Pi board is preloaded with the Arduino M0 bootloader which is pre-installed. However, check the following: |
|                                                              | Check the USB cable                                                                                                |
|                                                              | Change the USB port                                                                                                |
|                                                              | Check whether the Power LED on the board glows.                                                                    |
|                                                              | If the above does not work then check it again with another PC.                                                    |
| If the USB is enumerated then when can you see the output?   | Download the latest release file, run the .exe file.                                                               |
| The application is downloaded but it will not open?          | Give a check on whether it is the latest Java download from this link https://java.com/en/download/                |
| The application opens but there is a “port error”?           | Setup up virtual COM port of the Healthy Pi                                                                        |
| The application is blank/ No streaming                       | Firmware may be outdated, needs to be upgraded.                                                                    |
| Vitals are unclear or noisy                                  | Check the sensors if they are connected                                                                            |
|                                                              | Remove the AC adapter from the laptop, as it adds to the noise.   


*Please submit an [issue on Github](https://github.com/Protocentral/protocentral-healthypi-v3/issues/new) if you face any problems with the HealthyPi.*
