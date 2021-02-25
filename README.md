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
	├── api                 # Api tests files
	|	├── automate.py		# python script to post data to anabi dw server
	|	├── hardwareTest	# Directory containing micropython code for the esp32
	|	├── images
	├── docs                # Documentation files (alternatively `doc`)
	├── images              # Image files
	├── loratests           # arduino and esp32 loratests files     
	├── actions.txt			# Work actions 
	└── README.md

