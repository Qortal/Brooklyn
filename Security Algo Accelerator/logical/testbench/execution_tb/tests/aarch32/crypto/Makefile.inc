#-----------------------------------------------------------------------------
# The confidential and proprietary information contained in this file may
# only be used by a person authorised under and to the extent permitted
# by a subsisting licensing agreement from ARM Limited or its affiliates.
#
#            (C) COPYRIGHT 2013-2020 ARM Limited or its affiliates.
#                ALL RIGHTS RESERVED
#
# This entire notice must be reproduced on all copies of this file
# and copies of this file may only be made by a person if such person is
# permitted to do so under the terms of a subsisting license agreement
# from ARM Limited or its affiliates.
#
#            Release Information : HERCULESAE-MP106-r0p1-00eac0
#
#-----------------------------------------------------------------------------
# Makefile include file for AArch32 crypto. This must be included from the
# top-level Makefile; it is not a standalone Makefile.
#-----------------------------------------------------------------------------

# Note these variables must only be used in places where Make reads their
# immediate values rather than their deferred values.  This is because all
# the include files use the same variables and the deferred evaluation will
# yeild the last values set by the last include file.  They can be used in the
# target and prerequisite sections of rule definitions, which are evaluated
# immediately, but not in the recipe, where evaluation is deferred.
srcdir        := aarch32/crypto
common_srcdir := common/crypto
libdir        := common/shared
dstdir        := aarch32/crypto
target        := $(dstdir)/crypto.elf
csrcs         := $(wildcard $(common_srcdir)/*.c)
asmsrcs       := $(wildcard $(srcdir)/*.s)
libsrcs       := $(wildcard $(libdir)/*.c)
cobjs         := $(patsubst $(common_srcdir)/%.c,$(dstdir)/%.o,$(csrcs)) \
                 $(patsubst $(libdir)/%.c,$(dstdir)/%.o,$(libsrcs))
asmobjs       := $(patsubst %.s,%.o,$(asmsrcs))

# Find common C files (the source files are not in the build target directory)
vpath %.c $(common_srcdir) $(libdir)

# Change the CPU target to include crypto for all files that need compiling
$(asmobjs): ARCH = armv8-a+crypto

$(asmobjs): %.o: %.s
	@echo " [ASM ] $<"
	@$(ASM32) $(ASM_OPTS_AARCH32) $< -o $@

$(cobjs): $(dstdir)/%.o: %.c
	@echo " [CC $(CC32) ] $<"
ifeq ($(strip $(GCC)), yes)
	@$(CC32) $(subst -funroll-loops ,,$(CC_OPTS_AARCH32)) -O3 -mword-relocations -fno-inline-functions -fno-inline  $(foreach inc,$(^D),-I$(inc)) -I$(common_shared) $< -o $@
else
	@$(CC32) -mfpu=none $(subst -funroll-loops ,,$(CC_OPTS_AARCH32)) -O3 -fno-inline-functions -fno-inline  $(foreach inc,$(^D),-I$(inc)) -I$(common_shared) $< -o $@
endif

# Link.  For C-based tests this is done through GCC to make sure that all
# standard libraries are set up correctly.
$(target): $(asmobjs) $(cobjs) $(aarch32_bootobj) $(aarch32_c_bootobj)
	@echo " [LINK] $<"
ifeq ($(strip $(GCC)), yes)
	@$(CC32) $(LINK_OPTS_CSRC_AARCH32) $^ -o $@
else
	@$(LD32) $(LINK_OPTS_CSRC_AARCH32) $^ -o $@
endif

# ex: syntax=make:
