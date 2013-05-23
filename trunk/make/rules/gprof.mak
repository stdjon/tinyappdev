#-------------------------------------------------------------------------------
# Targets for helping to pass the right exe to gprof after a profile run.
# This will create a "gprof.out" file which contains the results of the profile
# run.

gprof:
	$(MAKE) PROFILE=1 gprof-run GPROF_=blob


%gprof:
	$(MAKE) PROFILE=1 gprof-run GPROF_=$(@:%gprof=%)


# NB: note we don't expand the target's name (i.e GPROF_) until as late as possible,
#   as we want to be sure that PROFILE=1 is in effect before the expansion takes
#   place, otherwise we will profile a binary from the wrong configuration!
profile_target=$(call bin_target,$(GPROF_))

gprof-run: $(profile_target)
	@echo Profiling \'$(profile_target)\'
	@$(profile_target)
	@echo Generating \'gprof.out\'
	@gprof $(profile_target) > gprof.out


