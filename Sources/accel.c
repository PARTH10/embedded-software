/*! @file
 *
 *  @brief Implementation of the HAL for the accelerometer.
 *
 *  @author Robin Wohlers-Reichel, Joshua Gonsalves
 *  @date 2016-03-23
 */

/*!
**  @addtogroup accel_module accel module documentation
**  @{
*/

// Accelerometer functions
#include "accel.h"

// Inter-Integrated Circuit
#include "I2C.h"

// Median filter
#include "median.h"

// K70 module registers
#include "MK70F12.h"

// CPU and PE_types are needed for critical section variables and the defintion of NULL pointer
#include "CPU.h"
#include "PE_types.h"

typedef enum
{
  DATE_RATE_800_HZ,
  DATE_RATE_400_HZ,
  DATE_RATE_200_HZ,
  DATE_RATE_100_HZ,
  DATE_RATE_50_HZ,
  DATE_RATE_12_5_HZ,
  DATE_RATE_6_25_HZ,
  DATE_RATE_1_56_HZ
} TOutputDataRate;

typedef enum
{
  SLEEP_MODE_RATE_50_HZ,
  SLEEP_MODE_RATE_12_5_HZ,
  SLEEP_MODE_RATE_6_25_HZ,
  SLEEP_MODE_RATE_1_56_HZ
} TSLEEPModeRate;

#define MMA8451Q_ADDR_SA0_LOW 0x1Cu
#define MMA8451Q_ADDR_SA0_HIGH 0x1Du

// Accelerometer registers
#define MMA8451Q_STATUS     0x00u
#define MMA8451Q_OUT_X_MSB  0x01u
#define MMA8451Q_OUT_X_LSB  0x02u
#define MMA8451Q_OUT_Y_MSB  0x03u
#define MMA8451Q_OUT_Y_LSB  0x04u
#define MMA8451Q_OUT_Z_MSB  0x05u
#define MMA8451Q_OUT_Z_LSB  0x06u
#define MMA8451Q_INT_SOURCE 0x0Cu
#define MMA8451Q_WHO_AM_I   0x0Du
#define MMA8451Q_CTRL_REG1  0x2Au
#define MMA8451Q_CTRL_REG2  0x2Bu
#define MMA8451Q_CTRL_REG3  0x2Cu
#define MMA8451Q_CTRL_REG4  0x2Du
#define MMA8451Q_CTRL_REG5  0x2Eu

#define MMA8451Q_STATUS_XDR_MASK   0x1u
#define MMA8451Q_STATUS_YDR_MASK   0x2u
#define MMA8451Q_STATUS_ZDR_MASK   0x4u
#define MMA8451Q_STATUS_ZYXDR_MASK 0x8u
#define MMA8451Q_STATUS_XOW_MASK   0x10u
#define MMA8451Q_STATUS_YOW_MASK   0x20u
#define MMA8451Q_STATUS_ZOW_MASK   0x40u
#define MMA8451Q_STATUS_ZYXOW_MASK 0x80u

#define MMA8451Q_INT_SOURCE_SRC_DRDY_MASK   0x1u
#define MMA8451Q_INT_SOURCE_SRC_FF_MT_MASK  0x4u
#define MMA8451Q_INT_SOURCE_SRC_PULSE_MASK  0x8u
#define MMA8451Q_INT_SOURCE_SRC_LNDPRT_MASK 0x10u
#define MMA8451Q_INT_SOURCE_SRC_TRANS_MASK  0x20u
#define MMA8451Q_INT_SOURCE_SRC_FIFO_MASK   0x40u
#define MMA8451Q_INT_SOURCE_SRC_ASLP_MASK   0x80u

#define MMA8451Q_WHO_AM_I_VALUE 0x1A

#define MMA8451Q_CTRL_REG1_ACTIVE_MASK	  0x1u
#define MMA8451Q_CTRL_REG1_F_READ_MASK  	0x2u
#define MMA8451Q_CTRL_REG1_LNOISE_MASK  	0x4u
#define MMA8451Q_CTRL_REG1_DR_MASK	    	0x38u
#define MMA8451Q_CTRL_REG1_ASLP_RATE_MASK	0xC0u

#define MMA8451Q_CTRL_REG2_RST_MASK 0x40u

#define MMA8451Q_CTRL_REG3_PP_OD_MASK		    0x1u
#define MMA8451Q_CTRL_REG3_IPOL_MASK		    0x2u
#define MMA8451Q_CTRL_REG3_WAKE_FF_MT_MASK	0x8u
#define MMA8451Q_CTRL_REG3_WAKE_PULSE_MASK	0x10u
#define MMA8451Q_CTRL_REG3_WAKE_LNDPRT_MASK	0x20u
#define MMA8451Q_CTRL_REG3_WAKE_TRANS_MASK	0x40u
#define MMA8451Q_CTRL_REG3_FIFO_GATE_MASK	  0x80u

#define MMA8451Q_CTRL_REG4_INT_EN_DRDY_MASK	  0x1u
#define MMA8451Q_CTRL_REG4_INT_EN_FF_MT_MASK	0x4u
#define MMA8451Q_CTRL_REG4_INT_EN_PULSE_MASK	0x8u
#define MMA8451Q_CTRL_REG4_INT_EN_LNDPRT_MASK	0x10u
#define MMA8451Q_CTRL_REG4_INT_EN_TRANS_MASK	0x20u
#define MMA8451Q_CTRL_REG4_INT_EN_FIFO_MASK	  0x40u
#define MMA8451Q_CTRL_REG4_INT_EN_ASLP_MASK	  0x80u

