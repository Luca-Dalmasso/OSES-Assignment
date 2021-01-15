# OSES-Assignment: A Layer for a custom Yocto-Distribution
RasperryPi3 + QemuARM

# LAYER INSTRUCTIONS FOR QEMU ARM

This repo contains a layer including the two recipes: one for building and deploying the APP and one for the cDD
 hearth rate monitor, which is made of two components:
 1)A Linux character-based driver (cDD) used to access a “virtual” Photopletismography(PPG) sensor, 
 2)Linux user application (APP)

This README file contains information on the contents of the 'OSES-assignment' layer.
Please see the corresponding sections below for details.

I suppose that it has been done a previous yocto setup & build for ARM target NAMED "build-qemuarm",
this instructions just show how to integrate a new layer in a yocto distribution.

STARTING FROM your poky directory (cd path_to_poky):

# 0:Clone the repo, and source to the build environment directory
    cd path_to_poky
    git clone https://github.com/Luca-Dalmasso/OSES-Assignment.git
    --Now should have a directory: OSES-Assignment, which is our layer
    source oe-init-build-env build-qemuarm
    
# 1: Add the layer
    bitbake-layers add-layer ../OSES-Assignment
    
# 2: Check if the Layer has been added (optional if you have done #1)
    cd conf
    cat bblayers.conf
    --check that after the field 'BBLAYERS ?= "' you have something like 'poky/OSES-Assignment \'
    --if not then write it manually
    
# 3: Add to the image the driver+user application
    --add at the end of 'local.conf' the following lines:
    IMAGE_INSTALL_append = " driver"
    KERNEL_MODULE_AUTOLOAD += " driver"
    IMAGE_INSTALL_append = " userapp"

# 4: Compile new image
    bitbake core-image-minimal
    
# 5: Test the application
    runqemu qemuarm
    --login as root
    --you can use command: dmesg, to see if 'driver' kernel module has been correctly loaded
    --find out what is the Major number: i suppose it is 251 0 [major,minor]
    cat proc/devices | grep driver
    --create the associated device file named virtual_sensor
    mknod /dev/virtual_sensor c 251 0
    --now the application can comunicate to the device file, start the application:
    userapp

# LAYER INSTRUCTIONS FOR RASPBERRYPI3, STARTING FROM QEMUARM BUILD
	--in order to install everithing on the rasperry only few more steps are needed. 
	--this will take a lot of time to compile (from scratch) everything for the new machine.
   
# 6: Prerequisites:
	--You will "convert" all the previuous work just to be compatible with the RPI3
	
# 7: Add new remote-layers to poky
	--Needed in order to build a custom yocto distro for RPi3
	git clone -b dunfell git://git.openembedded.org/meta-openembedded
	git clone -b dunfellhttps://github.com/meta-qt5/meta-qt5
	git clone -b dunfell git://git.yoctoproject.org/meta-security
	git clone -b dunfell git://git.yoctoproject.org/meta-raspberrypi
	
	--edit 'bblayers.conf' file, in order to add new layers --> THE SAME FILE OF STEP #2
	--should looks like this:
	
	POKY_BBLAYERS_CONF_VERSION = "2"

	BBPATH = "${TOPDIR}"
	BBFILES ?= ""

	BBLAYERS ?= " \
  	/poky/meta \
  	/home/usr/poky/meta-poky \
  	/poky/meta-yocto-bsp \
  	/poky/meta-openembedded/meta-oe \
  	/poky/meta-openembedded/meta-multimedia \
  	/poky/meta-openembedded/meta-networking \
  	/poky/meta-openembedded/meta-python \
  	/poky/meta-raspberrypi \
  	/poky/OSES-Assignment \
  "
# 8: Add new configurations to local.conf
	--same file as #3
	--Comment the line MACHINE ?= "qemuarm", now we are changing architecture
	--add following lines:
	MACHINE ?= "raspberrypi3"
	ENABLE_UART = "1"
	IMAGE_FSTYPES = "tar.xz ext3 rpi-sdimg"
	
# 9: Modify driver.bb file in the driver layer
   --bitbake needs to compile this kernel module to be compatible with raspberry machine so:
   cd ../../OSES-Assignment/recipes-example/driver
	--modify the "COMPATIBLE_MACHINE" field like this:
	COMPATIBLE_MACHINE = "raspberrypi3"
	--compile everything
	bitbake core-image-minimal
	
# 10: Micro SD setup
	--you cannot test this on qemu as before.
	--Suppose that your SD card is on /dev/sdb, (check it with lsblk command)
	sudo dd if=tmp/deploy/images/raspberrypi3/core-image-minimal-raspberrypi3.rpi-sdimg of=/dev/sdb bs=1M
	
