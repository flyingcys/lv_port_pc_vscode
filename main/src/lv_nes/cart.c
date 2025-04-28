#include "cart.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lvgl/lvgl.h"

Cartridge *cart_init(void)
{
	Cartridge *cart = malloc(sizeof(Cartridge));
	if (!cart)
	{
		LV_LOG_ERROR("Failed to allocate memory for Cartridge");
	}
	return cart;
}

/* iNES format */
int load_cart(Cartridge *cart, const char *filename, Cpu6502 *cpu, Ppu2A03 *ppu)
{
	uint8_t header[16];
	unsigned trainer;
	uint8_t mapper;
	uint32_t file_size;

	lv_fs_file_t rom;
	uint32_t rn;

	// FILE* &rom = fopen(filename, "rb");
	lv_fs_res_t res = lv_fs_open(&rom, filename, LV_FS_MODE_RD);

	/* Error Detection */
	if (res != LV_FS_RES_OK)
	{
		LV_LOG_ERROR("Error: couldn't open file.");
		return 8;
	}

	/* Getting filesize */
	lv_fs_seek(&rom, 0L, LV_FS_SEEK_END);
	lv_fs_tell(&rom, &file_size);
	if (file_size < 0x4010)
	{
		LV_LOG_ERROR("Error: input file is too small.");
		lv_fs_close(&rom);
		return 8;
	}

	lv_fs_close(&rom);
	lv_fs_open(&rom, filename, LV_FS_MODE_RD | LV_FS_MODE_WR);

	/* loading first 16 bytes of .nes file into header */
	lv_fs_read(&rom, &header, 16, &rn);
	if (rn != 16)
	{
		LV_LOG_ERROR("Error: unable to read ROM header.");
		lv_fs_close(&rom);
		return 8;
	}
	printf("%s", header);

	if (memcmp(header, "NES\x1A", 4))
	{
		LV_LOG_ERROR("Error: invalid nes file.");
		lv_fs_close(&rom);
		return 8;
	}

	/* parsing header */
	cart->prg_rom.size = 16 * (KiB)*header[4];
	cart->prg_ram.size = 8 * (KiB)*header[8];
	cart->chr.rom_size = 8 * (KiB)*header[5]; // Pattern table data (if any)
	cart->chr.ram_size = 8 * (KiB) * !header[5];
	mapper = (header[7] & 0xF0) | ((header[6] & 0xF0) >> 4);
	cpu->cpu_mapper_io->mapper_number = mapper;

	/* Flags 6 */
	if (!(header[6] & 0x08))
	{
		if (header[6] & 0x01)
		{
			ppu->mirroring = 1;
		}
		else
		{
			ppu->mirroring = 0;
		}
	}
	else
	{
		ppu->mirroring = 4;
	}

	if (header[6] & 0x04)
	{
		trainer = 512;
	}
	else
	{
		trainer = 0;
	}

	/* Flags 9 */
	if (header[9] & 0x01)
	{
		cart->video_mode = PAL;
	}
	else
	{
		cart->video_mode = NTSC;
	}

	/* Loading data into PRG_ROM */
	lv_fs_seek(&rom, trainer, SEEK_CUR); /* fseek has gone past header now needs to skip trainer */
	cart->prg_rom.data = malloc(cart->prg_rom.size);
	if (!cart->prg_rom.data)
	{
		lv_fs_close(&rom);
		return 8;
	}
	// lv_fs_read(cart->prg_rom.data, 1, cart->prg_rom.size, &rom);
	lv_fs_read(&rom, cart->prg_rom.data, cart->prg_rom.size, NULL);

	/* loading data into chr_rom */
	unsigned chr_size = cart->chr.rom_size ? cart->chr.rom_size : cart->chr.ram_size;
	if (chr_size)
	{
		cart->chr.data = malloc(chr_size);
		if (!cart->chr.data)
		{
			free(cart->prg_rom.data);
			lv_fs_close(&rom);
			return 8;
		}
		// lv_fs_read(cart->chr.data, 1, chr_size, &rom);
		lv_fs_read(&rom, cart->chr.data, chr_size, NULL);
	}

	lv_fs_close(&rom);

	/* Mapper select */
	init_mapper(cart, cpu, ppu);

	return 0;
}
