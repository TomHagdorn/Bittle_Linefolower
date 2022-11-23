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


## Installation
1. Install the Arduino IDE
2. Install the Arduino Extension for Visual Studio Code
3. Install OpenCV
4. Clone this repository
5. Open the folder in Visual Studio Code
6. To be continued...


## [Tutorial](<https://i40.fh-aachen.de//courses/dta/activities/bittle/workspace_setup.html#node-red>)
## Dependencies 
<details>
<summary>Install dependencies on Linux</summary>
<br>
Install Node-Red
Check if Node.js and npm are installed
```
node -v
npm -v
```
If not installed, install Node.js and npm
```
$ sudo apt-get install nodejs
$ sudo apt-get install npm
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

## Tips and tricks
<details>
<summary>Bittle serial cmds</summary>
<br>

![Getting Started](./images/serial_commands.png)


</details>


