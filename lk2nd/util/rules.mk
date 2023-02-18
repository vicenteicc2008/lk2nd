# SPDX-License-Identifier: BSD-3-Clause
LOCAL_DIR := $(GET_LOCAL_DIR)

OBJS += \
	$(LOCAL_DIR)/cmdline.o \
	$(LOCAL_DIR)/lkfdt.o \
	$(LOCAL_DIR)/mmu.o \

# Generate the version tag from VCS

LK2ND_VERSION := $(shell $(LK_TOP_DIR)/lk2nd/scripts/describe-version.sh)
VERSION_FILE := $(BUILDDIR)/$(LOCAL_DIR)/version.c

.PHONY: $(VERSION_FILE)
$(VERSION_FILE):
	@$(MKDIR)
	@echo generating $@ for lk2nd $(LK2ND_VERSION)
	@echo "const char* LK2ND_VERSION = \"$(LK2ND_VERSION)\";" > $@

OBJS += \
	$(LOCAL_DIR)/version.o \