#define MMA8451Q_CTRL_REG5_INT_CFG_DRDY_MASK	 0x1u
#define MMA8451Q_CTRL_REG5_INT_CFG_FF_MT_MASK  0x4u
#define MMA8451Q_CTRL_REG5_INT_CFG_PULSE_MASK	 0x8u
#define MMA8451Q_CTRL_REG5_INT_CFG_LNDPRT_MASK 0x10u
#define MMA8451Q_CTRL_REG5_INT_CFG_TRANS_MASK  0x20u
#define MMA8451Q_CTRL_REG5_INT_CFG_FIFO_MASK   0x40u
#define MMA8451Q_CTRL_REG5_INT_CFG_ASLP_MASK   0x80u

/*!
 * @brief Callback once data is available.
 */
static void (*DataCallback)(void *);

/*!
 * @brief Argument to pass to data callback.
 */
static void *DataCallbackArgument;

/*!
 * @brief Callback once read is complete.
 */
static void (*ReadCallback)(void *);

/*!
 * @brief Argument passed to read callback.
 */
static void *ReadCallbackArgument;

/*!
 * @brief The current mode of the accel module.
 */
static TAccelMode CurrentMode;

/*!
 * @brief Change the active mode of the accelerometer.
 *
 * @param isActive bTRUE in order to activate the accelerometer.
 */
void SetActive(BOOL isActive)
{
	uint8_t reg1Tmp;
	I2C_PollRead(MMA8451Q_CTRL_REG1, &reg1Tmp, 1);
	if (isActive)
	{
		reg1Tmp |= MMA8451Q_CTRL_REG1_ACTIVE_MASK;
	}
	else
	{
		reg1Tmp &= ~MMA8451Q_CTRL_REG1_ACTIVE_MASK;
	}
	I2C_Write(MMA8451Q_CTRL_REG1, reg1Tmp, bFALSE);
}

BOOL Accel_Init(const TAccelSetup* const accelSetup)
{
	TAccelMode CurrentMode;

	DataCallback = accelSetup->dataReadyCallbackFunction;
	DataCallbackArgument = accelSetup->dataReadyCallbackArguments;

	ReadCallback = accelSetup->readCompleteCallbackFunction;
	ReadCallbackArgument = accelSetup->readCompleteCallbackArguments;

	//Let there be clocks!
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;

	/*portb GPIO, clear interrupt, interrupt on falling edge.*/
	PORTB_PCR7 = (PORT_PCR_MUX(0x01) | PORT_PCR_ISF_MASK | PORT_PCR_IRQC(0x0A));

	/* NVICIP88: PRI88=0x80 */
	NVICIP88 = NVIC_IP_PRI88(0x80);
	/* NVICISER2: SETENA|=0x01000000 */
	NVICISER2 |= NVIC_ISER_SETENA(0x01000000);

	//Check if we are connected to the correct device
	uint8_t whoAmI;
	I2C_SelectSlaveDevice(MMA8451Q_ADDR_SA0_HIGH);
	I2C_PollRead(MMA8451Q_WHO_AM_I, &whoAmI, 1);

	if (whoAmI != MMA8451Q_WHO_AM_I_VALUE)
	{
		//This is not the i2c device we are looking for
		return bFALSE;
	}

	//Reset the accelerometer
	I2C_Write(MMA8451Q_CTRL_REG2, MMA8451Q_CTRL_REG2_RST_MASK, bFALSE);
	uint8_t reg2 = MMA8451Q_CTRL_REG2_RST_MASK;
	while (reg2 & MMA8451Q_CTRL_REG2_RST_MASK)
	{
		I2C_PollRead(MMA8451Q_CTRL_REG2, &reg2, 1);
	}

	/*
	 * activate
	 * enable fast read
	 * low noise
	 * data rate 1.56Hz (0x38)
	 *
	 */
	I2C_Write(MMA8451Q_CTRL_REG1, (0x38 | MMA8451Q_CTRL_REG1_ACTIVE_MASK | MMA8451Q_CTRL_REG1_F_READ_MASK | MMA8451Q_CTRL_REG1_LNOISE_MASK), bFALSE);
	return bTRUE;
}

void Accel_ReadXYZ(uint8_t data[3])
{
	I2C_SelectSlaveDevice(MMA8451Q_ADDR_SA0_HIGH);
	I2C_IntRead(MMA8451Q_OUT_X_MSB, data, 3, ReadCallback, ReadCallbackArgument);
}

void Accel_SetMode(const TAccelMode mode)
{
	//Update the static variable
	CurrentMode = mode;

	//Read register 4 of the accelerometer
	uint8_t reg4Tmp;
	I2C_PollRead(MMA8451Q_CTRL_REG4, &reg4Tmp, 1);
	switch (mode)
	{
	case ACCEL_POLL:
		reg4Tmp &= ~MMA8451Q_CTRL_REG4_INT_EN_DRDY_MASK;
	case ACCEL_INT:
	default:
		reg4Tmp |= MMA8451Q_CTRL_REG4_INT_EN_DRDY_MASK;
	}

	/*Active off*/
	SetActive(bFALSE);

	/*Write the interrupt (or not)*/
	I2C_Write(MMA8451Q_CTRL_REG4, reg4Tmp, bFALSE);

	/*Active on*/
	SetActive(bTRUE);
}

TAccelMode Accel_GetMode()
{
	return CurrentMode;
}

void __attribute__ ((interrupt)) AccelDataReady_ISR(void)
{
	//is it this interrupt
	if (!(PORTB_PCR7 & PORT_PCR_ISF_MASK))
	{
		return;
	}

	//clear the interrupt on the k70
	PORTB_PCR7 |= PORT_PCR_ISF_MASK;

	//clear the interrupt on the mma by reading the data.
	(DataCallback)(DataCallbackArgument);

}

/*!
 * @}
*/
