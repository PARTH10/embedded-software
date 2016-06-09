/*! @file
 *
 *  @brief Implementation of I/O routines for the K70 I2C interface.
 *
 *  This contains the functions for operating the I2C (inter-integrated circuit) module.
 *
 *  @author Robin Wohlers-Reichel, Joshua Gonsalves
 *  @date 2016-05-29
 */
/*!
**  @addtogroup i2c_module I2C module documentation
**  @{
*/
#include "I2C.h"

#include "OS.h"

#include "MK70F12.h"
#include "PE_Types.h"

/*!
 * @brief Macro for shifting read addresses into the i2c data register.
 */
#define I2C_D_READ(x) (((uint8_t)(((uint8_t)(x))<<1))|0x01)

/*!
 * @brief Macro for shifting write addresses into the i2c data register.
 */
#define I2C_D_WRITE(x) (((uint8_t)(((uint8_t)(x))<<1))|0x00)

/*!
 * @brief Direction of communication for current transmission.
 */
typedef enum
{
	I2C_READ = 0, I2C_WRITE = 1
} TI2CCommDirection;

/*!
 * @brief Possible module status.
 */
typedef enum
{
	I2C_AVAILABLE = 0, I2C_BUSY = 1, I2C_ERROR = 2
} TI2CStatus;

/*!
 * @brief Current module status.
 */
TI2CStatus Status;

/*!
 * @brief Current communication direction
 */
static TI2CCommDirection CommunicationDirection;

/*!
 * @brief Position in the I2C transmission.
 * 0 - Send device address (write)
 * 1 - Send register address
 * IF READ
 * 1.5  - send repeated start if read
 * 2 - send device address (read)
 * 3...(n-1) - read data and ak.
 * n - read data and nak. STOP
 * IF WRITE
 * 2...n - send data. STOP
 */
static uint8_t Position = 0;

/*!
 * @brief Address of slave to communicate with.
 */
static uint8_t SlaveAddress;

/*!
 * @brief Register to read / write from.
 */
static uint8_t RegisterAddress;

/*!
 * @brief Copy of byte to write.
 */
static uint8_t WriteByteCopy;

/*!
 * @brief Destination to write data.
 */
static uint8_t *ReadDestination;

/*!
 * @brief Pointer to last byte of destination.
 */
static uint8_t *ReadDestinationEnd;

/*!
 * @brief Callback once current operation completes.
 */
static void (*CallbackFunction)(void*);

/*!
 * @brief Argument passed to callback.
 */
static void *CallbackArguments;

/*!
 * @brief Tolerance of the baud rate search.
 */
#define BAUD_SEARCH_TOLERANCE 3000

/*!
 * @brief Count of multiplier options.
 */
const static size_t multiplierSize = 3;

/*!
 * @brief Multiplier options.
 */
const static uint8_t multiplier[] = { 1, 2, 4 };

/*!
 * @brief Count of scl options.
 */
const static size_t sclSize = 64;

/*!
 * @brief scl options.
 */
const static uint16_t scl[] = { 20, 22, 24, 26, 28, 30, 34, 40, 28, 32, 36, 40, 44, 48,
		56, 68, 48, 56, 64, 72, 80, 88, 104, 128, 80, 96, 112, 128, 144, 160, 192,
		240, 160, 192, 224, 256, 288, 320, 384, 480, 320, 384, 448, 512, 576, 640,
		768, 960, 640, 768, 896, 1024, 1152, 1280, 1536, 1920, 1280, 1536, 1792,
		2048, 2304, 2560, 3072, 3840 };

/*!
 * @brief Set the I2C baud rate.
 * @param baudRate The target baud rate.
 * @param moduleClk The module clock.
 * @return BOOL if successful or not.
 */
