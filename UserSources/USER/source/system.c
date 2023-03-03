#include "datatypes.h"
#include "system.h"
#include "strukture.h"
#include <stdint.h>

#define MAX_PRECISION	(10)

#define AUTHEN_LEN	80	//dužina buffera u koji se sprema uname+pass

UINT32 base_timer; /**< System 1.024 msec timer	*/

UINT8 sleep_mode = 0; /**< Used to store information about power-saving state we're in (if any) */

char base64[128];

/********************************************************************************
 Function:		bufsearch
 Parameters:		UINT8* startadr - start address of given buffer
 UINT16 len - buffer length
 UINT8* str - given searchstring
 Return val:		INT16 - (-1) Not found
 (>=0) Start of matched string from startadr
 Desc:			Seeks given string from given buffer
 *********************************************************************************/

INT16 bufsearch(UINT8* startadr, UINT16 len, UINT8* str)
    {
    UINT16 i;
    INT16 position;
    UINT8 matchesneeded, matchesnow;
    UINT8* target, *key;

    target = startadr;
    position = -1;
    key = str;
    matchesnow = 0;
    matchesneeded = 0;
    while (*key++ != '\0')
	{/* How many matches we need?	*/
	matchesneeded++;/* Break possible deadlock	*/
	if (matchesneeded > 30)
	    return (-1);
	}
    key = str;/* Search for first mark and continue searching if found	*/
    for (i = 0; i < len; i++)
	{
	if (*target == *key)
	    {/* We found matching character		*/
	    matchesnow++;
	    key++;/* Move to next character of key	*/
	    target++;
	    if (matchesnow == 1)
		{
		position = i;/* First character match	*/
		}
	    if (matchesneeded == matchesnow)
		return (position);/* Whole string matched	*/

	    }
	else
	    {
	    if (matchesnow != 0)
		{/* It wasn't a complete match...	*/
		matchesnow = 0;/* Initialize counters and start again	*/
		key = str;
		target = startadr;/* Move to next character of target after previous matching character	*/
		target += position;
		target += 1;
		i = position;
		}
	    else
		target++;/* Just continue searching the first match		*/
	    }
	}
    return (-1);/* No matches found...	*/
    }/***** bufsearch() *****/

/* HexToAscii - Take one byte and return its two ASCII  */
/* values for both nibbles								*/

UINT16 hextoascii(UINT8 c)
    {
    UINT8 ch;
    UINT8 as1;
    UINT8 as2;
    ch = c;
    /* take the char and turn it to ASCII */

    as1 = ch;
    as1 >>= 4;
    as1 &= 0x0F;
    if (as1 < 10)
	as1 += 48;
    else
	as1 += 55;

    as2 = ch;
    as2 &= 0x0F;
    if (as2 < 10)
	as2 += 48;
    else
	as2 += 55;

    return (((UINT16) (as1) << 8) + as2);

    }

/* Convert ASCII character to numerical	*/
/* e.g. '1' -> 0x01, 'A' ->0x0A			*/

UINT8 asciitohex(UINT8 ch)
    {
    if ((ch < 58) && (ch > 47))
	return (ch - 48);

    if ((ch < 71) && (ch > 64))
	return (ch - 55);
    return 0;
    }

void sendchar(char port, char c)
    {
//
//while ( !( USARTC0.STATUS & 0x20) );
//    USARTC0.DATA = c;
////delay_us(100);
    }/***** sendchar()  *****/

/* Debug/String output	*/

void mputs(UINT8 const * msg)
    {
//delay_us(100);
////while( *msg !='\0')
////    {
////	sendchar(0,*msg);       //moja debug funkcija
////	msg++;
////	}
    }

/* Debug/Hex output a number*/
void mputhex(UINT8 nbr)
    {

    UINT16 i;

    i = hextoascii(nbr);
    sendchar(0, i >> 8);
    sendchar(0, (UINT8) i);
    }

/*	Watchdog refresh	*/

void kick_WD(void)
    {
//wdt_reset ();
    }

