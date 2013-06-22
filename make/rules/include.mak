#-------------------------------------------------------------------------------
# Includes

ALL_INCLUDES:=

define emit_include_rule
$1_src_includes:=$(foreach file,$(wildcard $(call expand_tree,src/$1,.h)),$(file:src/%=include/%))
$1_obj_includes:=$(foreach file,$(wildcard $(call expand_tree,obj/$1,.h)),$(file:obj/%=include/%))
ALL_INCLUDES+=$1-include
$1-include: $$($1_src_includes) $$($1_obj_includes)
endef

define make_header
	@$(ECHO) '/*------------------------------------------------------------------------------' > $@
	@$(ECHO) ' * '$(@:include/%=%) >> $@
	@$(ECHO) ' *------------------------------------------------------------------------------' >> $@
	@$(CAT) LICENSE | sed -e's!^! * !' >> $@
	@$(ECHO) ' *----------------------------------------------------------------------------*/' >> $@
	@$(CAT) $< >> $@
endef

include/%.h: src/%.h
	@$(ECHO) :: $@
	@$(MKDIR) $(dir $@)
	$(call make_header)

include/%.h: obj/%.h
	@$(ECHO) :: $@
	@$(MKDIR) $(dir $@)
	$(call make_header)


# general rule to copy folders from src/$1 and obj/$1 into include/$1 - can be
# added to target.mak files. $(call make_include,??)
make_include=$(eval $(call emit_include_rule,$1))

