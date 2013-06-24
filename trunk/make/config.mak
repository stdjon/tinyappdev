#===============================================================================
# BUILD CONFIGURATION

# TODO: There is quite an amount of platform-specific configuration happening
#   in here - it might be better to break that out into individual .mak files
#   (one per platform...)
# TODO: This file might benefit from some more general refactoring as well...

# turn on callstack support on Linux/Windows (other platforms don't have implementations
# yet...)
ifneq ($(PLATFORM),MACOSX)
CALLSTACK:=1
endif


#-------------------------------------------------------------------------------
# INCLUDE PATHS FOR THIRD-PARTY LIBS

#TODO: Target-specific includes would be nice, in addition to platform-specific.

DEFAULT_USER_INCLUDES:= \
	# DEFAULT_USER_INCLUDES


LINUX_USER_INCLUDES:= \
	/usr/X11R6/include \
	 # LINUX_USER_INCLUDES


MACOSX_USER_INCLUDES:= \
	/Developer/Headers/FlatCarbon \
	/usr/X11R6/include \
	# MACOSX_USER_INCLUDES


WIN32_USER_INCLUDES:= \
	sr3/win32-dx5/include \
	 # WIN32_USER_INCLUDES


USER_INCLUDES:=\
	$(DEFAULT_USER_INCLUDES) \
	$($(PLATFORM)_USER_INCLUDES) \
	# USER_INCLUDES


#-------------------------------------------------------------------------------
# $(check-flag) can be used to normalize input flags from the commandline or from
# userconf.mak. It evaluates to true if the $1 is neither literal '0' (zero) nor
# an empty string. More precisely, it expands to an empty string if the input is
# '0' or empty, evaluates to '1' otherwise. This is so it will work with $(if),
# and can also be used with ifeq by testing the result against with '1' or '':
#
#     $(if $(call check-flag,$(X)),X-is-true,X-is-false)
#
#     ifeq ($(call check-flag,$(X)),1)
#     # X-is-true
#     endif
#
#     ifeq ($(call check-flag,$(X)),)
#     # X-is-false
#     endif

check-flag=$(if $(or \
	$(findstring X_!!_X,X_!$1!_X), \
	$(findstring X_!0!_X,X_!$1!_X)),,1)


#-------------------------------------------------------------------------------
# BUILD TYPE

# There are four build types, each with its own variable:
#
# SANITY=1 - variant debug built with additional sanity checking enabled
# DEBUG=1 - symbols, diagnostic trace, asserts, no optimization, no sanity checks
# PROFILE=1 - much like release, but with instrumentation for gprof
# RELEASE=1 - no symbols, no trace, no asserts/sanity checks, optimization
#
# (For historical reasons, explicitly setting DEBUG=0 as an argument to make
# will imply RELEASE=1.)
#
# Each flag is checked to see if it is empty or set to 0 (disabled) or set to 1
# or any other value (enabled). If none are enabled, debug is selected. If more
# than one are set, they may halt the build if they are incompatible.
# The $(check-flag) function can be called from the build to query these flags,
# results should be as follows:
#
# sanity: SANITY=1 DEBUG=1 PROFILE=0 RELEASE=0
# debug: SANITY=0 DEBUG=1 PROFILE=0 RELEASE=0
# profile: SANITY=0 DEBUG=0 PROFILE=1 RELEASE=1
# release: SANITY=0 DEBUG=0 PROFILE=0 RELEASE=1


# Default to debug if none of these flags are specified
ifeq ($(SANITY)$(DEBUG)$(PROFILE)$(RELEASE),)
DEBUG?=1
endif

# Build release if DEBUG is explicitly set as 0 (we have to pick either debug
# or release, or break the build...)
ifeq ($(DEBUG),0)
RELEASE?=1
endif


ifeq ($(call check-flag,$(SANITY)),1)
DEBUG=1
TYPE?=sanity
$(if $(call check-flag,$(PROFILE)),$(error SANITY=1 with PROFILE=1))
$(if $(call check-flag,$(RELEASE)),$(error SANITY=1 with RELEASE=1))
endif
ifeq ($(call check-flag,$(DEBUG)),1)
TYPE?=debug
$(if $(call check-flag,$(PROFILE)),$(error DEBUG=1 with PROFILE=1))
$(if $(call check-flag,$(RELEASE)),$(error DEBUG=1 with RELEASE=1))
endif
ifeq ($(call check-flag,$(PROFILE)),1)
RELEASE=1
TYPE?=profile
$(if $(call check-flag,$(SANITY)),$(error PROFILE=1 with SANITY=1))
$(if $(call check-flag,$(DEBUG)),$(error PROFILE=1 with DEBUG=1))
endif
ifeq ($(call check-flag,$(RELEASE)),1)
TYPE?=release
$(if $(call check-flag,$(SANITY)),$(error RELEASE=1 with SANITY=1))
$(if $(call check-flag,$(DEBUG)),$(error RELEASE=1 with DEBUG=1))
endif


