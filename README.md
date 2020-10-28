# ProtoCentral HealthyPi v3 HAT for Raspberry Pi

![Healthy Pi v3 HAT](docs/images//hpi3-with-display.jpg)

HealthyPi is the first fully open-source, full-featured vital sign monitor. Using the Raspberry Pi as its computing and display platform, the HealthyPi add-on HAT turns the Raspberry Pi into a vital sign monitoring system.

If you don't already have one, you can now buy HealthyPi from any of the following places:

* [Crowd Supply](https://www.crowdsupply.com/protocentral/healthypi-v3)
* [ProtoCentral India (Worldwide)](https://www.protocentral.com/open-source-health/1252-healthypi-v3-complete-kit-includes-raspberry-pi.html)
* [DigiKey](https://www.digikey.in/product-detail/en/crowd-supply-inc/CS-HEALTHYPI-01/1848-1035-ND)
* [Mouser](https://www.mouser.in/ProductDetail/392-CS-HEALTHYPI-01)

Repository Contents
-------------------
* **/firmware** - Atmel studio  and aduino example sketches.
* **/Hardware** - All Eagle design files (.brd, .sch)
* **/extras** - includes the datasheet
* **/gui** - HealthyPi GUI for Raspberry Pi and PC
* **/docs** - Documentation

Getting Started (On Raspberry Pi):
----------------------------------

### For getting started videos and tutorials, please go to [http://healthypi.protocentral.com](http://healthypi.protocentral.com)

Mount ProtoCentral's HealthyPi-v3 HAT carefully onto the Raspberry Pi Board's GPIO Pins. Connect this setup with the Raspberry Pi's Display or a HDMI Monitor. The HAT communicates with Raspberry Pi using UART interface on the GPIO pin header. Connect the ECG electrodes and SPO2 Probe to the ProtoCentral's Healthy PI Hat.

It's now easy to get started with our brand-new installation script. Just follow the following steps on your Raspberry Pi.

To start, open up the terminal window (Menu -> Accessories -> Terminal) on Raspbian running on your Raspberry Pi:

![Open the terminal](docs/images/terminal.jpg)

In the terminal window, type the following lines.

```bash
curl -sS http://pi.protocentral.com/hpi3.sh | sudo bash
```

It really is as simple as that !!

This code will install all the required overlays, configuration and application files to get the Healthy Pi up and running.

After the script reboots, you should be able to see the GUI display on the screen

![GUI in Processing](docs/images/hpi3-screen.jpg)

### This completes the install!

If the above script does not work for you and would like to do a manual installation or would like to customize the code, check out [Advanced HealthyPi] (/docs/advanced-healthypi.md).

Using the HealthyPi GUI on Windows, MacOS and Linux
---------------------------------------------------
The HealthyPi board now streams the same data on the on-board USB port. This allows you to get the same data that goes to the Raspberry Pi, now on your desktop PC as well.

Java 8 is required on all platforms for running the processing-based GUI application. You can download Java for your platform from the following link.

[https://java.com/en/download/](https://java.com/en/download/)

#### Installing drivers (only for Windows)

HealthyPi uses the same drivers as an Arduino Zero. Once plugged in to the USB port, the device would be recognized as an "Unknown Device". You can locate the device is the Windows Device Manager and manually install the drivers provided in the "drivers" folder in the Windows Executable ZIP archive provided.

MacOS and Linux do not need any drivers to be installed.

### Processing GUI Installation

Download the zip file containing the executable files from the releases section for 32-bit/64-bit Windows. If you do not know if you have a 64-bit or 32-bit computer, please download the 32-bit version.

[Download the latest GUI for HealthyPi](https://github.com/Protocentral/protocentral-healthypi-v3/releases/latest)

Simply download the appropriate file for your operating system, unzip the contents and run the executable program contained in it.

### For more documentation, videos and tutorials, please go to [http://healthypi.protocentral.com](http://healthypi.protocentral.com)

License Information
===================

This product is open source! Both, our hardware and software are open source and licensed under the following licenses:

Hardware
---------

**All hardware is released under [Creative Commons Share-alike 4.0 International](http://creativecommons.org/licenses/by-sa/4.0/).**
![CC-BY-SA-4.0](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)

You are free to:

* Share — copy and redistribute the material in any medium or format
* Adapt — remix, transform, and build upon the material for any purpose, even commercially.
The licensor cannot revoke these freedoms as long as you follow the license terms.

Under the following terms:

* Attribution — You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
* ShareAlike — If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original.

Software
--------

**All software is released under the MIT License(http://opensource.org/licenses/MIT).**

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


Please check [*LICENSE.md*](LICENSE.md) for detailed license descriptions.
