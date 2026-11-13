/* Valve Steam Controller (2026), nRF52833: 512 KB flash, 128 KB RAM.
 *
 *   0x00000  Valve's bootloader (32 KB), never written
 *   0x08000  the firmware, as the bootloader loads it
 *   0x70000  what the firmware saves (board_config.h NRF52_FLASH_STORAGE_*)
 *   0x7C000  the bootloader's copy of the firmware's header
 *   0x7D000  Valve's own settings (calibration, pairing), left alone
 *
 * The last 16 bytes of RAM are the bootloader's: a request left there survives the reset
 * (main.c cb_nrf52_bootloader_request).
 *
 * Every restart goes through the bootloader, which uses RAM of its own: a mode request left at
 * the bottom of RAM (0x20000F64) did not survive one. The firmware's RAM ends 8 KB early, and
 * the request is kept at that end (__boot_memory), clear of the bottom and of a stack at the top.
 *
 * An update erases 0x08000 to 0x7CFFF, what the firmware saves included. The saved records wait
 * in RAM meanwhile (__record_carry, the library's nrf52 record_hal.c): 18 KB under the stack,
 * which the firmware's own memory must stay below.
 */

SEARCH_DIR(.)
GROUP(-lgcc -lc)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00008000, LENGTH = 0x68000
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x1E000
}

PROVIDE(__boot_memory = ORIGIN(RAM) + LENGTH(RAM));
/* Bench builds keep their log in the 4 KB after it (utilities/bench_log.h) */
PROVIDE(__bench_log_keep = __boot_memory + 16);

INCLUDE "nrf_common.ld"

/* The global settings (3.9 KB) and four profiles (3.1 KB each), back to back */
PROVIDE(__record_carry_size = 0x4800);
PROVIDE(__record_carry = __StackLimit - __record_carry_size);
ASSERT(__HeapLimit <= __record_carry, "the firmware's memory reaches the records kept over an update")
