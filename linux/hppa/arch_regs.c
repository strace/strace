static unsigned long hppa_r28;

#define PT_GR20 (20*4)
#define PT_GR26 (26*4)
#define PT_GR28 (28*4)
#define PT_GR30 (30*4)
#define PT_IAOQ0 (106*4)
#define PT_IAOQ1 (107*4)

#define ARCH_PC_PEEK_ADDR PT_IAOQ0
#define ARCH_SP_PEEK_ADDR PT_GR30
