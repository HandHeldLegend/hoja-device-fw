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
 */

SEARCH_DIR(.)
GROUP(-lgcc -lc)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00008000, LENGTH = 0x68000
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x1FFF0
}

INCLUDE "nrf_common.ld"
