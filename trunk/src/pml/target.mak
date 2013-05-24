# miscellaneous common functionality (mainly debugging/diagnostic aids...)
LIB_TARGET:=pml

SOURCE:= \
	malloc.c \
	# SOURCE

# publish pml headers to include/
$(call make_include,pml)

