/*
 * lfs_plat.c
 *
 *  Created on: 1. lis 2018.
 *      Author: teovu
 */
#include <W25Q256.h>
#include "lfs.h"
#include "define.h"
#include "strukture.h"
#include <string.h>

uint8_t lfs_read_buf[256];
uint8_t lfs_prog_buf[256];
uint8_t lfs_lookahead_buf[32];	// 128/8=16
uint8_t lfs_file_buf[256];

int block_device_read(const struct lfs_config *c, lfs_block_t block,
	lfs_off_t off, void *buffer, lfs_size_t size)
    {
    CMD_READ((block * c->block_size + off), (uint8_t*) buffer, size);

    //W25X_Read((uint8_t*) buffer, (block * c->block_size + off), size);
    return 0;
    }

int block_device_prog(const struct lfs_config *c, lfs_block_t block,
	lfs_off_t off, const void *buffer, lfs_size_t size)
    {
    //W25X_Write_NoCheck((uint8_t*) buffer, (block * c->block_size + off), size);
    CMD_PP((block * c->block_size + off), (uint8_t*) buffer, size);
    return 0;
    }

int block_device_erase(const struct lfs_config *c, lfs_block_t block)
    {
    //W25X_Erase_Sector(block * c->block_size);
    CMD_SE(block * c->block_size);
    return 0;
    }

int block_device_sync(const struct lfs_config *c)
    {
    return 0;
    }



void LFS_Config(void)
    {
    // block device operations
    wtcfg.read = block_device_read;
    wtcfg.prog = block_device_prog;
    wtcfg.erase = block_device_erase;
    wtcfg.sync = block_device_sync;

    // block device configuration
    wtcfg.read_size = 256;
    wtcfg.prog_size = 256;
    wtcfg.block_size = LFS_BLOCK_SIZE;
    wtcfg.block_count = LFS_BLOCKS_NUM;
    wtcfg.lookahead_size = 256;
    wtcfg.cache_size = 256;
    wtcfg.block_cycles=1000;

    wt_cfg.buffer = &lfsBuf1[0];
    http_cfg.buffer = &lfsBuf2[0];
    tft_cfg.buffer = &lfsBuf3[0];
    wt_cfg.attr_count = 0;
    tft_cfg.attr_count = 0;
    http_cfg.attr_count = 0;



   /* wtcfg.read_buffer = lfs_read_buf;
    wtcfg.prog_buffer = lfs_prog_buf;
    wtcfg.lookahead_buffer = lfs_lookahead_buf;*/
    //wtcfg.file_buffer = lfs_file_buf;
    }


void get_filetime(struct _filetime *ftime)
{
RTC_TimeTypeDef RTC_Time;
RTC_DateTypeDef RTC_Date;
RTC_Time.RTC_H12=RTC_HourFormat_24;
RTC_GetTime(RTC_Format_BIN, &RTC_Time);
RTC_GetDate(RTC_Format_BIN, &RTC_Date);

ftime->year=RTC_Date.RTC_Year+2000;
ftime->month=RTC_Date.RTC_Month;
ftime->date=RTC_Date.RTC_Date;
ftime->hour=RTC_Time.RTC_Hours;
ftime->min=RTC_Time.RTC_Minutes;
ftime->sec=RTC_Time.RTC_Seconds;
}


