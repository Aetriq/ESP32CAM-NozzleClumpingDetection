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

## Steps to Install
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
