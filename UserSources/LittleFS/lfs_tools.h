/*
 * lfs_tools.h
 *
 *  Created on: 2. lis 2018.
 *      Author: teovu
 */

#include "lfs.h"

#include <string.h>
#include <stdio.h>

char str_fname[25];
extern lfs_t lfs;
extern lfs_file_t file;

/** \brief kreira log fajl i na pocÂ�etak dodaje header
 *
 * \param   fname....ime fajla u kojeg se dodaje zapis
 * \param   ibuf.....pointer na buffer koji sadrÅ¾i zapis koji se dodaje
 * \return  za sada nista
 *
 */
extern uint8_t lfs_create(char *fname, char *ibuf);
/** \brief dodaje zapis na kraj fajla
 *
 * \param   fname....ime fajla u kojeg se dodaje zapis
 * \param   ibuf.....pointer na buffer koji sadrÄ¹Å¾i zapis koji se dodaje
 * \param   length.....broj bajotova koji se upisuju na kraj fajla
 * \return  za sada nista
 *
 */
uint8_t lfs_append(char *fname, char *ibuf, uint16_t length);


/** \brief CÂ�ita podatak iz fajla sa zadane pozicije u fajlu
 *
 * \param   fname....ime fajla u kojeg se dodaje zapis
 * \param   ibuf.....pointer na buffer koji sadrÄ¹Å¾i zapis koji se dodaje
 * \param   startpos.....pozicija u fajlu sa koje starta Ã„Â�itanje
 * \param   length.....broj bajotova koji se Ã„Â�ita iz fajla
 * \return  za sada nista
 *
 */
extern uint8_t lfs_read(char *fname, uint8_t *ibuf, lfs_soff_t startpos, uint32_t length);


