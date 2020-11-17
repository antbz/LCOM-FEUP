#ifndef DEFS_H
#define DEFS_H

#include <lcom/lcf.h>

// Register defines
#define KBD_STAT_REG    0x64
#define KBC_CMD_REG 0x64
#define OUT_BUF     0x60
#define KBC_ARG_REG 0x60
#define IRQ1        1

// Status byte defines
#define PARITY    BIT(7)
#define TIMEOUT   BIT(6)
#define AUX       BIT(5)
#define INH       BIT(4)
#define A2        BIT(3)
#define SYS       BIT(2)
#define IBF       BIT(1)
#define OBF       BIT(0)

// Two byte codes
#define TWO_BYTE 0xe0

// Command defines
#define READ_CMD 0x20
#define WRITE_CMD 0x60

// Misc
#define WAIT_KBC  20000

#endif
