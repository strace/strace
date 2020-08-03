/*
 * s390-specific syscalls decoders.
 *
 * Copyright (c) 2018-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#if defined S390 || defined S390X

# include <sys/user.h>

# include "print_fields.h"

# include "xlat/s390_guarded_storage_commands.h"
# include "xlat/s390_runtime_instr_commands.h"
# include "xlat/s390_sthyi_function_codes.h"

/*
 * Since, for some reason, kernel doesn't expose all these nice constants and
 * structures in UAPI, we have to re-declare them ourselves.
 */

/**
 * "The header section is placed at the beginning of the response buffer and
 * identifies the location and length of all other sections. Valid sections have
 * nonzero offset values in the header. Each section provides information about
 * validity of fields within that section."
 */
struct sthyi_hdr {
	/**
	 * Header Flag Byte 1 - These flag settings indicate the environment
	 * that the instruction was executed in and may influence the value of
	 * the validity bits. The validity bits, and not these flags, should be
	 * used to determine if a field is valid.
	 *  - 0x80 - Global Performance Data unavailable
	 *  - 0x40 - One or more hypervisor levels below this level does not
	 *           support the STHYI instruction. When this flag is set the
	 *           value of INFGPDU is not meaningful because the state of the
	 *           Global Performance Data setting cannot be determined.
	 *  - 0x20 - Virtualization stack is incomplete. This bit indicates one
	 *           of two cases:
	 *   - One or more hypervisor levels does not support the STHYI
	 *     instruction. For this case, INFSTHYI will also be set.
	 *   - There were more than three levels of guest/hypervisor information
	 *     to report.
	 *  - 0x10 - Execution environment is not within a logical partition.
	 */
	uint8_t  infhflg1;
	uint8_t  infhflg2; /**< Header Flag Byte 2 reserved for IBM(R) use */
	uint8_t  infhval1; /**< Header Validity Byte 1 reserved for IBM use */
	uint8_t  infhval2; /**< Header Validity Byte 2 reserved for IBM use */
	char     reserved_1__[3]; /**< Reserved for future IBM use */
	uint8_t  infhygct; /**< Count of Hypervisor and Guest Sections */
	uint16_t infhtotl; /**< Total length of response buffer */
	uint16_t infhdln;  /**< Length of Header Section mapped by INF0HDR */
	uint16_t infmoff;  /**< Offset to Machine Section mapped by INF0MAC */
	uint16_t infmlen;  /**< Length of Machine Section */
	uint16_t infpoff;  /**< Offset to Partition Section mapped by INF0PAR */
	uint16_t infplen;  /**< Length of Partition Section */
	uint16_t infhoff1; /**< Offset to Hypervisor Section1 mapped by INF0HYP */
	uint16_t infhlen1; /**< Length of Hypervisor Section1 */
	uint16_t infgoff1; /**< Offset to Guest Section1 mapped by INF0GST */
	uint16_t infglen1; /**< Length of Guest Section1 */
	uint16_t infhoff2; /**< Offset to Hypervisor Section2 mapped by INF0HYP */
	uint16_t infhlen2; /**< Length of Hypervisor Section2 */
	uint16_t infgoff2; /**< Offset to Guest Section2 mapped by INF0GST */
	uint16_t infglen2; /**< Length of Guest Section2 */
	uint16_t infhoff3; /**< Offset to Hypervisor Section3 mapped by INF0HYP */
	uint16_t infhlen3; /**< Length of Hypervisor Section3 */
	uint16_t infgoff3; /**< Offset to Guest Section3 mapped by INF0GST */
	uint16_t infglen3; /**< Length of Guest Section3 */
} ATTRIBUTE_PACKED;
static_assert(sizeof(struct sthyi_hdr) == 44,
	      "Unexpected struct sthyi_hdr size");

struct sthyi_machine {
	uint8_t  infmflg1; /**< Machine Flag Byte 1 reserved for IBM use */
	uint8_t  infmflg2; /**< Machine Flag Byte 2 reserved for IBM use */
	/**
	 * Machine Validity Byte 1.
	 *  - 0x80 - INFMPROC, Processor Count Validity. When this bit is on,
	 *           it indicates that INFMSCPS, INFMDCPS, INFMSIFL,
	 *           and INFMDIFL contain valid counts. The validity bit
	 *           may be off when:
	 *            - STHYI support is not available on a lower level
	 *              hypervisor, or
	 *            - Global Performance Data is not enabled.
	 *  - 0x40 - INFMMID, Machine ID Validity. This bit being on indicates
	 *           that a SYSIB 1.1.1 was obtained from STSI and information
	 *           reported in the following fields is valid: INFMTYPE,
	 *           INFMMANU, INFMSEQ, and INFMPMAN.
	 *  - 0x20 - INFMMNAM, Machine Name Validity. This bit being on
	 *           indicates that the INFMNAME field is valid.
	 *  - 0x10 - INFMPLNV, reserved for IBM use.
	 */
	uint8_t  infmval1;
	uint8_t  infmval2; /**< Machine Validity Byte 2 reserved for IBM use */
	/**
	 * Number of shared CPs configured in the machine or in the physical
	 * partition if the system is physically partitioned.
	 */
	uint16_t infmscps;
	/**
	 * Number of dedicated CPs configured in this machine or in the physical
	 * partition if the system is physically partitioned.
	 */
	uint16_t infmdcps;
	/**
	 * Number of shared IFLs configured in this machine or in the physical
	 * partition if the system is physically partitioned.
	 */
	uint16_t infmsifl;
	/**
	 * Number of dedicated IFLs configured in this machine or in the
	 * physical partition if the system is physically partitioned.
	 */
	uint16_t infmdifl;
	char     infmname[8];  /**< EBCDIC Machine Name */
	char     infmtype[4];  /**< EBCDIC Type */
	char     infmmanu[16]; /**< EBCDIC Manufacturer */
	char     infmseq[16];  /**< EBCDIC Sequence Code */
	char     infmpman[4];  /**< EBCDIC Plant of Manufacture */
	char     reserved_1__[4]; /**< Reserved for future IBM use */
	char     infmplnm[8];  /**< EBCDIC Reserved for IBM use */
} ATTRIBUTE_PACKED;
static_assert(sizeof(struct sthyi_machine) == 72,
	      "Unexpected struct sthyi_machine size");

