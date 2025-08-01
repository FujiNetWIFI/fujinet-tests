###################################################################
# fuji-call
###################################################################
ifeq ($(DEBUG),true)
    $(info >Starting fuji-call.mk)
endif

PLATFORM=$(CURRENT_TARGET)
FUJI_CALL = $(CACHE_DIR)/fuji-call
FUJI_CALL_PATH = $(FUJI_CALL)/build/fujinet.$(PLATFORM).lib

FUJI_CALL_REPO=https://github.com/FozzTexx/fujinet-lib-unified.git

.get_fuji_call:
	if [ ! -f "$(FUJI_CALL_PATH)" ]; then \
		if [ ! -d "$(FUJI_CALL)" ]; then \
			mkdir -p "$(CACHE_DIR)" ; \
			pushd "$(CACHE_DIR)" ; \
			git clone "$(FUJI_CALL_REPO)" "$$(basename $(FUJI_CALL))" ; \
			popd ; \
		fi; \
		make -k -C "$(FUJI_CALL)" build/fujinet.apple2.lib ; \
	fi

CFLAGS += --include-dir $(FUJI_CALL)/include
ASFLAGS += --asm-include-dir $(FUJI_CALL_VERSION_DIR)
LIBS += $(FUJI_CALL_PATH)
ALL_TASKS += .get_fuji_call
