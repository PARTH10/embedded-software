/*! @file
 *
 *  @brief FIFO buffer function implementations
 *
 *  @author Robin Wohlers-Reichel, Joshua Gonsalves
 *  @date 2016-03-23
 */
/*!
**  @addtogroup fifo_module FIFO module documentation
**  @{
*/
#include "FIFO.h"
#include "Cpu.h"

void FIFO_Init(TFIFO * const FIFO)
{
  FIFO->Start = 0;
  FIFO->End = 0;
  FIFO->NbBytes = 0;
}

BOOL FIFO_Put(TFIFO * const FIFO, const uint8_t data)
{
  if (FIFO->NbBytes >= FIFO_SIZE)
  {
    return bFALSE;
  }
  EnterCritical();
  FIFO->Buffer[FIFO->End] = data;
  FIFO->NbBytes++;
  FIFO->End++;
  if (FIFO->End >= FIFO_SIZE)
  {
    FIFO->End = 0;
  }
  ExitCritical();
  return bTRUE;
}

BOOL FIFO_Get(TFIFO * const FIFO, uint8_t volatile * const dataPtr)
{
  if (FIFO->NbBytes == 0)
  {
    return bFALSE;
  }
  EnterCritical();
  *dataPtr = FIFO->Buffer[FIFO->Start];
  FIFO->Start++;
  FIFO->NbBytes--;
  if (FIFO->Start >= FIFO_SIZE)
  {
    FIFO->Start = 0;
  }
  ExitCritical();
  return bTRUE;
}

/*!
** @}
*/
