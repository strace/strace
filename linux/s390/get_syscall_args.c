tcp->u_arg[0] = s390_regset.orig_gpr2;
tcp->u_arg[1] = s390_regset.gprs[3];
tcp->u_arg[2] = s390_regset.gprs[4];
tcp->u_arg[3] = s390_regset.gprs[5];
tcp->u_arg[4] = s390_regset.gprs[6];
tcp->u_arg[5] = s390_regset.gprs[7];
