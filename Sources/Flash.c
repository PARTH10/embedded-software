/*! @file
 *
 *  @brief This contains the functions in the program to read, write and erase flash.
 *
 *  @author Gideon Kanikevich - 11655899 & Joshua Gonsalves - 11848759
 *  @date 2015-08-14
 */
// new types
#include "types.h"
#include "Flash.h"
#include "MK70F12.h"
#define SECTOR_SIZE 4095
#define FLASH_ARRAY 8

/*! @brief Initializes the flash modules.
 *
 *  @return BOOL - TRUE if the Flash was setup successfully.
 */
BOOL Flash_Init(void)
{
  SIM_SCGC3 = SIM_SCGC3_NFC_MASK;
  return 1;
}

/*! @brief Makes space for a non-volatile variable in the Flash memory.
 *
 *  @param variable is the address of a pointer to a variable that is to be allocated space in Flash memory.
 *  @param size The size, in bytes, of the variable that is to be allocated space in the Flash memory. Valid values are 1, 2 and 4.
 *  @return BOOL - TRUE if the variable was allocated space in the Flash memory.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_AllocateVar(void **variable, const uint8_t size)
{
  // uint32_t *addy = 0x80000;
  uint8_t flashArray[FLASH_ARRAY]; // Define flash array and initialize all elements to zero
  int tempSize = 0;
  uint8_t *dataPtr; // define data pointer
  dataPtr = *variable; // Copy address of variable to dataPtr
  uint8_t temp = 0;
  while (temp < 8)
    {
      if (flashArray[temp] == 0xFF) //if there is an empty space
	{
	  tempSize++;
	}
      if (tempSize == size) //if the number of empty spaces = to the size of the data
	{
	  for (int i = size; i >= 0; i--)
	    {
	      flashArray[i] = dataPtr[i]; // write the data to the array
	    }

	}
      else
	{
	  tempSize = 0;
	}
      temp++;
    }



}

/*! @brief Puts a 32-bit integer to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 32-bit data to write.
 *  @return BOOL - TRUE if Flash was written successfully, FALSE if address is not aligned to a 4-byte boundary or if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_Write32(uint32_t volatile * const address, const uint32_t data)
{

}

/*! @brief Puts a 16-bit integer to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 16-bit data to write.
 *  @return BOOL - TRUE if Flash was written successfully, FALSE if address is not aligned to a 2-byte boundary or if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_Write16(uint16_t volatile * const address, const uint16_t data)
{

}

/*! @brief Puts an 8-bit integer to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 8-bit data to write.
 *  @return BOOL - TRUE if Flash was written successfully, FALSE if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_Write8(uint8_t volatile * const address, const uint8_t data)
{
  uint8_t tempData[8] = {0};

  /*
  volatile uint8_t *flashPtr;
  flashPtr = 0x80000;
  */

  uint8_t index = *address - 0x80000; //gets an integer from the address pointer
  tempData[index] = data;

  Flash_Erase();

  FTFE_FCCOB0 = FTFE_FCCOB0_CCOBn(0x07); // defines the FTFE command to write
  FTFE_FCCOB1 = 0x40;
  FTFE_FCCOB2 = 0x00;
  FTFE_FCCOB3 = 0x00;
  FTFE_FCCOB4 = tempData[0]; // writes to register
  FTFE_FCCOB5 = tempData[1];
  FTFE_FCCOB6 = tempData[2];
  FTFE_FCCOB7 = tempData[3];
  FTFE_FCCOB8 = tempData[4];
  FTFE_FCCOB9 = tempData[5];
  FTFE_FCCOBA = tempData[6];
  FTFE_FCCOBB = tempData[7];


}

/*! @brief Erases the entire Flash sector.
 *
 *  @return BOOL - TRUE if the Flash "data" sector was erased successfully.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_Erase(void)
{
  FTFE_FCCOB0 = FTFE_FCCOB0_CCOBn(0x09); // defines the FTFE command to erase
  FTFE_FCCOB1 = 0x80; // sets flash address[23:16] to 128
  FTFE_FCCOB2 = 0x00; // sets flash address[15:8] to 0
  FTFE_FCCOB3 = 0x00; // sets flash address[7:0] to 0


  FTFE_FSTAT = FTFE_FSTAT_CCIF_MASK; // Launch command sequence

  while(!(FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK)) // Wait for command completion
  return bTRUE;

}