BOOL BaudRate(uint32_t baudRate, uint32_t moduleClk)
{
	uint32_t baudResult;
	const uint32_t lowerBaud = baudRate - BAUD_SEARCH_TOLERANCE;
	const uint32_t upperBaud = baudRate + BAUD_SEARCH_TOLERANCE;
	for (int i = 0; i < sclSize; i++)
	{
		for (int j = 0; j < multiplierSize; j++)
		{
			baudResult = (moduleClk / (multiplier[j] * scl[i]));
			if (lowerBaud < baudResult && baudResult < upperBaud)
			{
				I2C0_F |= I2C_F_ICR(i);
				I2C0_F |= I2C_F_MULT(j);
				return bTRUE;
			}
		}
	}
	return bFALSE;
}

BOOL I2C_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
//Get some clocks flowing
	SIM_SCGC4 |= SIM_SCGC4_IIC0_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;

// Clear the control register
	I2C0_C1 = 0;
// clear interrupt
	I2C0_S = I2C_S_IICIF_MASK;

	//I2C is Alt4, p280 manual, also open drain
	PORTE_PCR18 = PORT_PCR_MUX(0x4) | PORT_PCR_ODE_MASK;
	PORTE_PCR19 = PORT_PCR_MUX(0x4) | PORT_PCR_ODE_MASK;

// Interrupts
	NVICIP24 = NVIC_IP_PRI24(0x80);
	NVICISER0 |= NVIC_ISER_SETENA(0x01000000);

	if (!BaudRate(baudRate, moduleClk))
	{
		PE_DEBUGHALT();
		return bFALSE;
	}
// Enable i2c module
	I2C0_C1 |= I2C_C1_IICEN_MASK;
// Enable interrupt
	I2C0_C1 |= I2C_C1_IICIE_MASK;
	return bTRUE;
}

void I2C_SelectSlaveDevice(const uint8_t slaveAddress)
{
	SlaveAddress = slaveAddress;
}

/*!
 * @brief Start a transmission on the I2C bus.
 *
 * @param direction The direction of the communication.
 * @param registerAddress The address to read from on the I2C device.
 * @param data The data to write (if the direction is write).
 * @param destination An array with capacity nbBytes to store the bytes that are read.
 * @param nbBytes The number of bytes to read.
 * @param callback Callback after the operation completes.
 * @param callbackData Data for the callback.
 */
BOOL CommenceTransmission(const TI2CCommDirection direction, const uint8_t registerAddress, const uint8_t data, uint8_t * const destination, const uint8_t nbBytes, void (*callback)(void*), void *callbackData)
{
	if ((Status == I2C_BUSY) | (I2C0_S & I2C_S_BUSY_MASK))
	{
		return bFALSE;
	}

	//Only change globals if we have a lock.
	Status = I2C_BUSY;

	CommunicationDirection = direction;
	RegisterAddress = registerAddress;
	WriteByteCopy = data;
	CallbackFunction = callback;
	CallbackArguments = callbackData;
	ReadDestination = destination;
	ReadDestinationEnd = destination + nbBytes;

	//clear interrupt
	I2C0_S |= I2C_S_IICIF_MASK;
	//enable i2c and interrupts
	I2C0_C1 |= I2C_C1_IICEN_MASK | I2C_C1_IICIE_MASK;
	//start & transmit
	I2C0_C1 |= I2C_C1_TX_MASK | I2C_C1_MST_MASK;	//START signal assert

	//TODO: Arbitration loss test
	uint8_t status = I2C0_S;
	if (status & I2C_S_ARBL_MASK)
	{
		I2C0_C1 &= ~(I2C_C1_IICIE_MASK | I2C_C1_MST_MASK | I2C_C1_TX_MASK);
		Status = I2C_ERROR;
		return bFALSE;
	}

	//Set to next position
	Position = 1;
	//send slave address (w) (first byte)
	I2C0_D = I2C_D_WRITE(SlaveAddress);
	return bTRUE;
}

