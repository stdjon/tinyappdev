# this is a wrapper around 'time' command which allows it to be run from MSYS.
# (It doesn't like being invoked directly from sh or make, but this little
# indirection seems to appease it...)

time $*
