#!/bin/bash
#  This script directly adheres to the recommendations found at http://wiki.osdev.org/GCC_Cross-Compiler
#
# ./script <destination-dir> <build-dir>
# Running this script will take about 10-15 minutes to complete, depending on your system

BINUTILS_VER=2.28
GCC_VER=7.1.0
DEST_DIR=$1
BUILD_DIR=$2
STARTTIME=$SECONDS

CROSS_DIR=$DEST_DIR
mkdir -p $CROSS_DIR

export PREFIX="$CROSS_DIR"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# Build
mkdir -p $BUILD_DIR && cd $BUILD_DIR

# wget binutils/gcc
wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.bz2
wget ftp://ftp.mirrorservice.org/sites/sourceware.org/pub/gcc/releases/gcc-$GCC_VER/gcc-$GCC_VER.tar.bz2

# Binutils
tar -axf binutils-*
mkdir build-binutils
cd build-binutils
../binutils-*/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
cd ../

# GCC
tar -axf gcc-*
mkdir build-gcc
cd build-gcc
../gcc-*/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc all-target-libgcc
make install-gcc install-target-libgcc
cd ../

# Check
echo "Done; Be sure to add $DEST_DIR to your PATH"
echo "Total seconds taken: $(($SECONDS-$STARTTIME))"

