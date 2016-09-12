# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|
  config.vm.box = "ubuntu/trusty64"
  
  config.vm.provider "virtualbox" do |v|
    v.memory = 1536
    v.cpus = 4
  end
  
  config.vm.provision "shell", inline: <<-SHELL
    apt-get update
    apt-get install -y libgmp3-dev libmpfr-dev libisl-dev libcloog-isl-dev libmpc-dev texinfo make bison flex gcc g++ nasm build-essential grub qemu zip xorriso
    
    export PREFIX="/opt/cross64"
    export TARGET=x86_64-elf
    export PATH="$PREFIX/bin:$PATH"
    
    mkdir /tmp/
    mkdir /tmp/build
    cd /tmp/build
    
    wget -O binutils.tar.gz http://ftp.gnu.org/gnu/binutils/binutils-2.27.tar.gz
    tar -xf binutils.tar.gz
    
    wget -O gcc.tar.gz http://ftp.gnu.org/gnu/gcc/gcc-6.2.0/gcc-6.2.0.tar.gz
    tar -xf gcc.tar.gz
    
    # Build binutils and gcc for the local system first:
    
    cd binutils-2.27
    mkdir build-binutils-local
    cd build-binutils-local
    ../configure --prefix="/opt/gcc-6.2.0" --disable-nls --disable-werror
    make
    make install
    
    cd ../../gcc-6.2.0
    mkdir build-gcc-local
    cd build-gcc-local
    ../configure --prefix="/opt/gcc-6.2.0" --disable-nls --enable-languages=c,c++
    make
    make install
    
    export PATH="$PREFIX/bin:/opt/gcc-6.2.0/bin:$PATH
    
    # Now build the cross compiler toolchain:
    
    cd ../../binutils-2.27
    mkdir build-binutils-cross
    cd build-binutils-cross
    ../configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
    make
    make install
    
    cd ../../gcc-6.2.0
    mkdir build-gcc-cross
    cd build-gcc-cross
    ../configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
    make all-gcc
    make all-target-libgcc
    make install-gcc
    make install-target-libgcc
  SHELL
  
end
