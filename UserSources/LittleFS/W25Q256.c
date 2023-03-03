#include "stm32f4xx_conf.h"
#include "drajveri.h"
#include "define.h"
#include <string.h>
#include <W25Q256.h>

#define	RXNE	0x01
#define	TXE		0x02
#define	BSY		0x80

// status register
#define    FLASH_WIP_MASK         0x01
#define    FLASH_LDSO_MASK        0x02
#define    FLASH_QE_MASK          0x40

#define    PageProgramCycleTime	1000
#define    SectorEraseCycleTime 1000



uint8_t memBuf[512];
uint8_t diskBuf[8200];

void sFLASH_LowLevel_Init(void)
    {
    GPIO_InitTypeDef GPIO_InitStructure;

    /*!< Enable the SPI clock */
    sFLASH_SPI_CLK_INIT(sFLASH_SPI_CLK, ENABLE);

    /*!< Enable GPIO clocks */
    RCC_AHB1PeriphClockCmd(sFLASH_SPI_SCK_GPIO_CLK | sFLASH_SPI_MISO_GPIO_CLK |
    sFLASH_SPI_MOSI_GPIO_CLK | sFLASH_CS_GPIO_CLK, ENABLE);

    /*!< SPI pins configuration *************************************************/

    /*!< Connect SPI pins to AF5 */
    GPIO_PinAFConfig(sFLASH_SPI_SCK_GPIO_PORT, sFLASH_SPI_SCK_SOURCE,
    sFLASH_SPI_SCK_AF);
    GPIO_PinAFConfig(sFLASH_SPI_MISO_GPIO_PORT, sFLASH_SPI_MISO_SOURCE,
    sFLASH_SPI_MISO_AF);
    GPIO_PinAFConfig(sFLASH_SPI_MOSI_GPIO_PORT, sFLASH_SPI_MOSI_SOURCE,
    sFLASH_SPI_MOSI_AF);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

    /*!< SPI SCK pin configuration */
    GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_SCK_PIN;
    GPIO_Init(sFLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

    /*!< SPI MOSI pin configuration */
    GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_MOSI_PIN;
    GPIO_Init(sFLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

    /*!< SPI MISO pin configuration */
    GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_MISO_PIN;
    GPIO_Init(sFLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure sFLASH Card CS pin in output pushpull mode ********************/
    GPIO_InitStructure.GPIO_Pin = sFLASH_CS_PIN | sFLASH_HOLD_PIN
	    | sFLASH_RES_PIN | sFLASH_VP_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(sFLASH_CS_GPIO_PORT, &GPIO_InitStructure);

    /*!< inicijalizacija kontrolnih linija HOLD,RES i VP */
    GPIO_InitStructure.GPIO_Pin = sFLASH_HOLD_PIN | sFLASH_RES_PIN
	    | sFLASH_VP_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    }

/*
 * Function:       Initial_Spi
 * Arguments:      None
 * Description:    Initial spi flash state and wait flash warm-up
 *                 (enable read/write).
 * Return Message: None
 */
void MX25_Spi_Init(void)
    {
    SPI_InitTypeDef SPI_InitStructure;

    sFLASH_LowLevel_Init();

    /*!< SPI configuration */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    //SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    MX25_CS_HIGH();
    MX25_VP_LOW();
    MX25_RES_HIGH();
    MX25_HOLD_HIGH();

    delay_ms(1);
    MX25_RES_LOW();	//reset flash memorije
    delay_ms(1);
    MX25_RES_HIGH();

    SPI_Cmd(SPI1, ENABLE);
    }

/****************************************************************
 u8 MX_SIO( u8)
 ****************************************************************/
u8 MX_SIO(u8 sdata)
    {
    while (!(MXSR & TXE))
	;
    MXDR = sdata;
    while (!(MXSR & RXNE))
	;
    return MXDR;
    }/***** MX_SIO() *****/

/****************************************************************
 BOOL IsReady( )
 ****************************************************************/
BOOL IsReady()
    {
    u8 ready = FALSE;
    u8 dummy;

    while (ready == FALSE)
	{
	MX25_CS_LOW();
	MX_SIO(FLASH_CMD_RDSR);
	dummy = MX_SIO(0);
	MX25_CS_HIGH();
	if ((dummy & FLASH_WIP_MASK) == FLASH_WIP_MASK)
	    ready = FALSE;	//WRITE IN PROGRESS bit je aktivan
	else
	    ready = TRUE;
	}

// o ovu petlju treba ubaciti i neku vremensku čekalicu ako se zaglavi koja bi napravila reset
    return ready;
    }/***** IsReady() *****/

/****************************************************************
 unsigned char CMD_RDSCUR(void)
 Read security register
 ****************************************************************/
unsigned char CMD_RDSCUR(void)
    {
    u8 retval;
    MX25_CS_LOW();
    MX_SIO(FLASH_CMD_RDSCUR);
    retval = MX_SIO(0);
    MX25_CS_HIGH();
    return retval;
    }/***** CMD_RDSCUR() *****/

/****************************************************************
 unsigned char CMD_RDSR(void)
 Read status register
 ****************************************************************/
unsigned char CMD_RDSR(void)
    {
    u8 retval;
    MX25_CS_LOW();
    MX_SIO(FLASH_CMD_RDSR);
    retval = MX_SIO(0);
    MX25_CS_HIGH();
    return retval;
    }/***** CMD_RDSR() *****/

/* Function:      WaitFlashReady
 * Arguments:      ExpectTime, expected time-out value of flash operations.
 *                 No use at non-synchronous IO mode.
 * Description:    Synchronous IO
 *                 If flash is ready return TRUE.
 *                 If flash is time-out return FALSE.
 *                 Non-synchronous IO:
 *                 Always return TRUE
 * Return Message: TRUE, FALSE
 */
BOOL WaitFlashReady(uint32 ExpectTime)

    {

    uint32 temp = 0;

    while (IsReady() == FALSE)

	{
	delay_ms(1);
	if (temp > ExpectTime)

	    return FALSE;

	temp = temp + 1;

	}

    return TRUE;

    }

/****************************************************************
 BOOL IsFlash4Byte( void )
 ****************************************************************/
BOOL IsFlash4Byte(void)
    {
    uint8 gDataBuffer;
    gDataBuffer = CMD_RDSCUR();
    if ((gDataBuffer & FLASH_4BYTE_MASK) == FLASH_4BYTE_MASK)
	return TRUE;
    else
	return FALSE;
    }/***** IsFlash4Byte() *****/


/****************************************************************
 ReturnMsg CMD_WREN( void )
 ****************************************************************/
ReturnMsg CMD_WREN(void)
    {
    MX25_CS_LOW();
    MX_SIO(FLASH_CMD_WREN);
    MX25_CS_HIGH();
    return FlashOperationSuccess;
    }/***** CMD_WREN() *****/

/*
 * Function:       CMD_PP
 * Arguments:      flash_address, 32 bit flash memory address
 *                 source_address, buffer address of source data to program
 *                 byte_length, byte length of data to programm
 * Description:    The PP instruction is for programming
 *                 the memory to be "0".
 *                 The device only accept the last 256 byte ( one page ) to program.
 *                 If the page address ( flash_address[7:0] ) reach 0xFF, it will
 *                 program next at 0x00 of the same page.
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_PP(uint32 flash_address, uint8 *source_address,
	uint32 byte_length)
    {
    uint32 index;
    u8 status;
//ovdje treba čekati koliko je potrebno da se sektor izbriše?????
    if (IsReady() == FALSE)
	return FlashIsBusy;
    CMD_WREN();
    MX25_CS_LOW();
    MX_SIO(FLASH_CMD_PP);
    MX_SIO(flash_address >> 24);
    MX_SIO(flash_address >> 16);
    MX_SIO(flash_address >> 8);
    MX_SIO(flash_address);
    for (index = 0; index < byte_length; index++)
	MX_SIO(*(source_address + index));
    status = FALSE;
    MX25_CS_HIGH();
    while (status == FALSE)
	status = IsReady();	//ovo triba uštimati
    return FlashOperationSuccess;
    }/***** CMD_PP() *****/

/*
 * Function:       CMD_SE
 * Arguments:      flash_address, 32 bit flash memory address
 * Description:    The SE instruction is for erasing the data
 *                 of the chosen sector (4KB) to be "1".
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_SE(uint32 flash_address)
    {
    if (IsReady() == FALSE)
	return FlashIsBusy;
    CMD_WREN();
    MX25_CS_LOW();
    MX_SIO(FLASH_CMD_SE);
    MX_SIO(flash_address >> 24);
    MX_SIO(flash_address >> 16);
    MX_SIO(flash_address >> 8);
    MX_SIO(flash_address);
    MX25_CS_HIGH();
    if (WaitFlashReady( SectorEraseCycleTime))	//ovo treba riješiti
	return FlashOperationSuccess;
    else
	return FlashTimeOut;
    }/***** CMD_SE() *****/


/*
 * Function:       CMD_CE
 * Arguments:      None
 * Description:    Brisanje cijele memorije
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_CE(void)
    {
    if (IsReady() == FALSE)
	return FlashIsBusy;
    CMD_WREN();
    MX25_CS_LOW();
    MX_SIO(FLASH_CMD_CE);
    MX25_CS_HIGH();
    if (WaitFlashReady( SectorEraseCycleTime))	//ovo treba riješiti
	return FlashOperationSuccess;
    else
	return FlashTimeOut;
    }/***** CMD_CE() *****/





/*
 * Function:       CMD_READ
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 * Description:    The READ instruction is for reading data out.
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */

ReturnMsg CMD_READ(uint32 flash_address, uint8 *target_address,
	uint32 byte_length)
    {
    uint32 index;
    u8 jebiga;

    MX25_CS_LOW();
    MX_SIO(FLASH_CMD_READ);
    MX_SIO(flash_address >> 24);
    MX_SIO(flash_address >> 16);
    MX_SIO(flash_address >> 8);
    MX_SIO(flash_address);
    for (index = 0; index < byte_length; index++)
	{
	jebiga = MX_SIO(0);
	*(target_address + index) = jebiga;
	}
    MX25_CS_HIGH();
    return FlashOperationSuccess;
    }/***** CMD_READ() *****/

//============================================
// rutine povezane sa parametrima za programiranje 

extern BYTE diskBuf[];


void ChipErase(void)
    {
    CMD_CE();
    }

#define adresa1	0x1000000
#define adresa2	0x1200000
#define adresa3	0x1700000
#define adresa4	0x1A00000

#define adresa5	0x1750000
#define adresa6	0x1A10000

void MX25test(void)
{
unsigned char jure[6],mare[600];
unsigned int cnt;
IsReady( );
for(cnt=0;cnt<5;cnt++)
	jure[cnt]=0;
MX25_CS_LOW();
MX_SIO(FLASH_CMD_RDID);
jure[0]=MX_SIO(0xFF);
jure[1]=MX_SIO(0xFF);
jure[2]=MX_SIO(0xFF);
MX25_CS_HIGH();

CMD_SE(adresa1);
CMD_SE(adresa2);
CMD_SE(adresa3);
CMD_SE(adresa4);
CMD_SE(adresa5);
CMD_SE(adresa5);

for(cnt=0;cnt<512;cnt++)
	mare[cnt]=cnt;
CMD_PP(adresa1,&mare[0],512);

for(cnt=0;cnt<512;cnt++)
	mare[cnt]=2;
CMD_PP(adresa2,&mare[0],512);

for(cnt=0;cnt<512;cnt++)
	mare[cnt]=3;
CMD_PP(adresa3,&mare[0],512);

for(cnt=0;cnt<512;cnt++)
	mare[cnt]=4;
CMD_PP(adresa4,&mare[0],512);

for(cnt=0;cnt<512;cnt++)
	mare[cnt]=13;
CMD_PP(adresa5,&mare[0],512);

for(cnt=0;cnt<512;cnt++)
	mare[cnt]=14;
CMD_PP(adresa6,&mare[0],512);

CMD_READ(adresa1,&mare[0],512);
CMD_READ(adresa2,&mare[0],512);
CMD_READ(adresa3,&mare[0],512);
CMD_READ(adresa4,&mare[0],512);
CMD_READ(adresa5,&mare[0],512);
CMD_READ(adresa6,&mare[0],512);

}


