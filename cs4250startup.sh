#!/bin/bash

echo stop | sudo tee /sys/class/remoteproc/remoteproc0/state 2>/dev/null || true
sudo duo-pinmux -w B11/IIC1_SDA
sudo duo-pinmux -w B12/IIC1_SCL
sudo duo-pinmux -l | grep -E "B11|B12"
sudo /usr/sbin/i2cdetect -y -r 1

sudo cp /home/debian/script/projectthing.ino.elf /lib/firmware/projectthing.ino.elf

if [ cat /sys/class/remoteproc/remoteproc0/state running ]; then
	echo stop | sudo tee /sys/class/remoteproc/remoteproc0/state > /dev/null
fi

echo projectthing.ino.elf | sudo tee /sys/class/remoteproc/remoteproc0/firmware
echo start | sudo tee /sys/class/remoteproc/remoteproc0/state > /dev/null

echo stop  | sudo tee /sys/class/remoteproc/remoteproc0/state > /dev/null
echo start | sudo tee /sys/class/remoteproc/remoteproc0/state > /dev/null
