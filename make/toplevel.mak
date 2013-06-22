#-------------------------------------------------------------------------------
# Top level makefile implementation details


# List of targets which clean the project. These have a reduced set of dependency
# information, among other differences.

CLEAN_TARGETS:= \
	clean \
	clean-dep \
	clean-obj \
	clean-src \
	clean-include \
	rinse \
	rinse-dep \
	rinse-obj \
	distclean \
	wc \
	wc* \
	tags \
# CLEAN_TARGETS


# List of targets for doc builds (TODO: ideally, we wouldn't need to specify
# these here, just in the target.mak files).

DOC_TARGETS:= \
	docs \
	clean-docs \
# DOC_TARGETS


#-------------------------------------------------------------------------------
# Build timing

# This pretty much has to go here, unfortunately.
# Use TIMED_BUILD=1 to see build times, _time=<command for time on your system>.

ifeq ($(call check-flag,$(TIMED_BUILD)),1)
ifeq ($(PLATFORM),WIN32)
# time.sh will call the 'time' indirectly command due to Windows/msys strangeness
_time?=sh make/time.sh
else
_time?=time -p
endif
_prefix:=$(_time)
endif


# Turn on the -s (silent) flag, unless the user has specifically requested to
# see that output with VERBOSE=1

ifeq ($(call check-flag,$(VERBOSE)),1)
_silent:=
else
_silent:=-s
endif


#-------------------------------------------------------------------------------
# Functions for invoking make recursively for building and cleaning

# We limit ourselves to a single recursion here, which is only so we can have
# greater control over the commandline which does the actual making of the
# targets. Default 'all' target will build anything specified with a target.mak
# file in the src/ (and sr3/) tree, and all dependency information will be
# visible across the entire build, even if you only request a specific target to
# be built.
# (If you've read 'Recursive Make Considered Harmful', then we aren't doing it the
# way that's considered harmful...)

# make something
# $1 - target to make (probably $@)
# $2 - optional commandline args
define do_make
	@$(ECHO) Making \'$1\'... && $(_prefix) $(MAKE) -r $(_silent) -f make/main.mak $1 $2
endef


# clean something
# $1 - target to make (probably $@)
define do_clean
	$(call do_make,$1,CLEAN=1)
endef


# Make a documenation target
# $1 - target to make (probably $@)
define do_docs
	$(call do_make,$1,DOCS=1)
endef


#-------------------------------------------------------------------------------
# Emit additional rules (clean/docs targets)

# Expand to a rule for a clean target (expand a rule which has $(do_clean) as
# the rule body.
# $1 - name of the clean target
define emit_clean_target
$1:
	$(call do_clean,$1)
endef


# Create rules for each of CLEAN_TARGETS above, using $(eval) to interpret the
# results as make syntax.
define emit_clean_targets
	$(foreach trg,$(CLEAN_TARGETS),$(eval $(call emit_clean_target,$(trg))))
endef


# Expand to a rule for a docs target (expand a rule which has $(do_docs) as
# the rule body.
# $1 - name of the docs target
define emit_docs_target
$1:
	$(call do_docs,$1)
endef


# Create rules for each of CLEAN_TARGETS above, using $(eval) to interpret the
# results as make syntax.
define emit_docs_targets
	$(foreach trg,$(DOCS_TARGETS),$(eval $(call emit_docs_target,$(trg))))
endef


#-------------------------------------------------------------------------------
# Targets

# we just need to mention 'all' here, so that it's the default target.
all:


# call emit_clean_targets, which will expand the rules for clean targets
$(emit_clean_targets)


# call emit_docs_targets, which will expand the rules for docs targets
$(emit_docs_targets)


# default build rule - used by anything which isn't listed above
%::
	$(call do_make,$@)
	@$(ECHO) Finished


# This rule "makes" the optional header userconf.mak, allowing the build to
# continue if it doesn't exist.
userconf.mak:;


# This is a list of commonly used targets for the benefit of commandline
# completion in environments which support it.

debug:
run:
gprof:
test:


