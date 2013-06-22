#-------------------------------------------------------------------------------
# MISCELLANEOUS/UTILITY TARGETS

# report word/line/character counts on the src directory
do_wc=$(WC) $(foreach pattern,$(2),`find $(1) -name "$(pattern)"`)


# Report on all source code (.mak is debatable...)
wc:
	$(call do_wc, src, *.c *.cpp *.h *.scm *.mak)


# Report on anything in src which doesn't end in 'base' (SVN internals) (or have
# fewer than 4 letters, presumably).
wc*:
	$(call do_wc, src, *[!b][!a][!s][!e])


#-------------------------------------------------------------------------------
# Show the value of make variables (for diagnostic purposes)
# eg: make show:BIN_TARGETS

show\:%:
	@$(ECHO)
	@$(ECHO) $(@:show:%=%)="$($(@:show:%=%))"
	@$(ECHO)