/*****************************************************************************
 Function:
 WORD Base64Decode(BYTE* cSourceData, WORD wSourceLen,
 BYTE* cDestData, WORD wDestLen)

 Description:
 Decodes a Base-64 array to its literal representation.

 Precondition:
 None

 Parameters:
 cSourceData - Pointer to a string of Base-64 encoded data
 wSourceLen	- Length of the Base-64 source data
 cDestData	- Pointer to write the decoded data
 wDestLen	- Maximum length that can be written to cDestData

 Returns:
 Number of decoded bytes written to cDestData.

 Remarks:

 This function is binary safe and will ignore invalid characters (CR, LF,
 etc).  If cSourceData is equal to cDestData, the data will be converted
 in-place.  If cSourceData is not equal to cDestData, but the regions
 overlap, the behavior is undefined.

 Decoded data is always at least 1/4 smaller than the source data.
 ***************************************************************************/
WORD Base64Decode(BYTE* cSourceData, WORD wSourceLen, BYTE* cDestData,
	WORD wDestLen)
    {
    BYTE i;
    BYTE vByteNumber;
    char bPad;
    WORD wBytesOutput;

    vByteNumber = 0;
    wBytesOutput = 0;

    // Loop over all provided bytes
    while (wSourceLen--)
	{
	// Fetch a Base64 byte and decode it to the original 6 bits
	i = *cSourceData++;
	bPad = (i == '=');
	if (i >= 'A' && i <= 'Z')	// Regular data
	    i -= 'A' - 0;
	else if (i >= 'a' && i <= 'z')
	    i -= 'a' - 26;
	else if (i >= '0' && i <= '9')
	    i -= '0' - 52;
	else if (i == '+' || i == '-')
	    i = 62;
	else if (i == '/' || i == '_')
	    i = 63;
	else
	    // Skip all padding (=) and non-Base64 characters
	    continue;

	// Write the 6 bits to the correct destination location(s)
	if (vByteNumber == 0u)
	    {
	    if (bPad)// Padding here would be illegal, treat it as a non-Base64 chacter and just skip over it
		continue;
	    vByteNumber++;
	    if (wBytesOutput >= wDestLen)
		break;
	    wBytesOutput++;
	    *cDestData = i << 2;
	    }
	else if (vByteNumber == 1u)
	    {
	    vByteNumber++;
	    *cDestData++ |= i >> 4;
	    if (wBytesOutput >= wDestLen)
		break;
	    if (bPad)
		continue;
	    wBytesOutput++;
	    *cDestData = i << 4;
	    }
	else if (vByteNumber == 2u)
	    {
	    vByteNumber++;
	    *cDestData++ |= i >> 2;
	    if (wBytesOutput >= wDestLen)
		break;
	    if (bPad)
		continue;
	    wBytesOutput++;
	    *cDestData = i << 6;
	    }
	else if (vByteNumber == 3u)
	    {
	    vByteNumber = 0;
	    *cDestData++ |= i;
	    }
	}

    return wBytesOutput;
    }

/*****************************************************************************
 Function:
 WORD Base64Encode(BYTE* cSourceData, WORD wSourceLen,
 BYTE* cDestData, WORD wDestLen)

 Description:
 Encodes a binary array to Base-64.

 Precondition:
 None

 Parameters:
 cSourceData - Pointer to a string of binary data
 wSourceLen	- Length of the binary source data
 cDestData	- Pointer to write the Base-64 encoded data
 wSourceLen	- Maximum length that can be written to cDestData

 Returns:
 Number of encoded bytes written to cDestData.  This will always be
 a multiple of 4.

 Remarks:
 Encoding cannot be performed in-place.  If cSourceData overlaps with
 cDestData, the behavior is undefined.

 Encoded data is always at least 1/3 larger than the source data.  It may
 be 1 or 2 bytes larger than that.
 ***************************************************************************/
