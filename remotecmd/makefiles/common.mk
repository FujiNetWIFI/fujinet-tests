###################################################################
# Common across Platforms
###################################################################
ifeq ($(DEBUG),true)
    $(info >>>Starting common.mk)
endif

VERSION_FILE := src/version.txt
ifeq (,$(wildcard $(VERSION_FILE)))
	VERSION_FILE =
	ERSION_STRING =
else
	VERSION_STRING := $(file < $(VERSION_FILE))
endif

FUJINET_LIB_DIR=$(HOME)/source/fujinet/fujinet-lib-unified
CFLAGS += -Osir -I$(FUJINET_LIB_DIR)/include
PLATFORM=$(CURRENT_TARGET)
LIBS += $(FUJINET_LIB_DIR)/build/fujinet.$(PLATFORM).lib
