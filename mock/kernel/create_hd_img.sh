#!/bin/bash

DISK_SIZE=1024*1024*50
SECTOR_SIZE=512
START_SECTOR=2048
END_SECTOR=$(($DISK_SIZE/$SECTOR_SIZE - 1))

rm -f qemu_hd.bin
dd if=/dev/zero of=qemu_hd.bin bs=1M count=50
echo -e "o\nn\np\n1\n$START_SECTOR\n$END_SECTOR\nt\n7\na\n1\nw\n" | fdisk -b$SECTOR_SIZE -H255 -S63 qemu_hd.bin
#echo -e "d\nn\n\n\nbt7\ngc255\ns63\nWq" | cfdisk qemu_hd.bin
dd if=arch/i386/boot/rm_loader.bin of=qemu_hd.bin conv=notrunc bs=1 count=$((512 - 16*4 + 2))
#dd if=feeb.bin of=qemu_hd.bin conv=notrunc bs=1 seek=$((2048*512)) count=512
