PRODUCT = lcltest
PLATFORMS += coco
PLATFORMS += apple2
PLATFORMS += atari
#PLATFORMS += c64
PLATFORMS += adam_cpm
PLATFORMS += msdos
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
  msxdos+=msx \
  adam_cpm+=adam

include makefiles/toplevel-rules.mk

# If you need to add extra platform-specific steps, do it below:
#   coco/r2r:: coco/custom-step1
#   coco/r2r:: coco/custom-step2
# or
#   apple2/disk: apple2/custom-step1 apple2/custom-step2

DISK_EXTRA_DEPS_COCO = src/coco/autoexec.bas

coco/disk-post::
	decb copy -t -0 src/coco/autoexec.bas "$(DISK),AUTOEXEC.BAS"

DISK_EXTRA_FILES = commands.jsn tests.tst
DISK_EXTRA_FILES_ATARI = DOS.SYS

CFLAGS_EXTRA_MSX = -DAMALLOC
LDFLAGS_EXTRA_MSX = -pragma-define:CRT_HEAP_AMALLOC=3

CFLAGS_EXTRA_ADAM_CPM = -DAMALLOC
LDFLAGS_EXTRA_ADAM_CPM = -pragma-define:CRT_HEAP_AMALLOC=3
