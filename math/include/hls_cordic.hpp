// ©2025 Microchip Technology Inc. and its subsidiaries
//
// Subject to your compliance with these terms, you may use this Microchip
// software and any derivatives exclusively with Microchip products. You are
// responsible for complying with third party license terms applicable to your
// use of third party software (including open source software) that may
// accompany this Microchip software. SOFTWARE IS “AS IS.” NO WARRANTIES,
// WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING
// ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR
// A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY
// INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST
// OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED,
// EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
// FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL
// LIABILITY ON ALL CLAIMS LATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF
// FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. MICROCHIP
// OFFERS NO SUPPORT FOR THE SOFTWARE. YOU MAY CONTACT MICROCHIP AT
// https://www.microchip.com/en-us/support-and-training/design-help/client-support-services
// TO INQUIRE ABOUT SUPPORT SERVICES AND APPLICABLE FEES, IF AVAILABLE.

#pragma once
#include "hls_common.hpp"
#include "../utils/generators/generated_tables/cordic_angle_table.hpp"
#include "../utils/generators/generated_tables/cordic_angle_hyp_table.hpp"
namespace hls{
namespace math{
/***
 * @title cordic
 */

/***
 * @function cordic
 * Runs the CORDIC algorithm using the circular coordinate system. See hls_sincos and hls_atan for examples of usage.
 *
 * @template {int} SIZE number of desired CORDIC iterations. If SIZE is larger than the depth of the angle table, the depth of the angle table will be used for number of iterations instead.
 * @template {int} MODE variable to indicate vectoring mode (e.g. y -> 0) or rotating mode (e.g. angle -> 0). See hls_common.hpp for modes.
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} angle input argument in radians
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} x x-coordinate, will hold the resulting x-coordinate after CORDIC executes
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} y y-coordinate, will hold the resulting y-coordinate after CORDIC executes
 * @example
 * hls::ap_fixpt<32, 16> x = 0.60725293500888125616;
 * hls::ap_fixpt<32, 16> y = 0;
 * hls::ap_fixpt<32, 16> r = 3.14;
 * hls::math::cordic<16, ROTATING>(r, x, y); 
 * hls::math::cordic<16, VECTORING>(r, x, y);
 */
template <int SIZE, int MODE, unsigned int W_IN, int IW_IN>
void cordic(ap_fixpt<W_IN, IW_IN> &angle, ap_fixpt<W_IN, IW_IN> &x,
        ap_fixpt<W_IN, IW_IN> &y) {

  // If you'd like to make your own table, see the instructions in utils/generators.
  // Safe-guard in case SIZE is too big for table
  // TABLE_SIZE is defined in cordic_angle_table.hpp
  const int n_iter = (SIZE < TABLE_SIZE) ? SIZE : TABLE_SIZE;
  const ap_fixpt<W_IN, IW_IN> ZERO(0);
  ap_fixpt<W_IN, IW_IN> new_x(x);
  ap_fixpt<W_IN, IW_IN> new_y(y);

#pragma HLS loop unroll
  for (int i = 0; i < n_iter; i++) {
    ap_uint<1> d = (MODE == ROTATING) ? angle[W_IN-1] : !y[W_IN-1];

    ap_fixpt<W_IN, IW_IN> d_y = y >> i;
    ap_fixpt<W_IN, IW_IN> d_x = x >> i;
    if (d == 1) { // CCW
      new_x += d_y;
      new_y -= d_x;
      angle += cordicTable[i];
//      DBG_CODE { printf("PLUS %f, NEW ANGLE = %f\n", cordicTable[i].to_double(), angle.to_double());}
    } else { // CW
      new_x -= d_y;
      new_y += d_x;
      angle -= cordicTable[i];
     // DBG_CODE { printf("MINUS %f, NEW ANGLE = %f\n", cordicTable[i].to_double(), angle.to_double());}
    }

    x = new_x;
    y = new_y;
   // DBG_CODE { printf("NEW X = %f, NEW Y = %f\n\n", x.to_double(), y.to_double());}
  }
}

/***
 * @function cordic
 * Runs the CORDIC algorithm using the circular coordinate system. This version of CORDIC drives y to the fourth parameter, desired_val.
 *
 * See hls_asin for an example of usage.
 *
 * @template {int} SIZE number of desired CORDIC iterations. If SIZE is larger than the depth of the angle table, the depth of the angle table will be used for number of iterations instead.
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} angle input argument in radians
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} x x-coordinate, will hold the resulting x-coordinate after CORDIC executes
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} y y-coordinate, will hold the resulting y-coordinate after CORDIC executes
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} desired_val value to drive y to
* @example
 * hls::ap_fixpt<32, 16> x = 0.60725293500888125616;
 * hls::ap_fixpt<32, 16> y = 0;
 * hls::ap_fixpt<32, 16> r = 3.14;
 * hls::ap_fixpt<32, 16> dv = 3.14;
 * hls::math::cordic<16>(r, x, y, dv);
 */
template <int SIZE, unsigned int W_IN, int IW_IN>
void cordic(ap_fixpt<W_IN, IW_IN> &angle, ap_fixpt<W_IN, IW_IN> &x,
        ap_fixpt<W_IN, IW_IN> &y, ap_fixpt<W_IN, IW_IN> desired_val) {

  // If you'd like to make your own table, see the instructions in utils/generators.
  //cordicTable<W_IN, IW_IN> t;

  // Safe-guard in case specified size is too big for table
  // TABLE_SIZE is defined in cordic_angle_table.hpp
  const int n_iter = (SIZE < TABLE_SIZE) ? SIZE : TABLE_SIZE;

  ap_fixpt<W_IN, IW_IN> new_x(x);
  ap_fixpt<W_IN, IW_IN> new_y(y);

#pragma HLS loop unroll

  for (int i = 0; i < n_iter; i++) {
    if (y > desired_val) { // CCW
      new_x += y >> i;
      new_y -= x >> i;
      angle = angle + cordicTable[i];
//      DBG_CODE { printf("PLUS %f, NEW ANGLE = %f\n", t.cordic_tab[i].to_double(), angle.to_double());}
    } else { // CW
      new_x -= y >> i;
      new_y += x >> i;
      angle = angle - cordicTable[i];
  //    DBG_CODE { printf("MINUS %f, NEW ANGLE = %f\n", t.cordic_tab[i].to_double(), angle.to_double());}
    }

    x = new_x;
    y = new_y;
    DBG_CODE { printf("NEW X = %f, NEW Y = %f\n\n", x.to_double(), y.to_double());}
  }
}



/***
 * @function hyp_cordic
 * Runs the CORDIC algorithm. See hls_exp for an example of usage.
 *
 * NOTE: We use a gain of 0.82816, and a max angle of 1.11817. 
 *
 * @template {int} SIZE number of desired CORDIC iterations. If SIZE is larger than the depth of the angle table, the depth of the angle table will be used for number of iterations instead.
 * @template {int} MODE variable to indicate vectoring mode (e.g. y -> 0) or rotating mode (e.g. angle -> 0). See hls_common.hpp for modes.
 *
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} angle input argument in radians
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} x x-coordinate, will hold the resulting x-coordinate after CORDIC executes
 * @param {ap_fixpt<unsigned int W_IN, int IW_IN>} y y-coordinate, will hold the resulting y-coordinate after CORDIC executes
 * @example
 * hls::ap_fixpt<32, 16> x = 0.82816;
 * hls::ap_fixpt<32, 16> y = 0;
 * hls::ap_fixpt<32, 16> r = 3.14;
 * hls::math::cordic_hyp<16, ROTATING>(r, x, y);
 * hls::math::cordic_hyp<16, VECTORING>(r, x, y);
 */
template <int SIZE, int MODE, unsigned int W_IN, int IW_IN>
void cordic_hyp(ap_fixpt<W_IN, IW_IN> &angle, ap_fixpt<W_IN, IW_IN> &x,
        ap_fixpt<W_IN, IW_IN> &y) {

  // If you'd like to make your own table, see the instructions in utils/generators.

  // Safe-guard in case SIZE is too big for table
  // TABLE_SIZE is defined in cordic_angle_table.hpp
  const int n_iter = (SIZE < HYP_TABLE_SIZE) ? SIZE : HYP_TABLE_SIZE;	

  // Safe-guard in case specified size is too big for table
  // TABLE_SIZE is defined in cordic_angle_table.hpp
  ap_fixpt<W_IN, IW_IN> new_x(x);
  ap_fixpt<W_IN, IW_IN> new_y(y);
  ap_ufixpt<1, 1> ZERO(0);
  unsigned int i = 1, j = 4;

//To be fixed in a future release, unroll factor should be loop bounds, but unroll factor doesn't support const int as params at this time.
#pragma HLS loop unroll factor(SIZE + SIZE/4)
  while (i < n_iter) {
    ap_uint<1> d = (MODE == ROTATING) ? (ZERO > angle) : (y > ZERO);
    if (d) { 
      new_x -= y >> i;
      new_y -= x >> i;
      angle = angle + cordicHypTable[i];

//      DBG_CODE {printf("MINUS %f, NEW ANGLE = %f\n", t.cordic_tab[i].to_double(), angle.to_double()); }
    } else { 
      new_x += y >> i;
      new_y += x >> i;
      angle = angle - cordicHypTable[i];

  //    DBG_CODE {printf("PLUS %f, NEW ANGLE = %f\n", t.cordic_tab[i].to_double(), angle.to_double());}
    }
    if (i == j){
      j = 3 * i + 1;
    }
    else i++;
 
    x = new_x;
    y = new_y;
    DBG_CODE {printf("NEW X = %f, NEW Y = %f\n\n", x.to_double(), y.to_double());}
  }
 
}
}
}
