#-------------------------------------------------------------------------------
# MISCELLANEOUS/UTILITY TARGETS

# report word/line/character counts on the src directory
do_wc=wc $(foreach pattern,$(2),`find $(1) -name "$(pattern)"`)


# Report on all source code (.mak is debatable...)
wc:
	$(call do_wc, src, *.c *.cpp *.h *.scm *.mak)


# Report on anything in src which doesn't end in 'base' (SVN internals) (or have
# fewer than 4 letters, presumably).
wc*:
	$(call do_wc, src, *[!b][!a][!s][!e])


#-------------------------------------------------------------------------------
# tagfile (requires that ctags is installed)

tags:
	ctags `find . -name *.[ch]`
	cat $@ | grep -v \./include > $@~
	mv $@~ $@


#-------------------------------------------------------------------------------
# Diagnosis: print name=value of make variables
# eg: make diag D=BIN_TARGETS

diag:
	@echo
	@echo $(D)=$($(D))
	@echo



