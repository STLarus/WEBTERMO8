#include <stm32f4xx.h>
#include <stm32f4xx_i2c.h>

#define SLAVE_ADDRESS 0x50      //A0 je prava adresa, ovo je pomaknuto za 1 u desno
#define I2C_PORT    I2C1




/***************************************************************************/
/** * @brief  Ispituje da li je EEPROM slobodan za daljnje operacije
******************************************************************************/

void EE_WaitEepromStandbyState(void)
    {
    __IO uint16_t SR1_Tmp = 0;
    do
        {
        I2C_GenerateSTART(I2C_PORT, ENABLE);
        SR1_Tmp = I2C_ReadRegister(I2C_PORT, I2C_Register_SR1);
        I2C_Send7bitAddress(I2C_PORT, (SLAVE_ADDRESS<<1), I2C_Direction_Transmitter);
        }
    while(!(I2C_ReadRegister(I2C_PORT, I2C_Register_SR1) & 0x0002));
    I2C_ClearFlag(I2C_PORT, I2C_FLAG_AF);
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    }/**** EE_WaitEepromStandbyState() ****/



/** \brief  Ă�ita niz bajtova iz memorije
 *
 * \param   addr, adreasa u memoriji sa koje se pocimlje citati
 * \param   ptr, pointer na array u koji se upisuju procitani podaci
 * \param   size, broj bajtova koji se cita
 * \return  None
 *
 */
