#include <lcom/lcf.h>

#define VIDEOCARD   0x10

#define VBE_FUNC    0x4F

#define GET_MODE_INFO 0x01
#define SET_VBE_MODE 0x02

#define LINEAR_FB  1<<14

#define PC_CONFIG   0x11
#define MEM_CONFIG  0x12
#define KEYBOARD    0x16

#define VIDEO_MODE  0x105


// KEYBOARD DEFINES
// Register defines
#define KBD_STAT_REG    0x64
#define KBC_CMD_REG   0x64
#define OUT_BUF       0x60
#define KBC_ARG_REG   0x60
#define IRQ1          1

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
#define ESC       0x81
