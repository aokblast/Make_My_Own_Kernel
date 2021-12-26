#! /bin/bash

echo -e "1\nfd\n\nboot.img\n" | bximage

nasm boot.asm -o boot.bin

dd if=boot.bin of=boot.img bs=512 count=1 conv=notrunc

mkdir ./mnt

sudo mount ./boot.img ./mnt -t vfat -o loop

nasm loader.asm -o loader.bin

sudo cp loader.bin ./mnt

sudo umount ./mnt

rm -rf ./mnt

