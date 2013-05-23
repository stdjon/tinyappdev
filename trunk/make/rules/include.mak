#-------------------------------------------------------------------------------
# Includes

# Calculate the set of include files to be copied to the include/ folder
# during the build

PML_SRC_INCLUDES:=$(wildcard $(call expand_tree,src/pml,.h))
PML_INCLUDES:=$(foreach file,$(PML_SRC_INCLUDES),$(file:src/%=include/%))

pml-include: $(PML_INCLUDES)

include: pml-include


define make_header
	@echo '/*------------------------------------------------------------------------------' > $@
	@echo ' * '$(@:include/%=%) >> $@
	@echo ' *------------------------------------------------------------------------------' >> $@
	@cat LICENSE | sed -e's!^! * !' >> $@
	@echo ' *----------------------------------------------------------------------------*/' >> $@
	@cat $< >> $@
endef

include/%.h: src/%.h make/rules/include.mak
	@echo :: $@
	@mkdir -p $(dir $@)
	$(call make_header)

include/%.h: obj/%.h make/rules/include.mak
	@echo :: $@
	@mkdir -p $(dir $@)
	$(call make_header)



