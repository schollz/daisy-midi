
# Project Name
TARGET = daisy_midi

USE_DAISYSP_LGPL = 1

# APP_TYPE=BOOT_SRAM
# without BOOT_SRAM: make program
# with BOOT_SRAM: make program-boot && make program-dfu
# Sources
CPP_SOURCES = main.cpp

# C_SOURCES = audio.c

# Library Locations
LIBDAISY_DIR = libDaisy
DAISYSP_DIR = DaisySP
USE_FATFS = 1
LDFLAGS += -u _printf_float
XFADE_SAMPLES = 480

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

.venv:
	uv venv 
	uv pip install -r requirements.txt
