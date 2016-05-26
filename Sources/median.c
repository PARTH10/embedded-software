/*! @file
 *
 *  @brief Median filter.
 *
 *  This contains the functions for performing a median filter on byte-sized data.
 *
 *  @author Robin Wohlers-Reichel, Joshua Gonsalves
 *  @date 2015-10-12
 */
/*!
**  @addtogroup median_module Median module documentation
**  @{
*/

#include "median.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

uint8_t Median_Filter3(const uint8_t n1, const uint8_t n2, const uint8_t n3)
{
	return MAX(MIN(n1, n2), MIN(MAX(n1, n2), n3));
}
/*!
** @}
*/
