<h1 align="center">Lightweight ESP32-based 3D Printer Nozzle Clumping Detection</h1>

<p align="center">
  CEG4195 Project <br/>
  Faculty of Electrical and Computer Engineering @   University of Ottawa
  <br/>
</p>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Author-Alex%20Gordon (Aetriq on Github)-blue" alt="Author Badge"/>
  <img src="https://img.shields.io/badge/Status-In%20Development-orange" alt="Status Badge"/>
</p>

## Update Log
**6-8-26:** I'm currently beginning physical implementation of this on my ![custom 3D printer](https://aetriq.xyz/intrastice). Based on requirements, I have the current model's behaviour as follows:

<p align="center">
  <img src="https://github.com/Aetriq/ESP32CAM-NozzleClumpingDetection/blob/main/img/repo-img/3states.png?raw=true" alt="Repo img" width="40%"/>
</p>

Three states within the model are blob, clean and extruded. Extruded is the intermediate state preventing false positives.

<p align="center">
  <img src="https://github.com/Aetriq/ESP32CAM-NozzleClumpingDetection/blob/main/img/repo-img/State.png?raw=true" alt="Repo img" width="60%"/>
</p>

This is mostly final, only making improvements for things like deadlocks, etc.

The high level H/W setup diagram is below. This is fairly simple and modular:
<p align="center">
  <img src="https://github.com/Aetriq/ESP32CAM-NozzleClumpingDetection/blob/main/img/repo-img/circuit.png?raw=true" alt="Repo img" width="60%"/>
</p>

The actual trained model post transfer learning is looking very good. This is only on a 1250 image 82/18 dataset, but even quantized to int8 (which is basically required) yields a near 97% accuracy...

<p align="center">
  <img src="https://github.com/Aetriq/ESP32CAM-NozzleClumpingDetection/blob/main/img/repo-img/data.jpg?raw=true" alt="Repo img" width="60%"/>
</p>

## Project Description
A very lightweight and quick nozzle based clumping detection for Klipper-based 3D printers. Unlike other print failure detecting software, this system is done at the macro nozzle level to identify and resolve potential extrusion failiures mid-print. It will check the nozzle every fixed amount of grams of filament extruded and attempt to fix itself with the components 'on hand' before automatically pausing the print and notifying the operator.

---

<p align="center">
  <img src="https://github.com/Aetriq/ESP32CAM-NozzleClumpingDetection/blob/main/img/repo-img/topviewrender.png?raw=true" alt="Repo img" width="90%"/>
  <img src="https://github.com/Aetriq/ESP32CAM-NozzleClumpingDetection/blob/main/img/repo-img/coolcloseuprender.png?raw=true" alt="Repo img" width="90%"/>
</p>

---

## Features
- Based on the ESP32-S based ESP32-CAM with an OV2640 Camera. Should work on any ESP32 with a camera module. Quality/FPS is not a priority here as long as the camera can take close-up shots.
- Instead of being a secondary MCU, it connects directly with the onboard moonraker API for communication.
- Very customizable firmware. Can set thresholds, custom gcode actions, etc.
- This project was made with [Edge Impluse.](https://www.edgeimpulse.com/)


## Steps to Install
### Firmware (ESP32 CAM Client Side)
There are two methods to install. 

---

#### Method 1: PlatformIO 
PlatformIO is used if you want to compile the firmware yourself.

1.  Download and install [Visual Studio Code](https://code.visualstudio.com/).
2.  Open VS Code.
    * Click the Extensions icon on the left sidebar.
    * Search for "PlatformIO IDE" and click Install.
3.  Download the project source code and open the folder in VS Code.
4.  Plug your ESP32 into your computer via USB.
5.  Build and Flash:
    * Click the PlatformIO icon.
    * Under Project Tasks, select Build to compile the code.
    * Once successful, select Upload to flash the firmware to your device.
---

#### Method 2: Compiled .bin from Releases
If you don't need to change the code and just want to flash a pre-compiled `.bin` file, use the **Espressif Flash Download Tool**.

1. Go to the Releases section of the GitHub repository. Download the `firmware.bin`
2. Download the [Espressif Flash Download Tool](https://www.espressif.com/en/support/download/other-tools) (Windows only). 
    * *Note: For Mac/Linux users, use the command-line tool `esptool.py`.*
3.  Configure the Flash Tool:
    * Select Developer Mode -> ESP32 DownloadTool.
    * Load your files and set the memory addresses (Standard ESP-IDF defaults: `0x1000` for bootloader, `0x8000` for partitions, and `0x10000` for the app).
4.  Flash:
    * Select the correct COM Port and set the Baud rate (115200 is standard).
    * Click START.

### Software (Host Side)
#### Python service (via SSH)
This guide explains how to install a `.py` script on a headless server via SSH so that it runs automatically on startup using `systemd`.

1. Prepare the Environment
  * First, make sure your script is on the server and you have a virtual environment set up (recommended to avoid dependency conflicts).
  * WinSCP is a good choice to use to upload the file, or simply copy and paste it and use sudo nano  

2. SSH into your printer and run the following:

```bash
mkdir ~/nozzle_checker
cd ~/nozzle_checker
nano nozzle_check.py
# Create the virtual environment
python3 -m venv venv

# Install the required libraries (pyserial and requests)
./venv/bin/pip install pyserial requests
```
3. Create the service for your pi as seen below:

``` bash
sudo nano /etc/systemd/system/nozzle_check.service
```
Paste the following configuration into the file (Note: If your username is not pi, change /home/pi/ to your actual home path, e.g., /home/mainsail/):
``` bash
[Unit]
Description=Klipper Nozzle Serial Monitor Service
After=network.target

[Service]
# Change 'pi' to your linux username if different
User=pi
WorkingDirectory=/home/pi/nozzle_checker
ExecStart=/home/pi/nozzle_checker/venv/bin/python nozzle_check.py
Restart=always
RestartSec=5
StandardOutput=inherit
StandardError=inherit

[Install]
WantedBy=multi-user.target
```

4. Run these commands to activate the service:
``` bash
# Reload systemd to recognize the new service
sudo systemctl daemon-reload

# Enable the service to start on every boot
sudo systemctl enable nozzle_check.service

# Start the service immediately
sudo systemctl start nozzle_check.service
```

#### Klipper script
Simply add it on top of your existing klipper printer configuration. Make sure to edit the user variable positions to your printer.




---

## License & Academic Attribution
As stated above, this project was developed as a university project for CEG4195 at the University of Ottawa. 

The code, documentation, and assets in this repository are provided for portfolio and educational review purposes. This project **may not be explicitly shared, reproduced, or submitted as original work by others without proper attribution** to the original author. If you wish to use or build upon this work, please provide a clear reference to this repository. Please respect the academic integrity regarding plagiarism and uncredited code reuse; any repositories or reuse of my code without explicit attribution will be taken down in accordance to the University of Ottawa's [Academic Regulation A-4 – Academic Integrity and Academic Misconduct](https://www.uottawa.ca/about-us/leadership-governance/policies-regulations/a-4-academic-integrity-academic-misconduct).
