#include "I2C.h"

#include "MK70F12.h"

#define I2C_D_READ(x) (((uint8_t)(((uint8_t)(x))<<1))|0x01)
#define I2C_D_WRITE(x) (((uint8_t)(((uint8_t)(x))<<1))|0x00)

typedef enum
{
	I2C_AVAILABLE = 0, I2C_BUSY = 1, I2C_ERROR = 2
} TI2CStatus;

typedef enum
{
	I2C_WRITING = 0, I2C_READING = 1
} TI2CTXRX;

typedef struct
{
	uint16_t *sequence;
	uint16_t *sequenceEnd;
	uint8_t *receivedData;
	void (*callbackFunction)(void*);
	void *callbackArguments;
	uint8_t readsAhead;
	TI2CStatus status;
	TI2CTXRX txrx;
} TI2CSequence;

static TI2CSequence Sequence;

BOOL I2C_SendSequence(uint16_t *sequence, uint32_t sequenceLength, uint8_t *receivedData, void (*callbackFunction)(void*), void *userData);

BOOL I2C_Init(const TI2CModule* const aI2CModule, const uint32_t moduleClk)
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

	//Set the i2c module frequency, target (many)Khz. MAGIC!!
	I2C0_F = (I2C_F_MULT(0x02) | I2C_F_ICR(0x1E));
// Enable i2c module
	I2C0_C1 |= I2C_C1_IICEN_MASK;
// Enable interrupt
	I2C0_C1 |= I2C_C1_IICIE_MASK;
	return bTRUE;
}

static uint8_t SlaveAddress;

void I2C_SelectSlaveDevice(const uint8_t slaveAddress)
{
	SlaveAddress = slaveAddress;
}

static uint16_t write_sequence[3]; // = {0/*I2C_D_WRITE(slaveAddress)*/, 0/*registerAddress*/, data};

void I2C_Write(const uint8_t registerAddress, const uint8_t data)
{
	write_sequence[0] = I2C_D_WRITE(SlaveAddress);
	write_sequence[1] = registerAddress;
	write_sequence[2] = data;
	BOOL free = bFALSE;
	while (free == bFALSE)
	{
		free = I2C_SendSequence(write_sequence, 3, (uint8_t *) 0, (void *) 0,
				(void *) 0);
	}
	//Need to wait for send to finish,
	// as all the variables are pointing to the stack.
	while (Sequence.status == I2C_BUSY)
	{
	};
}

static uint16_t read_sequence[] = { 0/*I2C_D_WRITE()*/, 0/*registerAddress*/,
I2C_RESTART, 0/*I2C_D_READ()*/, I2C_READ, I2C_READ, I2C_READ, I2C_READ,
		I2C_READ, I2C_READ };

void I2C_PollRead(const uint8_t registerAddress, uint8_t* const data,
		const uint8_t nbBytes)
{
	read_sequence[0] = I2C_D_WRITE(SlaveAddress);
	read_sequence[1] = registerAddress;
	read_sequence[3] = I2C_D_READ(SlaveAddress);
	BOOL free = bFALSE;
	while (free == bFALSE)
	{
		free = I2C_SendSequence(read_sequence, (4 + nbBytes), data, (void *) 0,
				(void *) 0);
	}
	while (Sequence.status == I2C_BUSY)
	{
	};
}

void I2C_IntRead(const uint8_t registerAddress, uint8_t* const data,
		const uint8_t nbBytes, void (*callback)(void*), void *callbackData)
{
	read_sequence[0] = I2C_D_WRITE(SlaveAddress);
	read_sequence[1] = registerAddress;
	read_sequence[3] = I2C_D_READ(SlaveAddress);
	BOOL free = bFALSE;
	while (free == bFALSE)
	{
		free = I2C_SendSequence(read_sequence, (4 + nbBytes), data, callback,
				callbackData);
	}
}

BOOL I2C_SendSequence(uint16_t *sequence, uint32_t sequenceLength,
		uint8_t *receivedData, void (*callbackFunction)(void*), void *userData)
{
	if ((Sequence.status == I2C_BUSY) | (I2C0_S & I2C_S_BUSY_MASK))
	{
		return bFALSE;
	}
	Sequence.sequence = sequence;
	Sequence.sequenceEnd = sequence + sequenceLength;
	Sequence.receivedData = receivedData;
	Sequence.status = I2C_BUSY;
	Sequence.txrx = I2C_WRITING;
	Sequence.callbackFunction = callbackFunction;
	Sequence.callbackArguments = userData;

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
		Sequence.status = I2C_ERROR;
		return bFALSE;
	}

	//send slave address (w) (first byte)
	I2C0_D = *Sequence.sequence++;
	return bTRUE;
}

