# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
    $(info [i]: KERNELRELEASE is set == $(KERNELRELEASE))

    # A makefile symbol used by the kernel build system to determine which modules
    # should be built in the current directory
    obj-m := hello_world.o
    # If module is generated from two source files they can be added here
    module-objs := hello_world.o

    CFLAGS_hello_world.o := -DDEBUG

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
    $(info [i]: KERNELRELEASE is not set)
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    $(info [i]: KERNELDIR == $(KERNELDIR))
    PWD := $(shell pwd)
    $(info [i]: PWD == $(PWD))

.PHONY: modules
modules:
	$(info [i]: modules)
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

.PHONY: clean
clean:
	$(info [i]: clean)
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
