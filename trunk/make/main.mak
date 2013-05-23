##-------------------------------------------------------------------------------
## include platform information which was written by configure, or fail noisily.

ifeq ($(call check-flag,$(CLEAN)),)
include dep/platform.mak
ifeq ($(PLATFORM),)

# This warning is printed by make if configure hasn't been run.

$(warning " *************************************************** ")
$(warning " *                                                 * ")
$(warning " *  Please run ./configure before trying to build  * ")
$(warning " *  (or configure.bat on Windows)                  * ")
$(warning " *                                                 * ")
$(error "************************************************ ")
endif
endif


#-------------------------------------------------------------------------------
# The user can use a userconf.mak file placed in the root folder (next to the
# makefile) to add customizations to the build system (assuming they know what
# they are doing...)
-include userconf.mak

# include general configuration options
include make/config.mak

# include build rules
include make/build.mak
