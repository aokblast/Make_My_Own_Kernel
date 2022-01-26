#! /bin/bash

sudo mount ../boot.img /mnt -t vfat -o loop
sudo cp kernel.bin /mnt
sudo umount /mnt
