#include "defs.h"

#include <linux/spi/spidev.h>

#include "xlat/spi_mode.h"

static int
spidev_mode_byte(struct tcb *const tcp, const kernel_ulong_t arg)
{
    uint8_t val;

    if (entering(tcp))
        return 0;

    tprint_arg_next();
    if (umove_or_printaddr(tcp, arg, &val))
        return RVAL_IOCTL_DECODED;

    printflags(spi_mode, val, "SPI_???");

    return RVAL_IOCTL_DECODED;
}

static int
spidev_mode_u32(struct tcb *const tcp, const kernel_ulong_t arg)
{
    uint32_t val;

    if (entering(tcp))
        return 0;

    tprint_arg_next();
    if (umove_or_printaddr(tcp, arg, &val))
        return RVAL_IOCTL_DECODED;

    printflags(spi_mode, val, "SPI_???");

    return RVAL_IOCTL_DECODED;
}

static bool
print_spi_buf(struct tcb * tcp, kernel_ulong_t buf_k, const char* field_name, unsigned int len)
{
    char* local_buf = malloc(len);
    if (local_buf == NULL) {
        tprint_unavailable();
        return false;
    }
    tprints_field_name(field_name);
    if (!umoven_or_printaddr(tcp, buf_k, len, local_buf)) {
        print_quoted_string(local_buf, len, QUOTE_FORCE_HEX);
    }
    tprint_struct_next();

    free(local_buf);

    return true;
}

static bool
print_spi_ioc_transfer(struct tcb * tcp, void *elem_buf, size_t elem_size, void *opaque_data)
{
    struct spi_ioc_transfer* elem = (struct spi_ioc_transfer*)elem_buf;

    tprint_struct_begin();
    if (!print_spi_buf(tcp, elem->tx_buf, "tx_buf", elem->len))
        return false;
    if (!print_spi_buf(tcp, elem->rx_buf, "rx_buf", elem->len))
        return false;
    PRINT_FIELD_X(*elem, speed_hz);
	tprint_struct_next();
    PRINT_FIELD_X(*elem, delay_usecs);
	tprint_struct_next();
    PRINT_FIELD_X(*elem, bits_per_word);
	tprint_struct_next();
    PRINT_FIELD_X(*elem, cs_change);
	tprint_struct_next();
    PRINT_FIELD_X(*elem, tx_nbits);
	tprint_struct_next();
    PRINT_FIELD_X(*elem, rx_nbits);
	tprint_struct_next();
    PRINT_FIELD_X(*elem, word_delay_usecs);
	tprint_struct_next();
    PRINT_FIELD_X(*elem, pad);
    tprint_struct_end();

    return true;
}

static int
spidev_message(struct tcb *const tcp, const kernel_ulong_t arg, int len)
{
    if (entering(tcp))
        return 0;

    tprint_arg_next();

    struct spi_ioc_transfer buf;
    print_array(tcp, arg, len, &buf, sizeof(buf), tfetch_mem, print_spi_ioc_transfer, NULL);

    return RVAL_IOCTL_DECODED;
}

static int
spidev_max_speed_hz(struct tcb *const tcp, const kernel_ulong_t arg)
{
    if (entering(tcp))
        return 0;

    tprint_arg_next();

    uint32_t speed;
    if (umove_or_printaddr(tcp, arg, &speed))
        return RVAL_IOCTL_DECODED;

    printnum_ulong(tcp, speed);

    return RVAL_IOCTL_DECODED;
}

static int
spidev_bits_per_word(struct tcb *const tcp, const kernel_ulong_t arg)
{
    if (entering(tcp))
        return 0;

    tprint_arg_next();

    uint8_t bpw;
    if (umove_or_printaddr(tcp, arg, &bpw))
        return RVAL_IOCTL_DECODED;

    printnum_ulong(tcp, bpw);

    return RVAL_IOCTL_DECODED;
}

static int
spidev_lsb_first(struct tcb *const tcp, const kernel_ulong_t arg)
{
    if (entering(tcp))
        return 0;

    tprint_arg_next();

    uint8_t lsb_first;
    if (umove_or_printaddr(tcp, arg, &lsb_first))
        return RVAL_IOCTL_DECODED;

    if (lsb_first == 0) {
        STRACE_PRINTS("MSB_FIRST");
    } else {
        STRACE_PRINTS("LSB_FIRST");
    }

    return RVAL_IOCTL_DECODED;
}

int
spidev_ioctl(struct tcb *const tcp, const unsigned int code, const kernel_ulong_t arg)
{
    // IOC_MESSAGE
    if (_IOC_DIR(code) == _IOC_WRITE && _IOC_NR(code) == _IOC_NR(SPI_IOC_MESSAGE(0))) {
        return spidev_message(tcp, arg, _IOC_SIZE(code) / sizeof(struct spi_ioc_transfer));
    }

    switch (code) {
        case SPI_IOC_RD_MODE:
        case SPI_IOC_WR_MODE:
            return spidev_mode_byte(tcp, arg);
        case SPI_IOC_RD_MODE32:
        case SPI_IOC_WR_MODE32:
            return spidev_mode_u32(tcp, arg);
        case SPI_IOC_RD_LSB_FIRST:
        case SPI_IOC_WR_LSB_FIRST:
            return spidev_lsb_first(tcp, arg);
        case SPI_IOC_RD_BITS_PER_WORD:
        case SPI_IOC_WR_BITS_PER_WORD:
            return spidev_bits_per_word(tcp, arg);
        case SPI_IOC_RD_MAX_SPEED_HZ:
        case SPI_IOC_WR_MAX_SPEED_HZ:
            return spidev_max_speed_hz(tcp, arg);
    }

	return RVAL_DECODED;
}