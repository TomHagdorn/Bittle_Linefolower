# Robot_Dog
Bittle Robot Dog linefollower


Tutorial

<https://i40.fh-aachen.de//courses/dta/activities/bittle/workspace_setup.html#node-red>

Basic Setup

Install Node-Red
```
$ sudo apt-get install nodejs
$ sudo apt-get install npm
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

Bittle serial cmds
![Getting Started](./images/serial_commands.png)





