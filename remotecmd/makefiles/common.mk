###################################################################
# Common across Platforms
###################################################################
ifeq ($(DEBUG),true)
    $(info >>>Starting common.mk)
endif

-include ./makefiles/fuji-call.mk

VERSION_FILE := src/version.txt
ifeq (,$(wildcard $(VERSION_FILE)))
	VERSION_FILE =
	VERSION_STRING =
else
	VERSION_STRING := $(file < $(VERSION_FILE))
endif

CFLAGS += -Osir
