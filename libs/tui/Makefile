# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

# tui = Text User Interface ( different from GUI ... )

default: all

TOP ?= $(abspath ../..)
MODULE = libs/tui

INT_LIBS = \
	libtui \
	libtui_cpp

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES =        \
	$(TUI_OBJ)    \
	$(TUI_CPP_OBJ)

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs compile
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS))

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean


#-------------------------------------------------------------------------------
# tui
#
$(ILIBDIR)/libtui: $(addprefix $(ILIBDIR)/libtui.,$(ILIBEXT))

TUI_SRC = \
	systui \
	screen \
	string_cache \
	eventring \
	tui \
	line_policy \
	tui_palette \
	tui_widget \
	tui_widget_label \
	tui_widget_button \
	tui_widget_tabhdr \
	tui_widget_checkbox \
	tui_widget_inputline \
	tui_widget_radiobox \
	tui_widget_string_list \
	tui_widget_progress \
	tui_widget_spin_edit \
	tui_widget_grid \
	tui_menu \
	tui_dlg_helper \
	tui_dlg \
	dir_dlg \
	file_dlg \
	tui_std_dlg

TUI_OBJ = \
	$(addsuffix .$(LOBX),$(TUI_SRC))

TUI_LIB = \
	-lklib \
	-lkfs \
	-ldl

$(ILIBDIR)/libtui.$(LIBX): $(TUI_OBJ)
	$(LD) --slib -o $@ $^ $(TUI_LIB)


#-------------------------------------------------------------------------------
# tui_cpp
#
$(ILIBDIR)/libtui_cpp: $(addprefix $(ILIBDIR)/libtui_cpp.,$(ILIBEXT))

TUI_CPP_SRC = \
    tui_cpp

TUI_CPP_OBJ = \
	$(addsuffix .$(LOBX),$(TUI_CPP_SRC))

TUI_CPP_LIB = \
	-lklib \
	-lkfs \
	-ltui \
	-ldl

$(ILIBDIR)/libtui_cpp.$(LIBX): $(TUI_CPP_OBJ)
	$(LD) --slib -o $@ $^ $(TUI_CPP_LIB)


compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/
