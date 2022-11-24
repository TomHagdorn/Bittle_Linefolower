# Robot Dog Bittle - Linefollower
This is a linefollower for the robot dog Bittle. It is based on the [ESP32-CAM](https://www.espressif.com/en/products/hardware/esp32-cam/overview) and uses a linefollower program to detect the line. The ESP32-CAM is connected to the [Bittle](https://www.petoi.com/products/petoi-bittle-robot-dog). 
The bittle is controlled via serial communication. The serial communication is done via the [ESP32-CAM](https://www.espressif.com/en/products/hardware/esp32-cam/overview) and the [Bittle](https://www.petoi.com/products/petoi-bittle-robot-dog).
The linefollower is written in C++. The code is written for the ESP32-CAM.

I will add a slightly more detailed description later.

## Hardware
- [ESP32-CAM](https://www.espressif.com/en/products/hardware/esp32-cam/overview)
- [Bittle](https://www.petoi.com/products/petoi-bittle-robot-dog)

## Software
- [Arduino IDE](https://www.arduino.cc/en/software)
- [OpenCV](https://opencv.org/)
- [Visual Studio Code](https://code.visualstudio.com/)
- [Arduino Extension](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino)
- [NodeRed](https://nodered.org/)

## Dependencies for serial communication with node-red
<details>
<summary>Install dependencies on Linux</summary>
<br>
Install Node-Red

Check if Node.js and npm are installed
```
node -v && npm -v
```

If not installed, install Node.js and npm
```
$ sudo apt-get install nodejs && sudo apt-get install npm
```

Install Node-Red
```
$ sudo npm install -g --unsafe-perm node-red
```

Install Node-Red Dashboard
```
$ cd ~/.node-red
$ npm install node-red-dashboard
```

Install Node-Red Node for Bittle
```
$ cd ~/.node-red
$ npm install node-red-contrib-bittle
```

Install Node-Red Serialport
```
$ cd ~/.node-red
$ npm install node-red-node-serialport
```

Install OpenCV ESP32 Library
``` 
$ cd ~/Arduino/libraries
$ git clone
$ cd ~/Arduino/libraries/opencv_esp32
$ git submodule update --init
``` 
</details>

<details>
<summary>Install dependencies on Windows</summary>
<br>
Check if Node.js and npm are installed
```
node -v && npm -v
```

If not installed, install Node.js and npm on Windows
```
https://nodejs.org/en/download/
```

Install Node-Red Windows
```	
$ npm install -g --unsafe-perm node-red
```
Install Node-Red Dashboard
```
$ cd %userprofile%\.node-red
$ npm install node-red-dashboard
```

Install Node-Red Node for Bittle
```
$ cd %userprofile%\.node-red
$ npm install node-red-contrib-bittle
```

Install Node-Red Serialport
```
$ cd %userprofile%\.node-red
$ npm install node-red-node-serialport
```

Install OpenCV ESP32 Library
```
$ cd %userprofile%\Documents\Arduino\libraries
$ git clone
$ cd %userprofile%\Documents\Arduino\libraries\opencv_esp32
$ git submodule update --init
```
</details>



## Installation
1. Install the Arduino IDE
2. Install the Arduino Extension for Visual Studio Code
3. Install OpenCV
4. Clone this repository
5. Open the folder in Visual Studio Code
6. To be continued...


## [Tutorial](<https://i40.fh-aachen.de//courses/dta/activities/bittle/workspace_setup.html#node-red>)

## Tips and tricks
<details>
<summary>Bittle serial cmds</summary>
<br>

![Getting Started](docs/images/bittle_serial_cmds.png)


</details>


## Printed parts
<details>
<summary>Printed parts</summary>
<br>

GummifussV2.stl

    Covers the feet of the Bittle with TPU or PLA. This prevents the Bittle from slipping on the floor.
    The PLA needs to be Rubber coated and the TPU needs to be very soft.

CAM-Holder.stl

    Holds the ESP32-CAM on the Bittle. The ESP32-CAM is screwed on the holder. 
    The holder is clipped where the Bittle's head would be.


Skelleton.stl

    Skelleton cover for the back of the Bittle. It is printed in PLA+ and is for weight reduction of 0.5g.
    This is not necessary, but it looks cool.

standWithCalibration.stl

    Stand for the Bittle. It is printed in PLA+ and it is for testing and calibration without risking the Bittle to fall down.
</details>

## [Official Repository](https://github.com/PetoiCamp)
