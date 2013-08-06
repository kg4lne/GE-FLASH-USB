
# Project name
PROJECT = GEFlashUSB

# CPU architecture : {avr0|...|avr6}
# Parts : {at90usb646|at90usb647|at90usb1286|at90usb1287|at90usb162|at90usb82}
MCU = at90usb1286

# Source files
CSRCS = \
  main.c\
  scheduler.c\
  storage_task.c\
  usb_descriptors.c\
  usb_specific_request.c\
  usb_device_task.c\
  usb_standard_request.c\
  usb_task.c\
  lib_mcu/wdt/wdt_drv.c\
  lib_mcu/usb/usb_drv.c\
  lib_mcu/power/power_drv.c\

# Assembler source files
ASSRCS = \