void EEread(uint16_t addr, uint8_t *ptr, int size)
    {
    uint32_t retdata;
    uint16_t cnt;
    EE_WaitEepromStandbyState();
    while(I2C_GetFlagStatus(I2C_PORT, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_PORT, (SLAVE_ADDRESS<<1), I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    //adresa memorijske lokacije
    I2C_SendData(I2C_PORT,(uint8_t)(addr>>8));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(I2C_PORT,(uint8_t)(addr));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    //ponovni start
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_PORT, (SLAVE_ADDRESS<<1), I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    //ÄŤitanje podatka
    I2C_AcknowledgeConfig(I2C_PORT, ENABLE);
    for(cnt=0; cnt<(size-1); cnt++)
        {
        while( !I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED) );
        *(ptr+cnt) = I2C_ReceiveData(I2C_PORT);
        }
    //zadnji bajt koji se ÄŤita
    I2C_AcknowledgeConfig(I2C_PORT, DISABLE);
    while( !I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    *(ptr+size-1)= I2C_ReceiveData(I2C_PORT);
    //stop
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    return retdata;
    }/**** EEread() ****/


/** \brief  Ă�ita jedan bajt iz memorije eeproma
 *
 * \param   location, adreasa u memoriji sa koje se cita podatak
 * \return  procitani podatak
 *
 */
uint8_t EEread_8( uint16_t addr)
    {
    EE_WaitEepromStandbyState();
    while(I2C_GetFlagStatus(I2C_PORT, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_PORT, (SLAVE_ADDRESS<<1), I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    //adresa memorijske lokacije
    I2C_SendData(I2C_PORT,(uint8_t)(addr>>8));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(I2C_PORT,(uint8_t)(addr));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    //ponovni start
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_PORT, (SLAVE_ADDRESS<<1), I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    I2C_AcknowledgeConfig(I2C_PORT, DISABLE);
    //ÄŤitanje podatka
    while( !I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    uint8_t retval = I2C_ReceiveData(I2C_PORT);
    //stop
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    return (retval);
    } /**** EEread_8() ****/


/** \brief  Ă�ita 2 bajta iz memorije (integer)
 *
 * \param   addr,adreasa u memoriji sa koje se cita integer
 * \return  procitani integer
 *
 */
uint16_t EEread_16( uint16_t addr)
    {
    uint8_t byte1,byte2;
    EE_WaitEepromStandbyState();
    while(I2C_GetFlagStatus(I2C_PORT, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_PORT, (SLAVE_ADDRESS<<1), I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    //adresa memorijske lokacije
    I2C_SendData(I2C_PORT,(uint8_t)(addr>>8));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(I2C_PORT,(uint8_t)(addr));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    //ponovni start
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_PORT, (SLAVE_ADDRESS<<1), I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    //ÄŤitanje podatka
    I2C_AcknowledgeConfig(I2C_PORT, ENABLE);
    while( !I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    byte1 = I2C_ReceiveData(I2C_PORT);

//zadnji bajt koji se ÄŤita
    I2C_AcknowledgeConfig(I2C_PORT, DISABLE);
    while( !I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    byte2 = I2C_ReceiveData(I2C_PORT);
    //stop
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    return ((byte1 << 8) + byte2);
    } /**** EEread_16() ****/



/** \brief  Ă�ita 4 bajta iz memorije (integer)
 *
 * \param   addr,adreasa u memoriji sa koje se cita long
 * \return  procitani long
 *
 */
uint32_t EEread_32( uint16_t addr)
    {
    uint32_t retdata=0;
    EE_WaitEepromStandbyState();
    while(I2C_GetFlagStatus(I2C_PORT, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_PORT, (SLAVE_ADDRESS<<1), I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    //adresa memorijske lokacije
    I2C_SendData(I2C_PORT,(uint8_t)(addr>>8));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(I2C_PORT,(uint8_t)(addr));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    //ponovni start
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_PORT, (SLAVE_ADDRESS<<1), I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    //ÄŤitanje podatka
    I2C_AcknowledgeConfig(I2C_PORT, ENABLE);
    while( !I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    retdata = I2C_ReceiveData(I2C_PORT);
    retdata=retdata<<8;
    while( !I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    retdata+= I2C_ReceiveData(I2C_PORT);
    retdata=retdata<<8;
    while( !I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    retdata+= I2C_ReceiveData(I2C_PORT);
    retdata=retdata<<8;

//zadnji bajt koji se ÄŤita
    I2C_AcknowledgeConfig(I2C_PORT, DISABLE);
    while( !I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    retdata += I2C_ReceiveData(I2C_PORT);
    //stop
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    return retdata;
    } /**** EEread_32() ****/





/** \brief  Upisuje u memoriju niz bajtova
 *
 * \param   addr,   adreasa u memoriji na koju se upisuje bajt
 * \param   ptr,    pointer na array koji se upisuje
 * \param   size,   broj bajtova koji se upisuju
 * \return  None
 *
 */
// TODO (Teo#1#): N A P O M  E N A\
Ovaj drajver ne vodi racuna o pocetku i kraju stranice\
Sve sto se upisuje na ovaj nacin treba stati u jednu stranicu\
od 64 Bajta


void EEwrite(uint16_t addr , uint8_t *ptr, int size)
    {
    uint16_t cnt;
    EE_WaitEepromStandbyState();
    while(I2C_GetFlagStatus(I2C_PORT, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_PORT, (SLAVE_ADDRESS<<1), I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    //adresa memorijske lokacije
    I2C_SendData(I2C_PORT,(uint8_t)(addr>>8));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(I2C_PORT,(uint8_t)(addr));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    //podatak koji se upisuje
    for(cnt=0; cnt<size; cnt++)
        {
        I2C_SendData(I2C_PORT, *(ptr+cnt));
        while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
        }


    //stop
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    return (0);
    }/**** EEwrite() ****/


/** \brief Upisuje jedan bajt u memoriju
 *
 * \param location, adreasa u memoriji na koju se upisuje bajt
 * \param evar, podatak koji se upisuej u FLASH
 * \return None
 *
 */
void EEwrite_8( uint16_t addr, uint8_t evar)
    {
    EE_WaitEepromStandbyState();
    while(I2C_GetFlagStatus(I2C_PORT, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_PORT, (SLAVE_ADDRESS<<1), I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    //adresa memorijske lokacije
    I2C_SendData(I2C_PORT,(uint8_t)(addr>>8));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(I2C_PORT,(uint8_t)(addr));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    //podatak koji se upisuje
    I2C_SendData(I2C_PORT, evar);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    //stop
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    return (0);
    } /**** EEwrite_8() ****/


/** \brief Upisuje dva bajta u memoriju
 *
 * \param location, adreasa u memoriji na koju se upisuje bajt
 * \param evar, podatak koji se upisuej u FLASH
 * \return None
 *
 */
void EEwrite_16( uint16_t addr, uint16_t evar)
    {
    EE_WaitEepromStandbyState();
    while(I2C_GetFlagStatus(I2C_PORT, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_PORT, (SLAVE_ADDRESS<<1), I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    //adresa memorijske lokacije
    I2C_SendData(I2C_PORT,(uint8_t)(addr>>8));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(I2C_PORT,(uint8_t)(addr));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    //podatak koji se upisuje
    I2C_SendData(I2C_PORT, (uint8_t)(evar>>8));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(I2C_PORT, (uint8_t)evar);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    //stop
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    return (0);
    } /**** EEwrite_16() ****/


/** \brief Upisuje Ă¨etiri bajta u memoriju
 *
 * \param location, adreasa u memoriji na koju se upisuje bajt
 * \param evar, podatak koji se upisuej u FLASH
 * \return None
 *
 */
void EEwrite_32( uint16_t addr, uint32_t evar)
    {
    EE_WaitEepromStandbyState();
    while(I2C_GetFlagStatus(I2C_PORT, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C_PORT, ENABLE);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_PORT, (SLAVE_ADDRESS<<1), I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    //adresa memorijske lokacije
    I2C_SendData(I2C_PORT,(uint8_t)(addr>>8));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(I2C_PORT,(uint8_t)(addr));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    //podatak koji se upisuje
    I2C_SendData(I2C_PORT, (uint8_t)(evar>>24));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(I2C_PORT, (uint8_t)(evar>>16));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(I2C_PORT, (uint8_t)(evar>>8));
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_SendData(I2C_PORT, (uint8_t)evar);
    while(!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    //stop
    I2C_GenerateSTOP(I2C_PORT, ENABLE);
    return (0);
    } /**** EEwrite_32() ****/
