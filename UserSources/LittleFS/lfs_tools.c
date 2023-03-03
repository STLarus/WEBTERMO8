/*
 * lfs_tools.h
 *
 *  Created on: 2. lis 2018.
 *      Author: teovu
 */

#include "lfs.h"
#include "strukture.h"

#include <string.h>
#include <stdio.h>

extern char str_fname[];
extern lfs_t lfs;
lfs_file_t lfsfile;

/**
 *
 * @param p
 * @param block
 * @return
 */
static int _traverse_df_cb(void *p, lfs_block_t block)
    {
    uint32_t *nb = p;
    *nb += 1;
    return 0;
    }

/** \brief kreira log fajl i na pocÂ�etak dodaje header
 *
 * \param   fname....ime fajla u kojeg se dodaje zapis
 * \param   ibuf.....pointer na buffer koji sadrÅ¾i zapis koji se dodaje
 * \return  za sada nista
 *
 */

uint8_t lfs_create(char *fname, char *ibuf)
    {
    int rtn;
    uint32_t fattime;


    rtn = lfs_file_open(&wtlfs, &lfsfile, fname, LFS_O_RDONLY);
    if (rtn < 0)
	{ //fajl ne postoji
	//rtn = lfs_file_close(&wtlfs, &lfsfile); // za svaki slučaj
	rtn = lfs_file_open(&wtlfs, &lfsfile, fname, LFS_O_RDWR | LFS_O_CREAT);
	}
    else
	{ //fajl postoji, zatvori i izađi
	rtn = lfs_file_close(&wtlfs, &lfsfile);
	return 1;
	}
    rtn = lfs_file_write(&wtlfs, &lfsfile, ibuf, strlen(ibuf));
    get_filetime(&filetime);
    rtn = lfs_setattr(&wtlfs, fname, ATTR_TIMESTAMP, &fattime, 4);
    rtn = lfs_file_sync(&wtlfs, &lfsfile);
    rtn = lfs_file_close(&wtlfs, &lfsfile);
    return 1;
    }/***** lfs_create() *****/

/** \brief dodaje zapis na kraj fajla
 *
 * \param   fname....ime fajla u kojeg se dodaje zapis
 * \param   ibuf.....pointer na buffer koji sadrÄ¹Å¾i zapis koji se dodaje
 * \param   length.....broj bajotova koji se upisuju na kraj fajla
 * \return  za sada nista
 *
 */
uint8_t lfs_append(char *fname, char *ibuf,uint16_t length)
    {
    int rtn;


    rtn = lfs_file_open(&wtlfs, &lfsfile, fname, LFS_O_WRONLY);
    if (rtn < 0)
	{
	rtn = lfs_file_close(&wtlfs,&lfsfile);
	return 0;
	}
    lfs_file_seek(&wtlfs,&lfsfile, 0, LFS_SEEK_END);
    rtn = lfs_file_write(&wtlfs, &lfsfile, ibuf, length);

    rtn = lfs_file_sync(&wtlfs, &lfsfile);
    get_filetime(&filetime);
    rtn = lfs_setattr(&wtlfs, fname, ATTR_TIMESTAMP, &filetime,
	    sizeof(filetime));
    rtn = lfs_file_close(&wtlfs, &lfsfile);
    return 1;

    }/***** lfs_append() *****/

/** \brief CÂ�ita podatak iz fajla sa zadane pozicije u fajlu
 *
 * \param   fname....ime fajla u kojeg se dodaje zapis
 * \param   ibuf.....pointer na buffer u koji se upisuje pročitani podatak
 * \param   startpos.....pozicija u fajlu sa koje starta Ã„Â�itanje
 * \param   length.....broj bajotova koji se Ã„Â�ita iz fajla
 * \return  broj pročitanih bajtova
 *
 */
int32_t lfs_read(char *fname, uint8_t *ibuf, lfs_soff_t startpos,
	uint32_t length)
    {
    int rtn;
    int32_t filelen;


    rtn = lfs_file_open(&wtlfs, &lfsfile, fname, LFS_O_RDONLY);
    if (rtn < 0)
	{
	rtn = lfs_file_close(&wtlfs, &lfsfile);
	return 0;
	}
    lfs_file_seek(&wtlfs, &lfsfile, startpos, LFS_SEEK_SET);

    filelen = lfs_file_read(&wtlfs, &lfsfile, ibuf, length);

    rtn = lfs_file_sync(&wtlfs, &lfsfile);
    rtn = lfs_file_close(&wtlfs, &lfsfile);
    return filelen;
    }/*****lfs_read() *****/

