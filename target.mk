###
# Copyright (c) 2017 Peter Leese
#
# Licensed under the GPL License. See LICENSE file in the project root for full license information.  
##

#
# Thanks goes to http://make.paulandlesley.org for the ideas and concepts that 
# I've borrowed in these makefiles
# 

.SUFFIXES:

# Calculate name of build directory
ifndef _ARCH
    _ARCH := $(shell uname -m)
    export _ARCH
endif
OBJDIR := __$(_ARCH)__

# Switch to OBJ DIR and reinvoke make
# The '+' means do it even if make -n is used
# The '@' means hide output
# MAKECMDGOALS passes the make goals to sub make
# Use relative path for SRCDIR instead of absolute so __FILE__ doesnt contain fullpath
.PHONY: $(OBJDIR)
$(OBJDIR):
	+ test -d $@ || mkdir -p $@
	+ $(MAKE) -C $@ -f $(CURDIR)/Makefile SRCDIR=.. $(MAKECMDGOALS)


# Do nothing rules for making the make files
Makefile: ;
%.mk:: ;

# Rule for making any target; it is dependent on $(OBJDIR) and runs the ':' command
# which does nothing
%:: $(OBJDIR)
	:

.PHONY: clean
clean:
	rm -rf $(OBJDIR)