#-------------------------------------------------------------------------------
# COMPILER FLAGS CONFIGURATION

# (TODO: move elsewhere?)
# NB: These flags are all utterly gcc-specific.
#     Any attempt to change compiler will need to address this.

#-------------------------------------------------------------------------------
# Hack/experimental per-file compile flags

# TODO: I'd like a more general file-specific/target-specific CFLAGS mechanism,
#   which would allow us to achieve these effects in a less ad-hoc manner (and
#   would probably yield other benefits as well...)

# SDL debug build occasionally needs -O1, as it seems modern versions of gcc
# (eg 4.6) produce errors when trying to compile SDL_blit*.c with no optimization
# flag enabled.

# expand to -O1 if source filename contains 'SDL_blit', and DEBUG=1
define sdl_blit_debug_optimize_hack
$(if $(and $(findstring SDL_blit,$<),$(call check-flag,$(DEBUG))),-O1)
endef

# I want -Werror enabled for as much code as possible in the src/ tree, but not
# for sr3/ tree (as I don't want to have to spend days fixing warnings in other
# people's code if they don't think that it's worth fixing the warnings in the
# first place) This is slightly complicated by code in src/ which includes
# headers from sr3/ which, in practice, is anything in src/util, and any of the
# implementation files (*_impl.c*)

# expand to -Werror if 'sr3/' and 'src/util' and '_impl.c' is *not* in the source
# filename (hence double , before -Werror, the $(if)'s 'then' clause is empty...)
define werror_on_for_src_tree_hack
$(if $(or $(findstring sr3/,$<),$(findstring src/util,$<),$(findstring _impl.c,$<)),,-Werror)
endef


#-------------------------------------------------------------------------------
# Warning supression (-Wno-* flags)

# Turn on all warnings (-Wall -Wextra) then selectively disable:
# * warnings about ignored 'const' on return values (who cares?)
# * warnings about unused parameters to functions (super annoying)
# * warnings about missing field initializers (breaks {0} for struct init)

WARNINGS_CFLAGS:=-Wall -Wextra \
	-Wno-ignored-qualifiers \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	# WARNINGS_CFLAGS


#-------------------------------------------------------------------------------
# Per-build type compile/link flags
# -gdwarf-2 and -g3 flags together enable gcc to provide debug information about
# macro definitions. (see gdb 'macro expand' command for more info...)

LINUX_SYMBOL_CFLAGS:=-gdwarf-2
SYMBOL_CFLAGS:=$($(PLATFORM)_SYMBOL_CFLAGS) -g3

# debug flags
debug_CFLAGS:=$(SYMBOL_CFLAGS) -D_DEBUG -DDEBUG_S -DTRACE_S
debug_LFLAGS:=

# sanity flags (copy the debug ones...)
# As the debug flags could get modified later in the config, we'll use = rather
# than := to delay the expansion of sanity flags until the last moment.
# TODO: clean this up (i.e. this config file)
sanity_CFLAGS=-DSANITY_S $(debug_CFLAGS)
sanity_LFLAGS=$(debug_LFLAGS)

# profile flags
profile_CFLAGS:=-O3 -pg -DNDEBUG -DPROFILE_S
profile_LFLAGS:=-pg

# release flags
release_CFLAGS:=-O3 -DNDEBUG -DRELEASE_S
release_LFLAGS:=


# If you want symbols in a profile/release build, use SYMS=1 (you will need to
# clean/rebuild for this to take effect). Can be handy for release-only crashes
# etc...)
ifeq ($(call check-flag,$(SYMS)),1)
$(TYPE)_CFLAGS+=$(SYMBOL_CFLAGS)
endif

# Callstack support pretty much only works on Windows and Linux at the moment.
# (-rdynamic is not recognized on OSX). Ideally, every platform needs its own
# code for capturing callstacks and resolving them to function names.
# (See the src/misc/ folder for platform implementation details...)

ifeq ($(call check-flag,$(CALLSTACK)),1)

# FIXME: why the hell are we modifying the debug flags only?

# needed for callstack support
ifeq ($(PLATFORM),LINUX)
debug_CFLAGS+=-DMEM_CALLSTACK_S
debug_LFLAGS+=-rdynamic
endif

ifeq ($(PLATFORM),WIN32)
debug_CFLAGS+=-gstabs -DMEM_CALLSTACK_S
debug_LFLAGS+=-lbfd -liberty -limagehlp #extra libs #-lintl
endif

endif


#-------------------------------------------------------------------------------
# Per-platform compile flags

BSD_CFLAGS:=

LINUX_CFLAGS:= \
	-D_GNU_SOURCE=1 \
	-DXTHREADS \
	-D_REENTRANT \
	-DHAVE_LINUX_VERSION_H
	# LINUX_CFLAGS

