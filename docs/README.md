# Bee Keeping Firmware
## Hardware components
### 1.Main controller -> ESP32
-logging data
-receive data from aux board via LoRa 
-post data to anabi DW server

### 2.Auxilliary board -> atmega328p 
-measures weight
-measure ambient temperature
-humidity
-send data to main board via loRa

## Folder structure
	.
	├── c			# C/arduino version of the firmware
		├── mainBoard.ino	#firmware for the main(esp32) board
		├── auxBoard.ino	#firmware for the auxilliary board
	├── micropython	# micropython version of the firmware
	├── pythonTests	# Api python tests
	├── libraries	# Arduino libraries used in the firmware
	├── images  
	└── README.md

## Hardware tools
1. uasbasp programmer
2. Micro-usb cable

## Software tools and libraries used
1. Arduino ide : [download](https://www.arduino.cc/en/software)  
2. Libraries used in the firmware can be obtained in the libraries dirctory where the can be installed as zip from the arduino ide [tutorial](https://www.arduino.cc/en/guide/libraries)

## Burning the bootloader on the aux board
1. connect the programmer's cable head to the icssp header of the auxboard
[icsp images]
![icspheader](images/icspheader1.jpg)
![icspheader](images/icspeader2.jpg)
2. Connect the programmer to your pc via usb
3. Launch the arduino ide
4. Select “USBasp” from the Tools > Programmer menu. 
3. Follow this [tutorial](https://tutorial45.com/arduino-bootloader-fix/) to burn the bootloader ontu the board

## Flashing firmware/uploading to the boards
### 1. Main Board
1. Install the ESP32 Board in Arduino IDE ; [tutorial](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)
1. Connect the board to the pc via the micro-usb cable 
2. launch the Arduino ide and open the mainboard.ino sketch
3. Select your Board in Tools > Board menu (in this case : "ESP32 Dev Module")
4. Select the Port (if you don’t see the COM Port in your Arduino IDE, you need to install the [FT232RL](https://www.usb-drivers.org/ft232r-usb-uart-driver.html) USB to UART Bridge VCP Drivers)
5. Press the upload icon on the arduino ide to upload the firmware to the board
6. Press the reset button on the board

### 2. Aux board
1. Connect the programmer to the aux board as described in the bootloader section
2. Launch the arduino software an open the auxboard.ino sketch
3. Select “USBasp” from the Tools > Programmer menu.
4. Select “Upload using Programmer” from the File menu. The uploading process should begin The uploading process is done when Arduino IDE display “Done uploading”

## Board component connection and fittings
This is a pictorial list of auxilliary component connection and enclosure fittings for the two boards
### 1. Main board
<img src="images/main.jpg" height="500"></img>
#### Battery

<p align="center">
	<img src="images/mainbat.jpg" height="500"></img>
	<img  height="500" src="images/mainbat2.jpg"></img>
</p>

#### Fitting
![main board fit]

### 2. Aux Board
<img src="https://github.com/An-bi/Firmware/blob/main/images/aux.jpg" height="400"></img>
#### Load cell and dht22 sensor
<img  height="500" src="images/loadcell.jpg"></img>

###### four wire load cell connection
| Aux Board | Load cell  |
|-----------|------------|
| vcc       | red        |
| gnd       | black      |
| A-        | white      |
| A+        | green      |

###### three wire/quater bridge load cell connection
<img  height="500" src="images/quarterload.jpg"></img>

#### Battery 
![aux boad bat]
#### fitting
![aux board fit]

## Board modifications
The following is a pictorial list of the hardware modifications done to the main board:

### 1. Reversing the Schottky diode D1

<img src="https://github.com/An-bi/Firmware/blob/main/images/d1.jpg" height="500"></img>

### 2. R3 resistor -> 0k 
Short the resistor r3 or just solder a 0k ohms resistor in its place

<img src="https://github.com/An-bi/Firmware/blob/main/images/r3.jpg" height="500"></img>

### 3. Main board Firmware upload modification
For firmware to be flashed ontu the main board it has to be in download boot mode(flash mode) and for this gpio0(which is pulled up) and gpio2(pulled-down) need to be at logic level 0 so for this immediately you press the upload button on the ide 
hold the boot and the reset button then release the reset button and the firmware should be uploaded to the board.

## Firmware Configurations
### 1. Main Board firmware

byte localAddress -> this is the adress of the main board
int loraTimeout -> This is the amount of time in seconds that the main board will listen/wait for a LoRa packet from the aux board before sending its payload(without the aux payload)
#### Source ids
const char mainTemp = "your main board temperature source id";

const char mainAudio = "your aux board audio source id";

const char auxTemp = "your aux board temperature source id";

const char auxHum = "your aux board humidity source id";

const char auxWeight = "your aux board weight source id";


### 2. Aux Board firmware

byte localAddress -> this is the address of the aux board

byte mainBoardAddress -> this is the address of the main board that pairs with this aux board

int interval -> this is the interval between LoRa packet sends

float calibrationFactor -> this is the load cells calibration factor

## Data in the Anabi website
<img  height="500" src="images/anabi2.png"></img>


