ifneq ($(KERNELRELEASE),)
obj-m := driver_kill_process.o
else
KDIR := /home/$(shell whoami)/linux-source-4.15.0/linux-source-4.15.0
all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif

clean: rm *.o