void I2C_Write(const uint8_t registerAddress, const uint8_t data,
		const BOOL waitCompletion)
{
	BOOL free = bFALSE;
	while (free == bFALSE)
	{
		free = CommenceTransmission(I2C_WRITE, registerAddress, data, (void *)0, 0, (void *) 0, (void *) 0);
	}
	if (waitCompletion)
	{
		//Need to wait for send to finish,
		// as all the variables are pointing to the stack.
		while (Status == I2C_BUSY)
		{
		};
	}
}

void I2C_PollRead(const uint8_t registerAddress, uint8_t* const destination,
		const uint8_t nbBytes)
{
	I2C_IntRead(registerAddress, destination, nbBytes, (void *) 0, (void *) 0);
	while (Status == I2C_BUSY)
	{
	};
}

void I2C_IntRead(const uint8_t registerAddress, uint8_t* const destination,
		const uint8_t nbBytes, void (*callback)(void*), void *callbackData)
{
	BOOL free = bFALSE;
	while (free == bFALSE)
	{
		free = CommenceTransmission(I2C_READ, registerAddress, 0, destination, nbBytes, callback, callbackData);
	}
}

/*!
 * @brief Handle an I2C error
 */
void Error()
{
	I2C0_C1 &= ~(I2C_C1_MST_MASK | I2C_C1_IICIE_MASK); /* Generate STOP and disable further interrupts. */
	Status = I2C_ERROR;
}

/*!
 * @brief Stop the I2C bus
 */
void Stop()
{
	I2C0_C1 &= ~(I2C_C1_MST_MASK | I2C_C1_IICIE_MASK | I2C_C1_TXAK_MASK);
	Status = I2C_AVAILABLE;
	if (CallbackFunction)
	{
		(*CallbackFunction)(CallbackArguments);
	}
}

/*!
 * @brief Get the number of remaining reads
 */
uint8_t RemainingReads()
{
	return ReadDestinationEnd - ReadDestination;
}

void __attribute__ ((interrupt)) I2C_ISR(void)
{
	OS_ISREnter();
	//copy status register
	uint16_t element;
	uint8_t status = I2C0_S;
	if (!(status & I2C_S_IICIF_MASK))
	{
		//Could be the other i2c
		OS_ISRExit();
		return;
	}

	//acknowledge interrupt
	I2C0_S |= I2C_S_IICIF_MASK;

	//TODO: arbitration loss test
	if (status & I2C_S_ARBL_MASK)
	{
		I2C0_S |= I2C_S_ARBL_MASK;
		Error();
		OS_ISRExit();
		return;
	}

	if (CommunicationDirection == I2C_WRITE)
	{
		switch (Position)
		{
		case 0:
		case 1:
			//send register address
			I2C0_D = RegisterAddress;
			Position++;
			break;
		case 2:
			//restart if CommunicationDirection is READ
			I2C0_D = WriteByteCopy;
			Position++;
			break;
		default:
			Stop();
		}
	}
	else
	{
		switch (Position)
		{
		case 0:
		case 1:
			//send register address
			I2C0_D = RegisterAddress;
			Position++;
			break;
		case 2:
			I2C0_C1 |= I2C_C1_RSTA_MASK;
			I2C0_D = I2C_D_READ(SlaveAddress);
			Position++;
			break;
		case 3:
			I2C0_C1 &= ~I2C_C1_TX_MASK; /* Switch to RX mode. */
			if (RemainingReads() > 1)
			{
				//0 = AK
				I2C0_C1 &= ~(I2C_C1_TXAK_MASK);
			}
			else
			{
				//1 = NAK
				I2C0_C1 |= I2C_C1_TXAK_MASK;
			}
			*ReadDestination = I2C0_D;		//read out crap
			Position++;
			break;
		case 4:
			*ReadDestination++ = I2C0_D;
			uint8_t remainder = RemainingReads();
			if (remainder == 0)
			{
				Position++;
			}
			else if (remainder == 1)
			{
				I2C0_C1 |= I2C_C1_TXAK_MASK;
			}
			break;
		default:
			Stop();
			break;
		}
	}
	OS_ISRExit();
}
/*!
** @}
*/