void __attribute__ ((interrupt)) I2C_ISR(void)
{
	//copy status register
	uint16_t element;
	uint8_t status = I2C0_S;
	if (!(status & I2C_S_IICIF_MASK))
	{
		//Could be the other i2c
		return;
	}

	//acknowledge interrupt
	I2C0_S |= I2C_S_IICIF_MASK;

	//TODO: arbitration loss test
	if(status & I2C_S_ARBL_MASK) {
		I2C0_S |= I2C_S_ARBL_MASK;
		  goto i2c_isr_error;
		}

	if (Sequence.txrx == I2C_READING)
	{
		switch (Sequence.readsAhead)
		{
		case 0:
			/* All the reads in the sequence have been processed (but note that the final data register read still needs to
			 be done below! Now, the next thing is either a restart or the end of a sequence. In any case, we need to
			 switch to TX mode, either to generate a repeated start condition, or to avoid triggering another I2C read
			 when reading the contents of the data register. */
			I2C0_C1 |= I2C_C1_TX_MASK;

			/* Perform the final data register read now that it's safe to do so. */
			*Sequence.receivedData++ = I2C0_D;

			/* Do we have a repeated start? */
			if ((Sequence.sequence < Sequence.sequenceEnd)
					&& (*Sequence.sequence == I2C_RESTART))
			{

				I2C0_C1 |= I2C_C1_RSTA_MASK; /* Generate a repeated start condition. */

				/* A restart is processed immediately, so we need to get a new element from our sequence. This is safe, because
				 a sequence cannot end with a RESTART: there has to be something after it. Note that the only thing that can
				 come after a restart is an address write. */
				Sequence.txrx = I2C_WRITING;
				Sequence.sequence++;
				element = *Sequence.sequence;
				I2C0_D = element;
			}
			else
			{
				goto i2c_isr_stop;
			}
			break;

		case 1:
			I2C0_C1 |= I2C_C1_TXAK_MASK; /* do not ACK the final read */
			*Sequence.receivedData++ = I2C0_D;
			break;

		default:
			*Sequence.receivedData++ = I2C0_D;
			break;
		}

		Sequence.readsAhead--;

	}
	else
	{ /* channel->txrx == I2C_WRITING */
		/* First, check if we are at the end of a sequence. */
		if (Sequence.sequence == Sequence.sequenceEnd)
		{
			goto i2c_isr_stop;
		}

		if (status & I2C_S_RXAK_MASK)
		{
			/* We received a NACK. Generate a STOP condition and abort. */
			goto i2c_isr_error;
		}

		/* check next thing in our sequence */
		element = *Sequence.sequence;

		if (element == I2C_RESTART)
		{
			/* Do we have a restart? If so, generate repeated start and make sure TX is on. */
			I2C0_C1 |= I2C_C1_RSTA_MASK | I2C_C1_TX_MASK;
			/* A restart is processed immediately, so we need to get a new element from our sequence. This is safe, because a
			 sequence cannot end with a RESTART: there has to be something after it. */
			Sequence.sequence++;
			element = *Sequence.sequence;
			/* Note that the only thing that can come after a restart is a write. */
			I2C0_D = element;
		}
		else
		{
			if (element == I2C_READ)
			{
				Sequence.txrx = I2C_READING;
				/* How many reads do we have ahead of us (not including this one)? For reads we need to know the segment length
				 to correctly plan NACK transmissions. */
				Sequence.readsAhead = 1; /* We already know about one read */
				while (((Sequence.sequence + Sequence.readsAhead) < Sequence.sequenceEnd)
						&& (*(Sequence.sequence + Sequence.readsAhead) == I2C_READ))
				{
					Sequence.readsAhead++;
				}
				I2C0_C1 &= ~I2C_C1_TX_MASK; /* Switch to RX mode. */

				if (Sequence.readsAhead == 1)
				{
					I2C0_C1 |= I2C_C1_TXAK_MASK; /* do not ACK the final read */
				}
				else
				{
					I2C0_C1 &= ~(I2C_C1_TXAK_MASK); /* ACK all but the final read */
				}
				/* Dummy read comes first, note that this is not valid data! This only triggers a read, actual data will come
				 in the next interrupt call and overwrite this. This is why we do not increment the received_data
				 pointer. */
				*Sequence.receivedData = I2C0_D;
				Sequence.readsAhead--;
			}
			else
			{
				/* Not a restart, not a read, must be a write. */
				I2C0_D = element;
			}
		}
	}

	Sequence.sequence++;
	return;

	i2c_isr_stop:
	/* Generate STOP (set MST=0), switch to RX mode, and disable further interrupts. */
	I2C0_C1 &= ~(I2C_C1_MST_MASK | I2C_C1_IICIE_MASK | I2C_C1_TXAK_MASK);
	Sequence.status = I2C_AVAILABLE;
	/* Call the user-supplied callback function upon successful completion (if it exists). */
	if (Sequence.callbackFunction)
	{
		(*Sequence.callbackFunction)(Sequence.callbackArguments);
	}
	return;

	//TODO: Investigate
	i2c_isr_error:
	I2C0_C1 &= ~(I2C_C1_MST_MASK | I2C_C1_IICIE_MASK); /* Generate STOP and disable further interrupts. */
	Sequence.status = I2C_ERROR;
	return;

}