struct sthyi_partition {
	/**
	 * Partition Flag Byte 1.
	 *  - 0x80 - INFPMTEN, multithreading (MT) is enabled.
	 *  - 0x40 - INFPPOOL, reserved for IBM use.
	 */
	uint8_t  infpflg1;
	/** Partition Flag Byte 2 reserved for IBM use */
	uint8_t  infpflg2;
	/**
	 * Partition Validity Byte 1.
	 *  - 0x80 - INFPPROC, Processor Count Validity. This bit being on
	 *           indicates that INFPSCPS, INFPDCPS, INFPSIFL, and INFPDIFL
	 *           contain valid counts.
	 *  - 0x40 - INFPWBCC, Partition Weight-Based Capped Capacity Validity.
	 *           This bit being on indicates that INFPWBCP and INFPWBIF
	 *           are valid.
	 *  - 0x20 - INFPACC, Partition Absolute Capped Capacity Validity.
	 *           This bit being on indicates that INFPABCP and INFPABIF
	 *           are valid.
	 *  - 0x10 - INFPPID, Partition ID Validity. This bit being on indicates
	 *           that a SYSIB 2.2.2 was obtained from STSI and information
	 *           reported in the following fields is valid: INFPPNUM
	 *           and INFPPNAM.
	 *  - 0x08 - INFPLGVL, LPAR Group Absolute Capacity Capping Information
	 *           Validity. This bit being on indicates that INFPLGNM,
	 *           INFPLGCP, and INFPLGIF are valid.
	 *  - 0x04 - INFPPLNV, reserved for IBM use.
	 */
	uint8_t  infpval1;
	/** Partition Validity Byte 2 reserved for IBM use */
	uint8_t  infpval2;
	/** Logical partition number */
	uint16_t infppnum;
	/**
	 * Number of shared logical CPs configured for this partition.  Count
	 * of cores when MT is enabled.
	 */
	uint16_t infpscps;
	/**
	 * Number of dedicated logical CPs configured for this partition.  Count
	 * of cores when MT is enabled.
	 */
	uint16_t infpdcps;
	/**
	 * Number of shared logical IFLs configured for this partition.  Count
	 * of cores when MT is enabled.
	 */
	uint16_t infpsifl;
	/**
	 * Number of dedicated logical IFLs configured for this partition.
	 * Count of cores when MT is enabled.
	 */
	uint16_t infpdifl;
	/** Reserved for future IBM use */
	char     reserved_1__[2];
	/** EBCIDIC Logical partition name */
	char     infppnam[8];
	/**
	 * Partition weight-based capped capacity for CPs, a scaled number where
	 * 0x00010000 represents one  core.  Zero if not capped.
	 */
	uint32_t infpwbcp;
	/**
	 * Partition absolute capped capacity for CPs, a scaled number where
	 * 0x00010000 represents one  core.  Zero if not capped.
	 */
	uint32_t infpabcp;
	/**
	 * Partition weight-based capped capacity for IFLs, a scaled number
	 * where 0x00010000 represents one  core.  Zero if not capped.
	 */
	uint32_t infpwbif;
	/**
	 * Partition absolute capped capacity for IFLs, a scaled number where
	 * 0x00010000 represents one  core.  Zero if not capped.
	 */
	uint32_t infpabif;
	/**
	 * EBCIDIC LPAR group name. Binary zeros when the partition is not in
	 * an LPAR group. EBCDIC and padded with blanks on the right when in a
	 * group. The group name is reported only when there is a group cap on
	 * CP or IFL CPU types and the partition has the capped CPU type.
	 */
	char     infplgnm[8];
	/**
	 * LPAR group absolute capacity value for CP CPU type when nonzero. This
	 * field will be nonzero only when INFPLGNM is nonzero and a cap is
	 * defined for the LPAR group for the CP CPU type. When nonzero,
	 * contains a scaled number where 0x00010000 represents one core.
	 */
	uint32_t infplgcp;
	/**
	 * LPAR group absolute capacity value for IFL CPU type when nonzero.
	 * This field will be nonzero only when INFPLGNM is nonzero and a cap
	 * is defined for the LPAR group for the IFL CPU type. When nonzero,
	 * contains a scaled number where 0x00010000 represents one core.
	 */
	uint32_t infplgif;
	char     infpplnm[8]; /**< Reserved for future IBM use. */
} ATTRIBUTE_PACKED;
static_assert(sizeof(struct sthyi_partition) == 64,
	      "Unexpected struct sthyi_partition size");

struct sthyi_hypervisor {
	/**
	 * Hypervisor Flag Byte 1
	 *  - 0x80 - INFYLMCN, guest CPU usage hard limiting is using
	 *           the consumption method.
	 *  - 0x40 - INFYLMPR, if on, LIMITHARD caps use prorated core time
	 *           for capping. If off, raw CPU time is used.
	 *  - 0x20 - INFYMTEN, hypervisor is MT-enabled.
	 */
	uint8_t infyflg1;
	uint8_t infyflg2; /**< Hypervisor Flag Byte 2 reserved for IBM use */
	uint8_t infyval1; /**< Hypervisor Validity Byte 1 reserved for IBM use */
	uint8_t infyval2; /**< Hypervisor Validity Byte 2 reserved for IBM use */
	/**
	 * Hypervisor Type
	 *  - 1 - z/VM is the hypervisor.
	 */
	uint8_t infytype;
	char    reserved_1__[1]; /**< Reserved for future IBM use */
	/**
	 * Threads in use per CP core. Only valid when MT enabled
	 * (INFPFLG1 0x80 is ON).
	 */
	uint8_t infycpt;
	/**
	 * Threads in use per IFL core. Only valid when MT enabled
	 * (INFPFLG1 0x80 is ON).
	 */
	uint8_t infyiflt;
	/**
	 * EBCID System Identifier. Left justified and padded with blanks.
	 * This field will be blanks if non-existent.
	 */
	char     infysyid[8];
	/**
	 * EBCID Cluster Name. Left justified and padded with blanks. This is
	 * the name on the SSI statement in the system configuration file. This
	 * field will be blanks if nonexistent.
	 */
	char     infyclnm[8];
	/**
	 * Total number of CPs shared among guests of this hypervisor.
	 * Number of cores when MT enabled.
	 */
	uint16_t infyscps;
	/**
	 * Total number of CPs dedicated to guests of this hypervisor.
	 * Number of cores when MT enabled.
	 */
	uint16_t infydcps;
	/**
	 * Total number of IFLs shared among guests of this hypervisor.
	 * Number of cores when MT enabled.
	 */
	uint16_t infysifl;
	/**
	 * Total number of IFLs dedicated to guests of this hypervisor.
	 * Number of cores when MT enabled.
	 */
	uint16_t infydifl;
	/**
	 * Mask of installed function codes. Bit position corresponding
	 * to the function code number is on if the function code is supported
	 * by this hypervisor. Bits may be on even if the guest
	 * is not authorized.
	 *
	 * Element 0 (INFYINS0) flags:
	 *  - 0x80 - INFYFCCP, FC = 0, Obtain CPU Capacity Info.
	 *  - 0x40 - INFYFHYP, FC = 1, Hypervisor Environment Info.
	 *  - 0x20 - INFYFGLS, FC = 2, Guest List.
	 *  - 0x10 - INFYFGST, FC = 3, Designated Guest Info.
	 *  - 0x08 - INFYFPLS, FC = 4, Resource Pool List.
	 *  - 0x04 - INFYFPDS, FC = 5, Designated Resource Pool Information.
	 *  - 0x02 - INFYFPML, FC = 6, Resource Pool Member List.
	 */
	uint8_t  infyinsf[8];
	/**
	 * Mask of authorized functions codes. Bit position corresponding
	 * to the function code number is on if the function code is supported
	 * by this hypervisor and the guest has been authorized
	 * in the directory.
	 *
	 * The flags are the same as in infyinsf.
	 */
	uint8_t  infyautf[8];
} ATTRIBUTE_PACKED;
static_assert(sizeof(struct sthyi_hypervisor) == 48,
	      "Unexpected struct sthyi_hypervisor size");

