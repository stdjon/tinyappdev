BIN_TARGET:=test

SOURCE:= \
	main.cpp \
	pml/malloc.c \
	pml/malloc.cpp \
	# SOURCE

LIBS:= \
	pml \
	testframe \
	miniconf \
	misc \
	# LIBS


#-------------------------------------------------------------------------------
# Additional targets

# The testing targets use the following variables:
#
#     V - Verbosity level
#         The higher this is, the more output the tests will emit as they run.
#         This can be used as a debugging aid.
#
#     F - Focus suite
#         If this is specified, only the specified suite will be tested, rather
#         than everything. This is for rapid iteration on a particular test/suite.
#
#     D - General additional command line options
#         This can be used to pass additional commandlines to the test.out, if
#         necessary. Use quotes if you want to pass more than one option, or an
#         option takes an argument.
#
# These variables can be combined, as an example:
#
#     make test V=3 F=mytest D="-x blah -y blah"


# Run all tests on the current configuration (e.g. debug, release, etc)
test: all
	$(test_command)


# Run the test suite from GDB, for debugging
testdebug: all
	@echo Debugging \'$(call bin_target,test)\'
	$(call gdb_launch,$(test_command))


# Run the entire test suite in Sanity and Release configs.
# (Can be used to gain confidence in code while iterating)
test2:
	@$(MAKE) test SANITY=1
	@$(MAKE) test RELEASE=1
	@echo
	@echo \#\# SANITY/RELEASE OK \#\#
	@echo


# Run the entire test suite in every known configuration.
# NB: This is the one you want to run before checking anything in!
testall:
	@$(MAKE) test SANITY=1
	@$(MAKE) test DEBUG=1
	@$(MAKE) test RELEASE=1
ifneq ($(PLATFORM),BSD) # stupid link error w/ -pg flag on BSD :(
	@$(MAKE) test PROFILE=1
endif
	@echo
	@echo %% THIS IS ALL GOOD! %%
	@echo


#-------------------------------------------------------------------------------
# Local variables

F?=0
# This variable helps set the focus if F is set to a particular suite
ifneq ($(F),0)
test_focus:=-f "$(F)"
endif


# The actual command line for test.out
test_command:=$(call bin_target,test) $(D) $(verbosity) $(test_focus)

