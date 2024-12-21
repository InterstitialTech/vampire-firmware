# vampire-firmware
## firmware for the Vampire ESP32

### Dependencies
```
git clone -b v4.1.1 --recursive https://github.com/espressif/esp-idf.git esp-idf-v4.1.1
cd esp-idf-v4.1.1/
./install.sh
./export.sh
```

### Build and Flash
```
# from the repo top-level
idf.py build
idf.py -p /dev/ttyUSB0 flash
```
