include $(MK)/config-default.mk

HEMELB_CFG_ON_OSX := true
HEMELB_CFG_ON_BSD := true

HEMELB_DEFS += HEMELB_CFG_ON_BSD HEMELB_CFG_ON_OSX

HEMELB_CXXFLAGS +=

#PMETIS_INCLUDE_DIR := $(PREFIX)/include
#PMETIS_LIBRARY_DIR := $(PREFIX)/lib

PMETIS_LIBRARY_DIR := $(TOP)/parmetis/build/Darwin-i386/libmetis/ $(TOP)/parmetis/build/Darwin-i386/libparmetis/
