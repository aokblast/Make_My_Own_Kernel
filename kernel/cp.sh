#! /bin/bash

sudo mount ../boot.img /media -t vfat -o loop
sudo cp kernel.bin /media
sudo umount /media
