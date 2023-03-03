/** 
 *		\li BYTE	- unsigned 8 bit value
 *		\li WORD	- unsigned 16 bit value
 *		\li LWORD 	- unsigned 32 bit value
 *		\li UINT8	- unsigned 8 bit value
 *		\li INT8	- signed 8 bit value
 *		\li UINT16	- unsigned 16 bit value
 *		\li INT16	- signed 16 bit value
 *		\li UINT32	- unsigned 32 bit value
 *		\li INT32	- signed 32 bit value
 */
#ifndef INCLUDE_DATATYPES_H
#define INCLUDE_DATATYPES_H


//#define BYTE 	unsigned char				
//#define WORD 	unsigned short		/*< 16 bit unsigned*/ 
#define LWORD	unsigned long		/*< 32 bit unsigned  */

#define UINT8	unsigned char		/**< 8 bit unsigned */
#define INT8	signed char			/**< 8 bit signed */
#define	UINT16	unsigned short		/**< 16 bit unsigned */
#define INT16	signed short		/**< 16 bit signed */
#define UINT32	unsigned int		/**< 32 bit unsigned */
#define INT32 	signed int			/**< 32 bit signed */

//kopirano iz "integer.h"

/* These types must be 16-bit, 32-bit or larger integer */

//typedef int				INT;
typedef unsigned int	UINT;

/* These types must be 8-bit integer */
typedef signed char		CHAR;
typedef unsigned char	UCHAR;
typedef unsigned char	BYTE;

/* These types must be 16-bit integer */
typedef signed short	SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;
typedef unsigned short	WCHAR;

/* These types must be 32-bit integer */
typedef signed long		LONG;
typedef unsigned long	ULONG;
typedef unsigned long	DWORD;

#define KO        0
#define OK        1

#endif


