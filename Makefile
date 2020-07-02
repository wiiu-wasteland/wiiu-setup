TARGET		:= wiiu-setup
WIIU_COMMON_DIR := common

BUILD_HBL_ELF	 = 1
BUILD_RPX	 = 0
DEBUG            = 0
LOGGER_IP        =
LOGGER_TCP_PORT	 =

OBJ :=	source/main.o \
	source/zip/zip.o

DEFINES  :=
INCDIRS  := -Isource
CFLAGS   :=

CXXFLAGS := -fno-rtti -fno-exceptions
LDFLAGS  :=
LIBS     :=

include $(WIIU_COMMON_DIR)/system/common.mk