WORD Base64Encode(BYTE* cSourceData, WORD wSourceLen, BYTE* cDestData,
	WORD wDestLen)
    {
    BYTE i, j;
    BYTE vOutput[4];
    WORD wOutputLen;

    wOutputLen = 0;
    while (wDestLen >= 4u)
	{
	// Start out treating the output as all padding
	vOutput[0] = 0xFF;
	vOutput[1] = 0xFF;
	vOutput[2] = 0xFF;
	vOutput[3] = 0xFF;

	// Get 3 input octets and split them into 4 output hextets (6-bits each)
	if (wSourceLen == 0u)
	    break;
	i = *cSourceData++;
	wSourceLen--;
	vOutput[0] = (i & 0xFC) >> 2;
	vOutput[1] = (i & 0x03) << 4;
	if (wSourceLen)
	    {
	    i = *cSourceData++;
	    wSourceLen--;
	    vOutput[1] |= (i & 0xF0) >> 4;
	    vOutput[2] = (i & 0x0F) << 2;
	    if (wSourceLen)
		{
		i = *cSourceData++;
		wSourceLen--;
		vOutput[2] |= (i & 0xC0) >> 6;
		vOutput[3] = i & 0x3F;
		}
	    }

	// Convert hextets into Base 64 alphabet and store result
	for (i = 0; i < 4u; i++)
	    {
	    j = vOutput[i];

	    if (j <= 25u)
		j += 'A' - 0;
	    else if (j <= 51u)
		j += 'a' - 26;
	    else if (j <= 61u)
		j += '0' - 52;
	    else if (j == 62u)
		j = '+';
	    else if (j == 63u)
		j = '/';
	    else
		// Padding
		j = '=';

	    *cDestData++ = j;
	    }

	// Update counters
	wDestLen -= 4;
	wOutputLen += 4;
	}

    return wOutputLen;
    } /***** Base64Encode() *****/

/****************************************************************
 Èita dva bajta iz HTTP stringa i vraca jedan enkodirani
 *****************************************************************/
unsigned char https_read_encoded2(char *ibuf)
    {
    unsigned char temp, ch;
    temp = *ibuf++;
    if ((temp >= '0') && (temp <= '9'))
	ch = (temp - '0') << 4;
    else
	{
	if ((temp >= 'a') && (temp <= 'f'))
	    ch = (temp - 'a' + 10) << 4;
	else
	    ch = (temp - 'A' + 10) << 4;
	}
    temp = *ibuf++;
    if ((temp >= '0') && (temp <= '9'))
	ch |= (temp - '0');
    else
	{
	if ((temp >= 'a') && (temp <= 'f'))
	    ch |= (temp - 'a' + 10);
	else
	    ch |= (temp - 'A' + 10);
	}
    return ch;
    }/***** https_read_encoded2() *****/

void itoay(char *szBuffer, int x)
    {
    char tmbuf[7];
    int i = 0, n, xx;
    if (x != 0)
	{
	n = x;
	while (n > 0)
	    {
	    xx = n % 10;
	    n = n / 10;
	    tmbuf[i++] = '0' + xx;
	    }
	for (n = 0; n < i; n++)
	    *(szBuffer + n) = tmbuf[i - n - 1];	//radi obrnuti raspored u stringu
	*(szBuffer + i) = 0;
	}
    else
	{				//karakter je nula
	*(szBuffer) = '0';
	*(szBuffer + 1) = 0;

	}
    }

void itoax(char *tmpstr, int num)
    {
//cvt num to 5 ascii chars in tmpstr using repeated subtraction by powers of 10
    char i, dig;
    int pow10[4] =
	{
	10000, 1000, 100, 10
	};
    int pow10i;

    for (i = 0; i < 4; i++)
	{
	dig = 0;
	pow10i = pow10[i];
	while ((num - pow10i) > 0)
	    {
	    num -= pow10i;
	    dig++;
	    }
	*tmpstr++ = dig + 0x30;
	}
    *tmpstr++ = num + 0x30;
    *tmpstr = 0;
    }

///** \brief inicijalizira string authentikacije
// * \param  NONE
// *
// * \return  none
// *
// */
// void initAuth(void)
//{
//char pass_str[40];
//char passtring[40];
//strcpy(pass_str,"jure:mare");
//
//Base64Encode(pass_str,strlen(pass_str), passstring,40);
//}

/** \brief provjera da li prispjeli HTTP zahtjev ima ispravnu autentikaciju
 *
 * \param  istr...........pointer na string u kojemu se nalazi string authentikacije
 *
 * \return  TRUE ili FALSE
 *
 */
