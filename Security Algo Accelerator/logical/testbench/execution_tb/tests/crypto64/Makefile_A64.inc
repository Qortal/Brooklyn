###############################################################################
#    The confidential and proprietary information contained in this file may
#    only be used by a person authorised under and to the extent permitted
#    by a subsisting licensing agreement from ARM Limited.
#
#           (C) COPYRIGHT 2011-2013 ARM Limited.
#               ALL RIGHTS RESERVED
#
#    This entire notice must be reproduced on all copies of this file
#    and copies of this file may only be made by a person if such person is
#    permitted to do so under the terms of a subsisting license agreement
#    from ARM Limited.
#
###############################################################################
# Makefile.inc for crypto64
# setup source paths (crypto64)

crypto64_base =  crypto64
crypto64_src = $(crypto64_base)/src
crypto64_obj = $(crypto64_base)/obj
crypto64_elf = $(crypto64_base)/elf

#rules for crypto64

crypto64_asm_obj = $(incl_obj)/benchmark_boot_a64.o $(incl_obj)/vectors.o $(incl_obj)/num_cpus_a64.o $(crypto64_obj)/cryptolib_asm64.o
crypto64_c_obj = $(incl_obj)/sys_a64.o $(incl_obj)/stackheap_a64.o $(crypto64_obj)/cryptodata.o $(crypto64_obj)/crypto_test.o

crypto64: clean_crypto64 $(crypto64_elf)/crypto64.elf	

$(crypto64_obj)/%.o: $(crypto64_src)/%.c
		$(CC_A64) $(CC_A64_OPTS) $< -o $@

$(crypto64_obj)/%.o: $(crypto64_src)/%.s
		$(AS_A64) $(AS_A64_OPTS) $< -o $@

$(crypto64_elf)/crypto64.elf: $(crypto64_asm_obj) $(crypto64_c_obj) 
		$(LINK_A64) $(LINK_A64_OPTS) $(crypto64_asm_obj) $(crypto64_c_obj) -o $@

clean_crypto64:
		\rm -f $(crypto64_asm_obj) $(crypto64_c_obj) $(crypto64_elf)/crypto64.elf

