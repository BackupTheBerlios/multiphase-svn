# Makefile for 1-phase flow simulator.
# Copyright (C) 2005 Alexey Dobriyan <adobriyan@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

PROGRAM = 1phase
OBJECTS = 1phase.o misc.o parser.o
TEX = 1phase.tex

CFLAGS = -Wall

all: $(PROGRAM)

1phase: $(OBJECTS)

1phase.o: misc.h

misc.o: misc.h

parser.o: misc.h parser.h

dvi: $(TEX)
	for file in $(TEX); do latex $$file; done

clean:
	rm -f $(PROGRAM) $(OBJECTS) $(subst .tex,.dvi,$(TEX)) \
	$(subst .tex,.aux,$(TEX)) $(subst .tex,.log,$(TEX))