struct sthyi_guest {
	/**
	 * Guest Flag Byte 1
	 *  - 0x80 - Guest is mobility enabled
	 *  - 0x40 - Guest has multiple virtual CPU types
	 *  - 0x20 - Guest CP dispatch type has LIMITHARD cap
	 *  - 0x10 - Guest IFL dispatch type has LIMITHARD cap
	 *  - 0x08 - Virtual CPs are thread dispatched
	 *  - 0x04 - Virtual IFLs are thread dispatched
	 */
	uint8_t  infgflg1;
	uint8_t  infgflg2;    /**< Guest Flag Byte 2 reserved for IBM use */
	uint8_t  infgval1;    /**< Guest Validity Byte 1 reserved for IBM use */
	uint8_t  infgval2;    /**< Guest Validity Byte 2 reserved for IBM use */
	char     infgusid[8]; /**< EBCDIC Userid */
	uint16_t infgscps;    /**< Number of guest shared CPs */
	uint16_t infgdcps;    /**< Number of guest dedicated CPs */
	/**
	 * Dispatch type for guest CPs.  This field is valid if INFGSCPS or
	 * INFGDCPS is greater than zero.
	 *  - 0 - General Purpose (CP)
	 */
	uint8_t  infgcpdt;
	char     reserved_1__[3]; /**< Reserved for future IBM use */
	/**
	 * Guest current capped capacity for shared virtual CPs, a scaled number
	 * where 0x00010000 represents one  core.   This field is zero to
	 * indicate not capped when:
	 *  - There is no CP individual limit (that is, the "Guest CP dispatch
	 *    type has LIMITHARD cap" bit in field INFGFLG1 is OFF).
	 *  - There are no shared CPs on the system (that is, INFYSCPS = 0).
	 *    If there is a CP limit but there are no shared CPs or virtual CPs,
	 *    the limit is meaningless and does not apply to anything.
	 */
	uint32_t infgcpcc;
	uint16_t infgsifl; /**< Number of guest shared IFLs */
	uint16_t infgdifl; /**< Number of guest dedicated IFLs */
	/**
	 * Dispatch type for guest IFLs. This field is valid if INFGSIFL or
	 * INFGDIFL is greater than zero.
	 *  - 0 - General Purpose (CP)
	 *  - 3 - Integrated Facility for Linux (IFL)
	 */
	uint8_t  infgifdt;
	char     reserved_2__[3]; /**< Reserved for future IBM use */
	/**
	 * Guest current capped capacity for shared virtual IFLs,  a scaled
	 * number where 0x00010000 represents one core.   This field is zero
	 * to indicate not capped with an IFL limit when:
	 *  - There is no IFL individual limit (that is, the "Guest IFL dispatch
	 *    type has LIMITHARD cap" bit in field INFGFLG1 is OFF).
	 *  - The guest's IFLs are dispatched on CPs (that is, INFGIFDT = 00).
	 *    When the guest's IFLs are dispatched on CPs, the CP individual
	 *    limit (in INFGCPCC) is applied to the guest's virtual IFLs and
	 *    virtual CPs.
	 */
	uint32_t infgifcc;
	/**
	 * CPU Pool Capping Flags
	 *  - 0x80 - CPU Pool's CP virtual type has LIMITHARD cap
	 *  - 0x40 - CPU Pool's CP virtual type has CAPACITY cap
	 *  - 0x20 - CPU Pool's IFL virtual type has LIMITHARD cap
	 *  - 0x10 - CPU Pool's IFL virtual type has CAPACITY cap
	 *  - 0x08 - CPU Pool uses prorated core time.
	 */
	uint8_t  infgpflg;
	char     reserved_3__[3]; /**< Reserved for future IBM use */
	/**
	 * EBCDIC CPU Pool Name. This field will be blanks if the guest is not
	 * in a CPU Pool.
	 */
	char     infgpnam[8];
	/**
	 * CPU Pool capped capacity for shared virtual CPs, a scaled number
	 * where 0x00010000 represents one  core.  This field will be zero if
	 * not capped.
	 */
	uint32_t infgpccc;
	/**
	 * CPU Pool capped capacity for shared virtual IFLs, a scaled number
	 * where 0x00010000 represents one  core.  This field will be zero if
	 * not capped.
	 */
	uint32_t infgpicc;
} ATTRIBUTE_PACKED;
static_assert(sizeof(struct sthyi_guest) == 56,
	      "Unexpected struct sthyi_guest size");


static void
decode_ebcdic(const char *ebcdic, char *ascii, size_t size)
{
	/*
	 * This is mostly Linux's EBCDIC-ASCII conversion table, except for
	 * various non-representable characters that are converted to spaces for
	 * readability purposes, as it is intended to be a hint for the string
	 * contents and not precise conversion.
	 */
	static char conv_table[] =
		 "\0\1\2\3 \11 \177   \13\14\15\16\17"
		 "\20\21\22\23 \n\10 \30\31  \34\35\36\37"
		 "  \34  \n\27\33     \5\6\7"
		 "  \26    \4    \24\25 \32"
		 "          " " .<(+|"
		 "&         " "!$*);~"
		 "-/        " "|,%_>?"
		 "         `" ":#@'=\""
		 " abcdefghi" "      "
		 " jklmnopqr" "      "
		 " ~stuvwxyz" "      "
		 "^         " "[]    "
		 "{ABCDEFGHI" "      "
		 "}JKLMNOPQR" "      "
		"\\ STUVWXYZ" "      "
		 "0123456789" "      ";

	while (size--)
		*ascii++ = conv_table[(unsigned char) *ebcdic++];
}

# define DECODE_EBCDIC(ebcdic_, ascii_) \
	decode_ebcdic((ebcdic_), (ascii_), \
		      sizeof(ebcdic_) + MUST_BE_ARRAY(ebcdic_))
# define PRINT_EBCDIC(ebcdic_) \
	do { \
		char ascii_str[sizeof(ebcdic_) + MUST_BE_ARRAY(ebcdic_)]; \
		\
		DECODE_EBCDIC(ebcdic_, ascii_str); \
		print_quoted_string(ascii_str, sizeof(ascii_str), \
				    QUOTE_EMIT_COMMENT); \
	} while (0)

# define PRINT_FIELD_EBCDIC(prefix_, where_, field_) \
	do { \
		PRINT_FIELD_HEX_ARRAY(prefix_, where_, field_); \
		PRINT_EBCDIC((where_).field_); \
	} while (0)

# define PRINT_FIELD_WEIGHT(prefix_, where_, field_) \
	do { \
		PRINT_FIELD_X(prefix_, where_, field_); \
		if ((where_).field_) \
			tprintf_comment("%u %u/65536 cores", \
				(where_).field_ >> 16, \
				(where_).field_ & 0xFFFF); \
		else \
			tprints_comment("unlimited"); \
	} while (0)


# define IS_BLANK(arr_) /* 0x40 is space in EBCDIC */ \
	is_filled(arr_, '\x40', sizeof(arr_) + MUST_BE_ARRAY(arr_))

# define CHECK_SIZE_EX(hdr_, min_size_, size_, name_, ...) \
	do { \
		if ((size_) < (min_size_)) { \
			tprintf_comment("Invalid " name_ " with size " \
					"%hu < %zu expected", \
					##__VA_ARGS__, \
					(size_), (min_size_)); \
			print_quoted_string((char *) (hdr_), (size_), \
					    QUOTE_FORCE_HEX); \
			\
			return; \
		} \
	} while (0)