ifeq ($(call check-flag,$(CALLSTACK)),)
# this flag should give smaller code/faster link times, but does not play nicely
# with callstacks
LINUX_CFLAGS+=-fvisibility=hidden
endif

MACOSX_CFLAGS = \
	-D_GNU_SOURCE=1 \
	-DTARGET_API_MAC_CARBON \
	-DTARGET_API_MAC_OSX \
	-DXTHREADS \
	-D_THREAD_SAFE \
	-DSDL_TIMER_MACOS \
	-DSDL_CDROM_MACOSX \
	-DSDL_JOYSTICK_IOKIT \
	-DSDL_LOADSO_DLOPEN \
	# MACOSX_CFLAGS

#undef __STRICT_ANSI__ for MinGW
WIN32_CFLAGS:= \
	-DHAVE_RINT \
	-DUSE_STRLWR=0 \
	-DWINVER=0x0501 \
	-D_WIN32_WINNT=0x0501 \
	-U__STRICT_ANSI__ \
	# WIN32_CFLAGS

DEFAULT_CFLAGS:= \
	-Isrc \
	-Isr3 \
	-Iobj \
	-D$(PLATFORM)_S=1 \
	-DTFR_USE_MINICONF_S \
	# DEFAULT_CFLAGS

# NB: CFLAGS is now using = instead of := to allow the experimental
#   in debug configs. (See sdl_blit_debug_optimize_hack above)
CFLAGS= \
	$(call sdl_blit_debug_optimize_hack) \
	$(call werror_on_for_src_tree_hack) \
	$(DEFAULT_CFLAGS) \
	$(WARNINGS_CFLAGS) \
	$($(PLATFORM)_CFLAGS) \
	$($(TYPE)_CFLAGS) \
	$(USER_CFLAGS) \
	# CFLAGS

C99FLAGS=$(CFLAGS) -std=c99

CPPFLAGS=$(CFLAGS) -std=c++98

#-------------------------------------------------------------------------------
# Per-platform link flagS

# these are a bit of a hack too :(

BSD_LFLAGS:=

LINUX_LFLAGS:=

MACOSX_LFLAGS:=

WIN32_LFLAGS:=

LFLAGS:= \
	$($(TYPE)_LFLAGS) \
	$($(PLATFORM)_LFLAGS) \
	# LFLAGS


#-------------------------------------------------------------------------------
# PLATFORM PREFIXES/SUFFIXES

# NB: this build system originated on windows, and breaks horribly if binary
# files don't have an extension. So all the *nix platforms use .out in honour of
# a.out. (The extension can be stripped at the installation stage when the binary
# gets copied, so it's not the end of the world, but a bit annoying nonetheless).

WIN32_BINPRE:=
WIN32_BINEXT:=.exe
WIN32_DLLPRE:=
WIN32_DLLEXT:=.dll

BSD_BINPRE:=
BSD_BINEXT:=.out
BSD_DLLPRE:=
BSD_DLLEXT:=.so

LINUX_BINPRE:=
LINUX_BINEXT:=.out
LINUX_DLLPRE:=
LINUX_DLLEXT:=.so

MACOSX_BINPRE:=
MACOSX_BINEXT:=.out
MACOSX_DLLPRE:=
MACOSX_DLLEXT:=.dylib


BINPRE:=$($(PLATFORM)_BINPRE)
BINEXT:=$($(PLATFORM)_BINEXT)

DLLPRE:=$($(PLATFORM)_DLLPRE)
DLLEXT:=$($(PLATFORM)_DLLEXT)

LIBPRE:=lib
LIBEXT:=.a


#-------------------------------------------------------------------------------
# UTILITY FUNCTIONS

bin_target=bin/$(TYPE)/$(BINPRE)$(1)$(BINEXT)
dll_target=bin/$(TYPE)/$(DLLPRE)$(1)$(DLLEXT)
lib_target=lib/$(TYPE)/$(LIBPRE)$(1)$(LIBEXT)
pdf_target=doc/$(1).pdf


#-------------------------------------------------------------------------------
# TOOL DEFAULTS

# Creating and deleting directories - 
# we want to be able to create e.g. a/b/c, and have any/all directories in the
# path created at once.
MKDIR?=mkdir -p

# We want to be able to delete a directory and any contents it may have
# unconditionally.
RMDIR?=rm -rf

# Specify default gcc/g++ for C/C++ compilation/linking, but this should make it
# easier for someone to use a custom replacement tool (e.g. gcc-compatible
# compiler, or a custom built version...).
GCC?=gcc
G++?=g++
AR?=ar

# GDB(-compatible) debugger/profiler
GDB?=gdb
GPROF?=gprof

# Used for fw docs build. v1.8 gives better looking docs, 1.7 will still work
# though, at a pinch.
DOXYGEN?=doxygen

# misc tools (see misc.mak, etc)
CAT?=cat
CP?=cp
ECHO?=echo
GREP?=grep
MV?=mv
SED?=sed
WC?=wc

