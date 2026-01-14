PRODUCT = lcltest
PLATFORMS += coco
PLATFORMS += apple2
PLATFORMS += atari
PLATFORMS += c64
#PLATFORMS += adam
#PLATFORMS += msdos
PLATFORMS += msx

# You can run 'make <platform>' to build for a specific platform,
# or 'make <platform>/<target>' for a platform-specific target.
# Example shortcuts:
#   make coco        → build for coco
#   make apple2/disk → build the 'disk' target for apple2

# SRC_DIRS may use the literal %PLATFORM% token.
# It expands to the chosen PLATFORM plus any of its combos.
SRC_DIRS = src src/%PLATFORM%
INCLUDE_DIRS = include include/%PLATFORM%

CFLAGS = -DBUILD_$(PLATFORM_UC)

# FUJINET_LIB can be
# - a version number such as 4.7.6
# - a directory which contains the libs for each platform
# - a zip file with an archived fujinet-lib
# - a URL to a git repo
# - empty which will use whatever is the latest
# - undefined, no fujinet-lib will be used
FUJINET_LIB = https://github.com/FozzTexx/fujinet-lib-experimental.git

# Define extra dirs ("combos") that expand with a platform.
# Format: platform+=combo1,combo2
PLATFORM_COMBOS = \
  c64+=commodore \
  atarixe+=atari \
  msxrom+=msx \
  msxdos+=msx

include makefiles/toplevel-rules.mk

# If you need to add extra platform-specific steps, do it below:
#   coco/r2r:: coco/custom-step1
#   coco/r2r:: coco/custom-step2
# or
#   apple2/disk: apple2/custom-step1 apple2/custom-step2

# $1 == decb flags
# $2 == source file
# $3 == disk image
define coco-copy
  DEST="$$(basename $2 | tr '[:lower:]' '[:upper:]')" ; \
    decb copy $1 $2 $3,$${DEST}
endef

coco/disk-post:: commands.jsn tests.jsn
	$(call coco-copy,-a -3,commands.jsn,$(DISK))
	$(call coco-copy,-a -3,tests.jsn,$(DISK))

CFLAGS_EXTRA_MSX = -DAMALLOC
LDFLAGS_EXTRA_MSX = -pragma-define:CRT_HEAP_AMALLOC=3

# $1 == ac flags
# $2 == source file
# $3 == disk image
define apple2-copy
  DEST="$$(basename $2 | tr '[:lower:]' '[:upper:]')" ; \
    $(DISK_TOOL) $1 $3 $${DEST} < $2
endef

apple2/disk-post:: commands.jsn tests.jsn
	$(call apple2-copy,-ptx,commands.jsn,$(DISK))
	$(call apple2-copy,-ptx,tests.jsn,$(DISK))
