#-------------------------------------------------------------------------------
# Umbrella headers

# This function expands into the rule for a building a specific header for use
# with $(eval) - most of the work is done by the make_umbrella.sh script.
#
# This is a bit experimental: make a header file called "obj/$1" which includes
# all the header files in src/$2
#
# NB: I am of the (strong) opinion that autogenerated code lives in the obj/
#     tree, not the src/ tree. If we do ever need to publish headers, we can
#     synthesize include/ by copying headers out of both src/ and obj/
define emit_umbrella_rule
obj/$1: src/$2/*.h
	$(ECHO) [GEN] Generating \'obj/$1\'
	$(MKDIR) $(dir obj/$1)
	make/make_umbrella.sh obj/$1 $2/*.h $3 $4
endef

# general rule to make a header obj/a/b.h from src/a/b/*.h
# $1 the folder in src/* to scan, the output header filename obj/*.h
# $2 the header guard name (eg A_B_HEADER_H or similar)
make_umbrella=$(eval $(call emit_umbrella_rule,$1.h,$1,$2))


# more specific rule to make a header obj/a/b.h from src/a/b/*.h
# $1 the header's full filename (obj/ is prepended)
# $2 the src/ folder to scan for headers (not recursive)
# $3 the header guard name (eg A_B_HEADER_H or similar)
# $4 a list of explicitly excluded headers (e.g autogen headers in subdirectories).
make_umbrella_ex=$(eval $(call emit_umbrella_rule,$1,$2,$3,$4))


