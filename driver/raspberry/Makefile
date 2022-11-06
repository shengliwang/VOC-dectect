KERNEL_DIR := /lib/modules/`uname -r`/build

MOD_NAME := sensor

obj-m += ${MOD_NAME}.o
${MOD_NAME}-y := main.o
${MOD_NAME}-y += i2c_master.o
${MOD_NAME}-y += sgp30_driver.o

all:
	make -C user_space/
	make -j4 -C  ${KERNEL_DIR} M=`pwd` modules

clean:
	make -C user_space/ clean
	make -C  ${KERNEL_DIR} M=`pwd` clean

.PHONY = clean all
