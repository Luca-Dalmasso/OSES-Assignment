# OSES-Assignment: A Layer for a custom Yocto-Distribution
This repo contains a layer including the two recipes: one for building and deploying the APP and one for the cDD
 hearth rate monitor, which is made of two components:
 1)A Linux character-based driver (cDD) used to access a “virtual” Photopletismography(PPG) sensor
 2)Linux user application (APP)

This README file contains information on the contents of the 'OSES-assignment' layer.
Please see the corresponding sections below for details.

I suppose that it has been done a previous yocto setup & build for ARM target NAMED "build-qemuarm",
this instructions just show how to integrate a new layer to a yocto distribution.

STARTING FROM your poky directory (cd path_to_poky), after you cloned this project inside the directory:

# 0:Clone the repo and source to build directory
		cd path_to_poky
		git clone https://github.com/Luca-Dalmasso/OSES-Assignment.git
		--Now should have the layer: OSES-Assignment
    source oe-init-build-env build-qemuarm
    
# 1: Add the layer
    bitbake-layers add-layer ../OSES-Assignment
    
# 2: Check Layer has been added (optional if you have done #1)
    cd conf
    cat bblayers.conf
    --check that after the field 'BBLAYERS ?= "' you have something like 'poky/OSES-Assignment \'
    --if not then write it manually
    
# 3: Add to the image the driver+user application
    --add at the end of 'local.conf' the following lines:
    IMAGE_INSTALL_append = "driver"
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
    --create the associated device file named VIRTUAL_HART
    mknod /dev/VIRTUAL_HART c 251 0
    --now the application can comunicate to the device file, start the application:
    userapp
