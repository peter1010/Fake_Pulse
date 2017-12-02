###
# Copyright (c) 2017 Peter Leese
#
# Licensed under the GPL License. See LICENSE file in the project root for full license information.  
##

CFLAGS=-fpic -Wall -O3 -Wextra
CXXFLAGS=$(CFLAGS)

CPPFLAGS= -I$(SRCDIR)/../common -I/usr/include/pulse -DDEBUG

RM=rm -f
CC=gcc
CXX=g++
MAKEDEPEND=gcc -M $(CPPFLAGS)
LD=gcc
#-lstdc++

OBJS= shim.o pulse.o logging.o

.PHONY: all
all: fake_pulse.so


fake_pulse.so : $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@ -ldl -shared

#fake_pulse.so : $(OBJS) $(SRCDIR)/exportmap
#	$(LD) $(LDFLAGS) $(OBJS) -o $@ -shared -ldl -Wl,--version-script=$(SRCDIR)/exportmap

%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -MMD -o $@ $<
	@cp $*.d $*.P
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P
	@$(RM) $*.d
	@mv $*.P $*.d

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -MMD -o $@ $<
	@cp $*.d $*.P
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P
	@$(RM) $*.d
	@mv $*.P $*.d

-include $(OBJS:.o=.d) 