# define CHECK_SIZE(hdr_, size_, name_, ...) \
	CHECK_SIZE_EX((hdr_), sizeof(*(hdr_)), (size_), name_, ##__VA_ARGS__)

# define PRINT_UNKNOWN_TAIL_EX(hdr_, hdr_size_, size_) \
	do { \
		if ((size_) > (hdr_size_) && \
		    !is_filled(((char *) hdr_) + (hdr_size_), '\0', \
		               (size_) - (hdr_size_))) { \
			tprints(", "); \
			print_quoted_string(((char *) hdr_) + (hdr_size_), \
					    (size_) - (hdr_size_), \
					    QUOTE_FORCE_HEX); \
		} \
	} while (0)

# define PRINT_UNKNOWN_TAIL(hdr_, size_) \
	PRINT_UNKNOWN_TAIL_EX((hdr_), sizeof(*(hdr_)), (size_))

static void
print_sthyi_machine(struct tcb *tcp, struct sthyi_machine *hdr, uint16_t size,
		    bool *dummy)
{
	size_t last_decoded = offsetofend(typeof(*hdr), infmpman);
	int cnt_val, name_val, id_val;

	CHECK_SIZE_EX(hdr, last_decoded, size, "machine structure");

	tprints("/* machine */ {");
	if (!abbrev(tcp)) {
		if (hdr->infmflg1) { /* Reserved */
			PRINT_FIELD_0X("", *hdr, infmflg1);
			tprints(", ");
		}
		if (hdr->infmflg2) { /* Reserved */
			PRINT_FIELD_0X(", ", *hdr, infmflg2);
			tprints(", ");
		}
	}

	PRINT_FIELD_0X("", *hdr, infmval1);
	cnt_val  = !!(hdr->infmval1 & 0x80);
	id_val   = !!(hdr->infmval1 & 0x40);
	name_val = !!(hdr->infmval1 & 0x20);

	if (!abbrev(tcp)) {
		if (hdr->infmval1)
			tprintf_comment("processor count validity: %d, "
					"machine ID validity: %d, "
					"machine name validity: %d%s%#.0x%s",
					cnt_val, id_val, name_val,
					hdr->infmval1 & 0x1F ? ", " : "",
					hdr->infmval1 & 0x1F,
					hdr->infmval1 & 0x1F ? " - ???" : "");
		if (hdr->infmval2)
			PRINT_FIELD_0X(", ", *hdr, infmval2);
	}

	if (cnt_val || hdr->infmscps)
		PRINT_FIELD_U(", ", *hdr, infmscps);
	if (cnt_val || hdr->infmdcps)
		PRINT_FIELD_U(", ", *hdr, infmdcps);
	if (cnt_val || hdr->infmsifl)
		PRINT_FIELD_U(", ", *hdr, infmsifl);
	if (cnt_val || hdr->infmdifl)
		PRINT_FIELD_U(", ", *hdr, infmdifl);

	if (!abbrev(tcp)) {
		if (name_val || hdr->infmname)
			PRINT_FIELD_EBCDIC(", ", *hdr, infmname);

		if (id_val || !IS_ARRAY_ZERO(hdr->infmtype))
			PRINT_FIELD_EBCDIC(", ", *hdr, infmtype);
		if (id_val || !IS_ARRAY_ZERO(hdr->infmmanu))
			PRINT_FIELD_EBCDIC(", ", *hdr, infmmanu);
		if (id_val || !IS_ARRAY_ZERO(hdr->infmseq))
			PRINT_FIELD_EBCDIC(", ", *hdr, infmseq);
		if (id_val || !IS_ARRAY_ZERO(hdr->infmpman))
			PRINT_FIELD_EBCDIC(", ", *hdr, infmpman);

		if (size >= offsetofend(struct sthyi_machine, infmplnm)) {
			last_decoded = offsetofend(struct sthyi_machine,
						   infmplnm);

			if (!IS_ARRAY_ZERO(hdr->reserved_1__))
				PRINT_FIELD_HEX_ARRAY(", ", *hdr, reserved_1__);

			if (!IS_ARRAY_ZERO(hdr->infmplnm))
				PRINT_FIELD_EBCDIC(", ", *hdr, infmplnm);
		}

		PRINT_UNKNOWN_TAIL_EX(hdr, last_decoded, size);
	} else {
		tprints(", ...");
	}

	tprints("}");
}

static void
print_sthyi_partition(struct tcb *tcp, struct sthyi_partition *hdr,
		      uint16_t size, bool *mt)
{
	size_t last_decoded = offsetofend(typeof(*hdr), infpabif);
	int cnt_val, wcap_val, acap_val, id_val, lpar_val;

	*mt = false;

	CHECK_SIZE_EX(hdr, last_decoded, size, "partition structure");

	*mt = !!(hdr->infpflg1 & 0x80);

	PRINT_FIELD_0X("/* partition */ {", *hdr, infpflg1);
	if (!abbrev(tcp) && hdr->infpflg1)
		tprintf_comment("%s%s%#.0x%s",
			hdr->infpflg1 & 0x80 ?
				"0x80 - multithreading is enabled" : "",
			(hdr->infpflg1 & 0x80) && (hdr->infpflg1 & 0x7F) ?
				", " : "",
			hdr->infpflg1 & 0x7F,
			hdr->infpflg1 & 0x7F ? " - ???" : "");
	if (!abbrev(tcp) && hdr->infpflg2) /* Reserved */
		PRINT_FIELD_0X(", ", *hdr, infpflg2);

	PRINT_FIELD_0X(", ", *hdr, infpval1);
	cnt_val  = !!(hdr->infpval1 & 0x80);
	wcap_val = !!(hdr->infpval1 & 0x40);
	acap_val = !!(hdr->infpval1 & 0x20);
	id_val   = !!(hdr->infpval1 & 0x10);
	lpar_val = !!(hdr->infpval1 & 0x08);

	if (!abbrev(tcp) && hdr->infpval1)
		tprintf_comment("processor count validity: %d, "
				"partition weight-based capacity validity: %d, "
				"partition absolute capacity validity: %d, "
				"partition ID validity: %d, "
				"LPAR group absolute capacity capping "
				"information validity: %d%s%#.0x%s",
				cnt_val, wcap_val, acap_val, id_val, lpar_val,
				hdr->infpval1 & 0x07 ? ", " : "",
				hdr->infpval1 & 0x07,
				hdr->infpval1 & 0x07 ? " - ???" : "");
	if (!abbrev(tcp) && hdr->infpval2) /* Reserved */
		PRINT_FIELD_0X(", ", *hdr, infpval2);

	if (id_val || hdr->infppnum)
		PRINT_FIELD_U(", ", *hdr, infppnum);

	if (cnt_val || hdr->infpscps)
		PRINT_FIELD_U(", ", *hdr, infpscps);
	if (cnt_val || hdr->infpdcps)
		PRINT_FIELD_U(", ", *hdr, infpdcps);
	if (cnt_val || hdr->infpsifl)
		PRINT_FIELD_U(", ", *hdr, infpsifl);
	if (cnt_val || hdr->infpdifl)
		PRINT_FIELD_U(", ", *hdr, infpdifl);

	if (!abbrev(tcp) && !IS_ARRAY_ZERO(hdr->reserved_1__))
		PRINT_FIELD_HEX_ARRAY(", ", *hdr, reserved_1__);

	if (id_val || !IS_ARRAY_ZERO(hdr->infppnam))
		PRINT_FIELD_EBCDIC(", ", *hdr, infppnam);

	if (!abbrev(tcp)) {
		if (wcap_val || hdr->infpwbcp)
			PRINT_FIELD_WEIGHT(", ", *hdr, infpwbcp);
		if (acap_val || hdr->infpabcp)
			PRINT_FIELD_WEIGHT(", ", *hdr, infpabcp);
		if (wcap_val || hdr->infpwbif)
			PRINT_FIELD_WEIGHT(", ", *hdr, infpwbif);
		if (acap_val || hdr->infpabif)
			PRINT_FIELD_WEIGHT(", ", *hdr, infpabif);

		if (size >= offsetofend(struct sthyi_partition, infplgif)) {
			if (!IS_ARRAY_ZERO(hdr->infplgnm)) {
				PRINT_FIELD_EBCDIC(", ", *hdr, infplgnm);

				PRINT_FIELD_WEIGHT(", ", *hdr, infplgcp);
				PRINT_FIELD_WEIGHT(", ", *hdr, infplgif);
			} else {
				if (lpar_val)
					PRINT_FIELD_HEX_ARRAY(", ", *hdr, infplgnm);
				if (hdr->infplgcp)
					PRINT_FIELD_X(", ", *hdr, infplgcp);
				if (hdr->infplgif)
					PRINT_FIELD_X(", ", *hdr, infplgif);
			}
		}

		if (size >= offsetofend(struct sthyi_partition, infpplnm)) {
			last_decoded = offsetofend(struct sthyi_partition,
						   infpplnm);

			if (!IS_ARRAY_ZERO(hdr->infpplnm))
				PRINT_FIELD_EBCDIC(", ", *hdr, infpplnm);
		}

		PRINT_UNKNOWN_TAIL_EX(hdr, last_decoded, size);
	} else {
		tprints(", ...");
	}

	tprints("}");
}

static void
print_funcs(const uint8_t funcs[8])
{
	static const char *func_descs[] = {
		[0] = "Obtain CPU Capacity Info",
		[1] = "Hypervisor Environment Info",
		[2] = "Guest List",
		[3] = "Designated Guest Info",
		[4] = "Resource Pool List",
		[5] = "Designated Resource Pool Information",
		[6] = "Resource Pool Member List",
	};

	static_assert(ARRAY_SIZE(func_descs) <= 64,
		      "func_descs is too big");

	if (is_filled((const char *) funcs, 0, 8))
		return;

	bool cont = false;

	for (size_t i = 0; i < ARRAY_SIZE(func_descs); i++) {
		if (!func_descs[i])
			continue;

		size_t b = i >> 3;
		size_t f = 1 << (7 - (i & 7));

		if (!(funcs[b] & f))
			continue;

		tprintf("%s%zu: %s", cont ? ", " : " /* ", i, func_descs[i]);
		cont = true;
	}

	if (cont)
		tprints(" */");
}

static void
print_sthyi_hypervisor(struct tcb *tcp, struct sthyi_hypervisor *hdr,
		       uint16_t size, int num, bool mt)
{
	size_t last_decoded = offsetofend(typeof(*hdr), infydifl);

	CHECK_SIZE_EX(hdr, last_decoded, size, "hypervisor %d structure", num);

	tprintf("/* hypervisor %d */ ", num);
	PRINT_FIELD_0X("{", *hdr, infyflg1);
	if (!abbrev(tcp) && hdr->infyflg1)
		tprintf_comment("%s%s%s%s%s%s%#.0x%s",
			hdr->infyflg1 & 0x80 ?
				"0x80 - guest CPU usage had limiting is using "
				"the consumption method" : "",
			(hdr->infyflg1 & 0x80) && (hdr->infyflg1 & 0x40) ?
				", " : "",
			hdr->infyflg1 & 0x40 ?
				"0x40 - LIMITHARD caps use prorated core time "
				"for capping" : "",
			(hdr->infyflg1 & 0xC0) && (hdr->infyflg1 & 0x20) ?
				", " : "",
			hdr->infyflg1 & 0x20 ?
				"0x20 - hypervisor is MT-enabled" :"",
			(hdr->infyflg1 & 0xE0) && (hdr->infyflg1 & 0x1F) ?
				", " : "",
			hdr->infyflg1 & 0x1F,
			hdr->infyflg1 & 0x1F ? " - ???" : "");

	if (!abbrev(tcp)) {
		if (hdr->infyflg2) /* Reserved */
			PRINT_FIELD_0X(", ", *hdr, infyflg2);
		if (hdr->infyval1) /* Reserved */
			PRINT_FIELD_0X(", ", *hdr, infyval1);
		if (hdr->infyval2) /* Reserved */
			PRINT_FIELD_0X(", ", *hdr, infyval2);

		PRINT_FIELD_U(", ", *hdr, infytype);
		switch (hdr->infytype) {
		case 1:
			tprints_comment("z/VM is the hypervisor");
			break;
		default:
			tprints_comment("unknown hypervisor type");
		}

		if (!IS_ARRAY_ZERO(hdr->reserved_1__))
			PRINT_FIELD_HEX_ARRAY(", ", *hdr, reserved_1__);

		if (mt || hdr->infycpt)
			PRINT_FIELD_U(", ", *hdr, infycpt);
		if (mt || hdr->infyiflt)
			PRINT_FIELD_U(", ", *hdr, infyiflt);
	}

	if (!abbrev(tcp) || !IS_BLANK(hdr->infysyid))
		PRINT_FIELD_EBCDIC(", ", *hdr, infysyid);
	if (!abbrev(tcp) || !IS_BLANK(hdr->infyclnm))
		PRINT_FIELD_EBCDIC(", ", *hdr, infyclnm);

	if (!abbrev(tcp) || hdr->infyscps)
		PRINT_FIELD_U(", ", *hdr, infyscps);
	if (!abbrev(tcp) || hdr->infydcps)
		PRINT_FIELD_U(", ", *hdr, infydcps);
	if (!abbrev(tcp) || hdr->infysifl)
		PRINT_FIELD_U(", ", *hdr, infysifl);
	if (!abbrev(tcp) || hdr->infydifl)
		PRINT_FIELD_U(", ", *hdr, infydifl);

	if (!abbrev(tcp)) {
		if (size >= offsetofend(struct sthyi_hypervisor, infyautf)) {
			last_decoded = offsetofend(struct sthyi_hypervisor,
						   infyautf);

			PRINT_FIELD_HEX_ARRAY(", ", *hdr, infyinsf);
			print_funcs(hdr->infyinsf);

			PRINT_FIELD_HEX_ARRAY(", ", *hdr, infyautf);
			print_funcs(hdr->infyautf);
		}

		PRINT_UNKNOWN_TAIL_EX(hdr, last_decoded, size);
	} else {
		tprints(", ...");
	}

	tprints("}");
}

static void
print_sthyi_guest(struct tcb *tcp, struct sthyi_guest *hdr, uint16_t size,
		  int num, bool mt)
{
	CHECK_SIZE(hdr, size, "guest %d structure", num);

	tprintf("/* guest %d */ ", num);
	PRINT_FIELD_0X("{", *hdr, infgflg1);
	if (!abbrev(tcp) && hdr->infgflg1)
		tprintf_comment("%s%s%s%s%s%s%s%s%s%s%s%s%#.0x%s",
			hdr->infgflg1 & 0x80 ?
				"0x80 - guest is mobility enabled" : "",
			(hdr->infgflg1 & 0x80) && (hdr->infgflg1 & 0x40) ?
				", " : "",
			hdr->infgflg1 & 0x40 ?
				"0x40 - guest has multiple virtual CPU types" :
				"",
			(hdr->infgflg1 & 0xC0) && (hdr->infgflg1 & 0x20) ?
				", " : "",
			hdr->infgflg1 & 0x20 ?
				"0x20 - guest CP dispatch type has LIMITHARD "
				"cap" : "",
			(hdr->infgflg1 & 0xE0) && (hdr->infgflg1 & 0x10) ?
				", " : "",
			hdr->infgflg1 & 0x10 ?
				"0x10 - guest IFL dispatch type has LIMITHARD "
				"cap" : "",
			(hdr->infgflg1 & 0xF0) && (hdr->infgflg1 & 0x08) ?
				", " : "",
			hdr->infgflg1 & 0x08 ?
				"0x08 - virtual CPs are thread dispatched" :
				"",
			(hdr->infgflg1 & 0xF8) && (hdr->infgflg1 & 0x04) ?
				", " : "",
			hdr->infgflg1 & 0x04 ?
				"0x04 - virtual IFLs are thread dispatched" :
				"",
			(hdr->infgflg1 & 0xFC) && (hdr->infgflg1 & 0x03) ?
				", " : "",
			hdr->infgflg1 & 0x03,
			hdr->infgflg1 & 0x03 ? " - ???" : "");
	if (!abbrev(tcp)) {
		if (hdr->infgflg2) /* Reserved */
			PRINT_FIELD_0X(", ", *hdr, infgflg2);
		if (hdr->infgval1) /* Reserved */
			PRINT_FIELD_0X(", ", *hdr, infgval1);
		if (hdr->infgval2) /* Reserved */
			PRINT_FIELD_0X(", ", *hdr, infgval2);
	}

	PRINT_FIELD_EBCDIC(", ", *hdr, infgusid);

	if (!abbrev(tcp) || hdr->infgscps)
		PRINT_FIELD_U(", ", *hdr, infgscps);
	if (!abbrev(tcp) || hdr->infgdcps)
		PRINT_FIELD_U(", ", *hdr, infgdcps);

	if (!abbrev(tcp)) {
		PRINT_FIELD_U(", ", *hdr, infgcpdt);
		switch (hdr->infgcpdt) {
		case 0:
			tprints_comment("General Purpose (CP)");
			break;
		default:
			tprints_comment("unknown");
		}

		if (!IS_ARRAY_ZERO(hdr->reserved_1__))
			PRINT_FIELD_HEX_ARRAY(", ", *hdr, reserved_1__);
	}

	if (!abbrev(tcp) || hdr->infgcpcc)
		PRINT_FIELD_WEIGHT(", ", *hdr, infgcpcc);

	if (!abbrev(tcp) || hdr->infgsifl)
		PRINT_FIELD_U(", ", *hdr, infgsifl);
	if (!abbrev(tcp) || hdr->infgdifl)
		PRINT_FIELD_U(", ", *hdr, infgdifl);

	if (!abbrev(tcp)) {
		PRINT_FIELD_U(", ", *hdr, infgifdt);
		switch (hdr->infgifdt) {
		case 0:
			tprints_comment("General Purpose (CP)");
			break;
		case 3:
			tprints_comment("Integrated Facility for Linux (IFL)");
			break;
		default:
			tprints_comment("unknown");
		}

		if (!IS_ARRAY_ZERO(hdr->reserved_2__))
			PRINT_FIELD_HEX_ARRAY(", ", *hdr, reserved_2__);
	}

	if (!abbrev(tcp) || hdr->infgifcc)
		PRINT_FIELD_WEIGHT(", ", *hdr, infgifcc);

	PRINT_FIELD_0X(", ", *hdr, infgpflg);
	if (!abbrev(tcp) && hdr->infgpflg)
		tprintf_comment("%s%s%s%s%s%s%s%s%s%s%#.0x%s",
			hdr->infgpflg & 0x80 ?
				"0x80 - CPU pool's CP virtual type has "
				"LIMITHARD cap" : "",
			(hdr->infgpflg & 0x80) && (hdr->infgpflg & 0x40) ?
				", " : "",
			hdr->infgpflg & 0x40 ?
				"0x40 - CPU pool's CP virtual type has "
				"CAPACITY cap" : "",
			(hdr->infgpflg & 0xC0) && (hdr->infgpflg & 0x20) ?
				", " : "",
			hdr->infgpflg & 0x20 ?
				"0x20 - CPU pool's IFL virtual type has "
				"LIMITHARD cap" : "",
			(hdr->infgpflg & 0xE0) && (hdr->infgpflg & 0x10) ?
				", " : "",
			hdr->infgpflg & 0x10 ?
				"0x10 - CPU pool's IFL virtual type has "
				"CAPACITY cap" : "",
			(hdr->infgpflg & 0xF0) && (hdr->infgpflg & 0x08) ?
				", " : "",
			hdr->infgpflg & 0x08 ?
				"0x08 - CPU pool uses prorated core time" : "",
			(hdr->infgpflg & 0xF8) && (hdr->infgpflg & 0x07) ?
				", " : "",
			hdr->infgpflg & 0x07,
			hdr->infgpflg & 0x07 ? " - ???" : "");

	if (!abbrev(tcp)) {
		if (!IS_ARRAY_ZERO(hdr->reserved_3__))
			PRINT_FIELD_HEX_ARRAY(", ", *hdr, reserved_3__);

		if (!IS_BLANK(hdr->infgpnam))
			PRINT_FIELD_EBCDIC(", ", *hdr, infgpnam);

		PRINT_FIELD_WEIGHT(", ", *hdr, infgpccc);
		PRINT_FIELD_WEIGHT(", ", *hdr, infgpicc);

		PRINT_UNKNOWN_TAIL(hdr, size);
	} else {
		tprints(", ...");
	}

	tprints("}");
}

# define STHYI_PRINT_STRUCT(l_, name_) \
	do { \
		if (hdr->inf ##l_## off && hdr->inf ##l_## off + \
		    hdr->inf ##l_## len <= sizeof(data)) { \
			tprints(", "); \
			print_sthyi_ ##name_(tcp, (struct sthyi_ ##name_ *) \
					     (data + hdr->inf ##l_## off), \
					     hdr->inf ##l_## len, &mt); \
		} \
	} while (0)

# define STHYI_PRINT_HV_STRUCT(l_, n_, name_) \
	do { \
		if (hdr->inf ##l_## off ##n_ && hdr->inf ##l_## off ##n_ + \
		    hdr->inf ##l_## len ##n_ <= sizeof(data)) { \
			tprints(", "); \
			print_sthyi_ ##name_(tcp, (struct sthyi_ ##name_ *) \
					     (data + hdr->inf ##l_## off ##n_), \
					     hdr->inf ##l_## len ##n_, n_, mt); \
		} \
	} while (0)

static void
print_sthyi_buf(struct tcb *tcp, kernel_ulong_t ptr)
{
	char data[PAGE_SIZE];
	struct sthyi_hdr *hdr = (struct sthyi_hdr *) data;
	bool mt = false;

	if (umove_or_printaddr(tcp, ptr, &data))
		return;

	tprints("{");

	/* Header */
	PRINT_FIELD_0X("/* header */ {", *hdr, infhflg1);

	if (abbrev(tcp)) {
		tprints(", ...");
		goto sthyi_sections;
	}

	if (hdr->infhflg1)
		tprintf_comment("%s%s%s%s%s%s%s%s%#.0x%s",
			hdr->infhflg1 & 0x80 ?
				"0x80 - Global Performance Data unavailable" :
				"",
			(hdr->infhflg1 & 0x80) && (hdr->infhflg1 & 0x40) ?
				", " : "",
			hdr->infhflg1 & 0x40 ?
				"0x40 - One or more hypervisor levels below "
				"this level does not support the STHYI "
				"instruction" : "",
			(hdr->infhflg1 & 0xC0) && (hdr->infhflg1 & 0x20) ?
				", " : "",
			hdr->infhflg1 & 0x20 ?
				"0x20 - Virtualization stack is incomplete" :
				"",
			(hdr->infhflg1 & 0xE0) && (hdr->infhflg1 & 0x10) ?
				", " : "",
			hdr->infhflg1 & 0x10 ?
				"0x10 - Execution environment is not within "
				"a logical partition" : "",
			(hdr->infhflg1 & 0xF0) && (hdr->infhflg1 & 0x0F) ?
				", " : "",
			hdr->infhflg1 & 0x0F,
			hdr->infhflg1 & 0x0F ? " - ???" : "");
	if (hdr->infhflg2) /* Reserved */
		PRINT_FIELD_0X(", ", *hdr, infhflg2);
	if (hdr->infhval1) /* Reserved */
		PRINT_FIELD_0X(", ", *hdr, infhval1);
	if (hdr->infhval2) /* Reserved */
		PRINT_FIELD_0X(", ", *hdr, infhval2);

	if (!IS_ARRAY_ZERO(hdr->reserved_1__))
		PRINT_FIELD_HEX_ARRAY(", ", *hdr, reserved_1__);

	PRINT_FIELD_U(", ", *hdr, infhygct);
	PRINT_FIELD_U(", ", *hdr, infhtotl);

	PRINT_FIELD_U(", ", *hdr, infhdln);
	PRINT_FIELD_U(", ", *hdr, infmoff);
	PRINT_FIELD_U(", ", *hdr, infmlen);
	PRINT_FIELD_U(", ", *hdr, infpoff);
	PRINT_FIELD_U(", ", *hdr, infplen);

	PRINT_FIELD_U(", ", *hdr, infhoff1);
	PRINT_FIELD_U(", ", *hdr, infhlen1);
	PRINT_FIELD_U(", ", *hdr, infgoff1);
	PRINT_FIELD_U(", ", *hdr, infglen1);
	PRINT_FIELD_U(", ", *hdr, infhoff2);
	PRINT_FIELD_U(", ", *hdr, infhlen2);
	PRINT_FIELD_U(", ", *hdr, infgoff2);
	PRINT_FIELD_U(", ", *hdr, infglen2);
	PRINT_FIELD_U(", ", *hdr, infhoff3);
	PRINT_FIELD_U(", ", *hdr, infhlen3);
	PRINT_FIELD_U(", ", *hdr, infgoff3);
	PRINT_FIELD_U(", ", *hdr, infglen3);

	PRINT_UNKNOWN_TAIL(hdr, hdr->infhdln);

sthyi_sections:
	tprints("}");

	STHYI_PRINT_STRUCT(m, machine);
	STHYI_PRINT_STRUCT(p, partition);

	STHYI_PRINT_HV_STRUCT(h, 1, hypervisor);
	STHYI_PRINT_HV_STRUCT(g, 1, guest);
	STHYI_PRINT_HV_STRUCT(h, 2, hypervisor);
	STHYI_PRINT_HV_STRUCT(g, 2, guest);
	STHYI_PRINT_HV_STRUCT(h, 3, hypervisor);
	STHYI_PRINT_HV_STRUCT(g, 3, guest);

	tprints("}");
}

/**
 * Wrapper for the s390 STHYI instruction that provides hypervisor information.
 *
 * See
 * https://www.ibm.com/support/knowledgecenter/SSB27U_6.4.0/com.ibm.zvm.v640.hcpb4/hcpb4sth.htm
 * https://web.archive.org/web/20170306000915/https://www.ibm.com/support/knowledgecenter/SSB27U_6.3.0/com.ibm.zvm.v630.hcpb4/hcpb4sth.htm
 * for the instruction documentation.
 *
 * The difference in the kernel wrapper is that it doesn't require the 4K
 * alignment for the resp_buffer page (as it just copies from the internal
 * cache).
 */
SYS_FUNC(s390_sthyi)
{
	/* in, function ID from s390_sthyi_function_codes */
	kernel_ulong_t function_code = tcp->u_arg[0];
	/* out, pointer to page-sized buffer */
	kernel_ulong_t resp_buffer_ptr = tcp->u_arg[1];
	/* out, pointer to u64 containing function result */
	kernel_ulong_t return_code_ptr = tcp->u_arg[2];
	/* in, should be 0, at the moment */
	kernel_ulong_t flags = tcp->u_arg[3];

	if (entering(tcp)) {
		printxval64(s390_sthyi_function_codes, function_code,
			    "STHYI_FC_???");
		tprints(", ");
	} else {
		switch (function_code) {
		case STHYI_FC_CP_IFL_CAP:
			print_sthyi_buf(tcp, resp_buffer_ptr);
			break;

		default:
			printaddr(resp_buffer_ptr);
		}

		tprints(", ");
		printnum_int64(tcp, return_code_ptr, "%" PRIu64);
		tprintf(", %#" PRI_klx, flags);
	}

	return 0;
}


/*
 * Structures are written based on
 * https://www-304.ibm.com/support/docview.wss?uid=isg29c69415c1e82603c852576700058075a&aid=1#page=85
 */

struct guard_storage_control_block {
	uint64_t reserved;
	/**
	 * Guard Storage Designation
	 *  - Bits 0..J, J == 64-GSC - Guard Storage Origin (GSO)
	 *  - Bits 53..55 - Guard Load Shift (GLS)
	 *  - Bits 58..63 - Guard Storage Characteristic (GSC), this is J from
	 *                  the first item, valud values are 25..56.
	 */
	uint64_t gsd;
	uint64_t gssm;     /**< Guard Storage Section Mask */
	uint64_t gs_epl_a; /**< Guard Storage Event Parameter List Address */
};

struct guard_storage_event_parameter_list {
	uint8_t  pad1;
	/**
	 * Guard Storage Event Addressing Mode
	 *  - 0x40 - Extended addressing mode (E)
	 *  - 0x80 - Basic addressing mode (B)
	 */
	uint8_t  gs_eam;
	/**
	 * Guard Storage Event Cause indication
	 *  - 0x01 - CPU was in transaction execution mode (TX)
	 *  - 0x02 - CPU was in constrained transaction execution mode (CX)
	 *  - 0x80 - Instruction causing the event: 0 - LGG, 1 - LLGFGS
	 */
	uint8_t  gs_eci;
	/**
	 * Guard Storage Event Access Information
	 *  - 0x01 - DAT mode
	 *  - Bits 1..2 - Address space indication
	 *  - Bits 4..7 - AR number
	 */
	uint8_t  gs_eai;
	uint32_t pad2;
	uint64_t gs_eha; /**< Guard Storage Event Handler Address */
	uint64_t gs_eia; /**< Guard Storage Event Instruction Address */
	uint64_t gs_eoa; /**< Guard Storage Event Operation Address */
	uint64_t gs_eir; /**< Guard Storage Event Intermediate Result */
	uint64_t gs_era; /**< Guard Storage Event Return Address */
};

static void
guard_storage_print_gsepl(struct tcb *tcp, uint64_t addr)
{
	struct guard_storage_event_parameter_list gsepl;

	/* Since it is 64-bit even on 31-bit s390... */
	if (sizeof(addr) > current_klongsize &&
	    addr >= (1ULL << (current_klongsize * 8))) {
		tprintf("%#" PRIx64, addr);

		return;
	}

	if (umove_or_printaddr(tcp, addr, &gsepl))
		return;

	tprints("[{");

	if (!abbrev(tcp)) {
		if (gsepl.pad1) {
			PRINT_FIELD_0X("", gsepl, pad1);
			tprints(", ");
		}

		PRINT_FIELD_0X("",   gsepl, gs_eam);
		tprintf_comment("extended addressing mode: %u, "
				"basic addressing mode: %u",
				!!(gsepl.gs_eam & 0x2), !!(gsepl.gs_eam & 0x1));

		PRINT_FIELD_0X(", ", gsepl, gs_eci);
		tprintf_comment("CPU in TX: %u, CPU in CX: %u, instruction: %s",
				!!(gsepl.gs_eci & 0x80),
				!!(gsepl.gs_eci & 0x40),
				gsepl.gs_eci & 0x01 ? "LLGFGS" : "LGG");

		PRINT_FIELD_0X(", ", gsepl, gs_eai);
		tprintf_comment("DAT: %u, address space indication: %u, "
				"AR number: %u",
				!!(gsepl.gs_eai & 0x40),
				(gsepl.gs_eai >> 4) & 0x3,
				gsepl.gs_eai & 0xF);

		if (gsepl.pad2)
			PRINT_FIELD_0X(", ", gsepl, pad2);

		tprints(", ");
	}

	PRINT_FIELD_X("", gsepl, gs_eha);

	if (!abbrev(tcp)) {
		PRINT_FIELD_X(", ", gsepl, gs_eia);
		PRINT_FIELD_X(", ", gsepl, gs_eoa);
		PRINT_FIELD_X(", ", gsepl, gs_eir);
		PRINT_FIELD_X(", ", gsepl, gs_era);
	} else {
		tprints(", ...");
	}

	tprints("}]");
}

# define DIV_ROUND_UP(x,y) (((x) + ((y) - 1)) / (y))

static void
guard_storage_print_gscb(struct tcb *tcp, kernel_ulong_t addr)
{
	struct guard_storage_control_block gscb;

	if (umove_or_printaddr(tcp, addr, &gscb))
		return;

	tprints("{");

	if (gscb.reserved) {
		PRINT_FIELD_0X("", gscb, reserved);
		tprints(", ");
	}

	PRINT_FIELD_0X("", gscb, gsd);

	if (!abbrev(tcp)) {
		unsigned int gsc = gscb.gsd & 0x3F;
		bool gsc_valid = gsc >= 25 && gsc <= 56;
		tprintf_comment("GS origin: %#*.*" PRIx64 "%s, "
				"guard load shift: %" PRIu64 ", "
				"GS characteristic: %u",
				gsc_valid ? 2 + DIV_ROUND_UP(64 - gsc, 4) : 0,
				gsc_valid ? DIV_ROUND_UP(64 - gsc, 4) : 0,
				gsc_valid ? gscb.gsd >> gsc : 0,
				gsc_valid ? "" : "[invalid]",
				(gscb.gsd >> 8) & 0x7, gsc);
	}

	PRINT_FIELD_0X(", ", gscb, gssm);

	tprints(", gs_epl_a=");
	guard_storage_print_gsepl(tcp, gscb.gs_epl_a);

	tprints("}");
}

SYS_FUNC(s390_guarded_storage)
{
	int command = (int) tcp->u_arg[0];
	kernel_ulong_t gs_cb = tcp->u_arg[1];

	printxval(s390_guarded_storage_commands, command, "GS_???");

	switch (command) {
	case GS_ENABLE:
	case GS_DISABLE:
	case GS_CLEAR_BC_CB:
	case GS_BROADCAST:
		break;

	case GS_SET_BC_CB:
		tprints(", ");
		guard_storage_print_gscb(tcp, gs_cb);
		break;

	default:
		tprints(", ");
		printaddr(gs_cb);
	}

	return RVAL_DECODED;
}

SYS_FUNC(s390_runtime_instr)
{
	int command = (int) tcp->u_arg[0];
	int signum = (int) tcp->u_arg[1];


	printxval_d(s390_runtime_instr_commands, command,
		    "S390_RUNTIME_INSTR_???");

	/*
	 * signum is ignored since Linux 4.4, but let's print it for start
	 * command anyway.
	 */
	switch (command) {
	case S390_RUNTIME_INSTR_START:
		tprints(", ");
		printsignal(signum);
		break;

	case S390_RUNTIME_INSTR_STOP:
	default:
		break;
	}

	return RVAL_DECODED;
}

SYS_FUNC(s390_pci_mmio_write)
{
	kernel_ulong_t mmio_addr = tcp->u_arg[0];
	kernel_ulong_t user_buf  = tcp->u_arg[1];
	kernel_ulong_t length    = tcp->u_arg[2];

	tprintf("%#" PRI_klx ", ", mmio_addr);
	printstr_ex(tcp, user_buf, length, QUOTE_FORCE_HEX);
	tprintf(", %" PRI_klu, length);

	return RVAL_DECODED;
}

SYS_FUNC(s390_pci_mmio_read)
{
	kernel_ulong_t mmio_addr = tcp->u_arg[0];
	kernel_ulong_t user_buf  = tcp->u_arg[1];
	kernel_ulong_t length    = tcp->u_arg[2];

	if (entering(tcp)) {
		tprintf("%#" PRI_klx ", ", mmio_addr);
	} else {
		if (!syserror(tcp))
			printstr_ex(tcp, user_buf, length, QUOTE_FORCE_HEX);
		else
			printaddr(user_buf);

		tprintf(", %" PRI_klu, length);
	}

	return 0;
}

#endif /* defined S390 || defined S390X */