uint8_t checkAuthxxxxxx(char * istr)
    {
    char passtring[40];
    uint8_t status = FALSE;
    uint8_t passlength;
    char *endofstring = strchr(istr, 0x0D);   //traži CR na kraju stringa
    passlength = endofstring - istr;
    Base64Encode("jure:mare", strlen("jure:mare"), passtring, 40);
    if (strncmp(istr, passtring, passlength) == 0)
	;
    status = TRUE;
    return status;

    }

/*------------------------------------------------------------------------------
 Funkcija provjerava da li je ispravna autentikacija:
 *inbuf.....pointer na buffer sa primljenim podacima
 return.... 1 authentikacija OK, 0.... error
 ------------------------------------------------------------------------------*/
INT16 checkAuth(char *pbuf)
    {
    char tbuf[80];
    char ubuf[40];
    UINT8 cnt, i = 0;
    WORD decodelen;
    INT16 point1, point2;
    point1 = bufsearch(pbuf, 128, "&");	//traži prvi kontrolni znak
    if (point1)
	point2 = bufsearch(pbuf + point1 + 1, 128, "&");//traži drugi kontrolni znak
    else
	return 0;	//nije našao prvi kontrolni znak
    point2 = point2 + point1 + 1;//ovo radi jer je počeo na adresi pbug+point1
    if (point2 < 0)
	return 0;	//nije našao drugi znak
    decodelen = Base64Decode((pbuf + point1), (point2 - point1), tbuf,
	    AUTHEN_LEN);
    cnt = 0;
    i = 0;
    while (tbuf[cnt] != ':')	//traži username
	{
	ubuf[cnt] = tbuf[cnt];
	cnt++;
	if (cnt > AUTHEN_LEN)
	    return 0;
	}
    ubuf[cnt] = 0;
    if (strcmp(ubuf, username) != 0)
	return 0;
    cnt++;	//preskače dvotočku
    while (cnt < decodelen)	//traži password
	{
	ubuf[i] = tbuf[cnt];
	cnt++;
	i++;
	if (cnt > AUTHEN_LEN)
	    return 0;
	}
    ubuf[i] = 0;
    if (strcmp(ubuf, password) != 0)
	return 0;
    else
	return 1;
    }/***** checkAuth() *****/

static const double rounders[MAX_PRECISION + 1] =
    {
    0.5,				// 0
	    0.05,				// 1
	    0.005,				// 2
	    0.0005,				// 3
	    0.00005,			// 4
	    0.000005,			// 5
	    0.0000005,			// 6
	    0.00000005,			// 7
	    0.000000005,		// 8
	    0.0000000005,		// 9
	    0.00000000005
    // 10
	};

char * my_ftoa(double f, char * buf, int precision)
    {
    char * ptr = buf;
    char * p = ptr;
    char * p1;
    char c;
    long intPart;

    // check precision bounds
    if (precision > MAX_PRECISION)
	precision = MAX_PRECISION;

    // sign stuff
    if (f < 0)
	{
	f = -f;
	*ptr++ = '-';
	}

    if (precision < 0)  // negative precision == automatic precision guess
	{
	if (f < 1.0)
	    precision = 6;
	else if (f < 10.0)
	    precision = 5;
	else if (f < 100.0)
	    precision = 4;
	else if (f < 1000.0)
	    precision = 3;
	else if (f < 10000.0)
	    precision = 2;
	else if (f < 100000.0)
	    precision = 1;
	else
	    precision = 0;
	}

    // round value according the precision
    if (precision)
	f += rounders[precision];

    // integer part...
    intPart = f;
    f -= intPart;

    if (!intPart)
	*ptr++ = '0';
    else
	{
	// save start pointer
	p = ptr;

	// convert (reverse order)
	while (intPart)
	    {
	    *p++ = '0' + intPart % 10;
	    intPart /= 10;
	    }

	// save end pos
	p1 = p;

	// reverse result
	while (p > ptr)
	    {
	    c = *--p;
	    *p = *ptr;
	    *ptr++ = c;
	    }

	// restore end pos
	ptr = p1;
	}

    // decimal part
    if (precision)
	{
	// place decimal point
	*ptr++ = '.';

	// convert
	while (precision--)
	    {
	    f *= 10.0;
	    c = f;
	    *ptr++ = '0' + c;
	    f -= c;
	    }
	}

    // terminating zero
    *ptr = 0;

    return buf;
    }

