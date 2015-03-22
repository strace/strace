unsigned int currpers;

scno = tile_regs.regs[10];

#ifdef __tilepro__
currpers = 1;
#else
# ifndef PT_FLAGS_COMPAT
#  define PT_FLAGS_COMPAT 0x10000  /* from Linux 3.8 on */
# endif
if (tile_regs.flags & PT_FLAGS_COMPAT)
	currpers = 1;
else
	currpers = 0;
#endif

update_personality(tcp, currpers);
