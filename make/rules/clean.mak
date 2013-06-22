#-------------------------------------------------------------------------------
# CLEANING

# TODO: BIN and LIB are not handled very well - dependencies and exe/lib files
# are not removed by the clean...

# Source directories.
# (Anything containing a target.mak.)

SOURCE_DIRS:=$(sort $(dir $(INPUT_TARGETS)))
3P_SOURCE_DIRS:=$(filter sr3/%, $(SOURCE_DIRS))
1P_SOURCE_DIRS:=$(filter src/%, $(SOURCE_DIRS))


# local functions for cleaning (provide directory lists for passing to $(RMDIR))

clean_src_dirs=$(call sourcesubst,obj/%,$(1))
clean_obj_dirs=$(call sourcesubst,obj/$(TYPE)/%,$(1))
clean_dep_dirs=$(call sourcesubst,dep/$(TYPE)/%,$(1))


# Clean deliberately doesn't clean object files from third-party source. They
# take a substantial amount of time to build, and change less often than the
# rest of the source. Use 'rinse' to do a full clean of all object files.
# (These are a bit imprecise, but work acceptably for the time being.)
# (Use 'distclean' to remove anything built at all, ever.)

clean: clean-src clean-include clean-dep clean-obj

rinse: clean-src clean-include rinse-dep rinse-obj


clean-dep:
	$(RMDIR) $(call clean_dep_dirs, $(1P_SOURCE_DIRS))

clean-obj:
	$(RMDIR) $(call clean_obj_dirs, $(1P_SOURCE_DIRS))

clean-src:
	$(RMDIR) $(call clean_src_dirs, $(1P_SOURCE_DIRS))

clean-include:
	$(RMDIR) include


rinse-dep:
	$(RMDIR) $(call clean_dep_dirs, $(SOURCE_DIRS))

rinse-obj:
	$(RMDIR) $(call clean_obj_dirs, $(SOURCE_DIRS))


distclean:
	$(RMDIR) bin dep doc lib obj html include


