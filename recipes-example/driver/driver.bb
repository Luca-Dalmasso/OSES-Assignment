DESCRIPTION = "character device kernel module for virtual cardio"
LICENSE = "MIT"
LIC_FILES_CHKSUM ="file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit module
COMPATIBLE_MACHINE = "qemuarm"

SRC_URI = "file://Makefile \
	   file://driver.c \
	   file://data.h \
	  "

S = "${WORKDIR}"
