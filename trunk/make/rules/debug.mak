# TODO: remove project specific names, settings out of here!


# Launch the game from GDB debug the shell.
debug: all
	@$(ECHO) Debugging \'$(call bin_target,blob)\'
	$(call gdb_launch,$(blob_command))

# debug a test suite
%debug: all
	@$(ECHO) Debugging \'$(call bin_target,$(@:%debug=%))\'
	$(GDB) -q --args $(call bin_target,$(@:%debug=%)) -F _debuglevels $(D) -v $(V) $(TEST_FOCUS) $(TEST_ALLOCV)


#-------------------------------------------------------------------------------
# Local variables

# wrapper around gdb
gdb_launch=$(GDB) -q --args $(1)


