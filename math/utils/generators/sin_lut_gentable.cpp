// ©2022 Microchip Technology Inc. and its subsidiaries
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

#include "../../examples/test_utils.hpp"
#include <fstream>
#include <vector>

// DECIM = highest number of fractional bits the input can have without losing precision
#define DECIM 16
// W_OUT & IW_OUT = configuration of table values. Can be increased, 16 chosen because it uses 1 LSRAM.
#define W_OUT 16 
#define IW_OUT 2

using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::vector;


/* Generates cos & sin LUTs for angles w/i 0 to PI/2 */
int main(int argc, char *argv[]) {
    create_dir("generated_tables");

    const string filename = "generated_tables/sin_lut_table.hpp";

/*  W = user bitwidth, IW = user integer width
 *  Decim is the number of bits dedicated to decimal representation in the input. It's the n in Qm.n
 *  T_W = my (table's) angle bitwidth, T_IW = my integer width
 *  fa = table for larger angles, fb = table for smaller angles
 */
    unsigned T_W, T_IW, fa_width, fb_width, fa_table_entries, fb_table_entries, trans_HALF_PI;
    double half_pi = M_PI_2;

/*  We only need to represent the sin/cos values for angles between 0 and PI/2 (1.57079632679)-- i.e. input will only be within 0 to pi/2.
 *  DW and IW are meant to represent a "cozy" number representation, meaning every bit is used at lease once in the range of representing 0 to PI/2
 *  
 */    

    T_IW = 1;  // 1 integer bit for PI/2 
    T_W = DECIM + T_IW; 
    fa_width = T_W >> 1;
    fb_width = T_W - fa_width;

/* trans_HALF_PI = number of representable values between 0 to PI/2 using Q DW.decim
 * fa_table_entries = trans_half_pi / 2 + 1 <- fa will handle angles from 2^fb_width to trans_half_pi
 * fab_table_entries = 2^fb_width
 */    
    trans_HALF_PI = (unsigned)(half_pi * pow(2, DECIM)); // This used to be 90 << decim. Using PI/2, the types don't work out so we have to settle for this.
    fa_table_entries = (trans_HALF_PI >> fb_width) + 1;
    fb_table_entries = (1 << fb_width); 

    DBG_CODE {
      cout << "decim = " << DECIM << endl;
      cout << "T_W = " << T_W << ", T_IW = " << T_IW << endl;
      cout << "fa_width = " << fa_width << ", fb_width = " << fb_width << endl;
      cout << "trans_HALF_PI = " << trans_HALF_PI << endl;
      cout << "fa_table_entries = " << fa_table_entries << ", fb_table_entries = " << fb_table_entries << endl;
    }
    int dir = 1;
    if (((int)fb_width - (int)DECIM) < 0) dir = -1;

    DBG_CODE {
      cout << "dir = " << dir << endl;
    }

    double c;

    std::ofstream ofs(filename, std::ofstream::out);
    ofs << "#pragma once" << endl << endl;
    ofs << "#define _HLS_SIN_LUT_DECIM " << DECIM << endl << endl;
    ofs << "namespace hls {\n namespace math {\n" << endl;

    ofs << "  ap_fixpt<" << W_OUT <<", " << IW_OUT << "> sin_fa_lut[" << fa_table_entries << "] = {" << endl;

    for (unsigned i = 0; i < fa_table_entries; i++){
        double fa;
        if (dir == 1) fa = i << (fb_width - DECIM);
        else fa = (i / pow(2,(DECIM - fb_width)));
        c = sin(fa);
        ofs << c << (i == fa_table_entries - 1 ?"};\n" : ",") << endl;

        DBG_CODE {
          cout << "i = " << i << ", fa = " << (fa * 180 / M_PI) << endl;
          cout << "sin(fa) = " << sin(fa) << endl;
        }
    }


    ofs << "  ap_fixpt<" << W_OUT <<", " << IW_OUT << "> sin_fb_lut[" << fb_table_entries << "] = {" << endl;
    for (unsigned i = 0; i < fb_table_entries; i++){
	double fb = i / pow(2, DECIM);
        c = sin(fb);
	ofs << c << (i == fb_table_entries - 1 ?"};\n" : ",") << endl;	

	DBG_CODE {	
          cout << "i = " << i << ", fb = " <<  (fb * 180 / M_PI) << endl;
          cout << "sin(fb) = " << sin(fb) << endl;
	}
    }

    DBG_CODE {
      cout << endl << "Starting fa cos:" << endl;
    }

    ofs << "  ap_fixpt<" << W_OUT <<", " << IW_OUT << "> cos_fa_lut[" << fa_table_entries << "] = {" << endl;    
    for (unsigned i = 0; i < fa_table_entries; i++){
	double fa;
        if (dir == 1) fa = i << (fb_width - DECIM);
	else fa = (i / pow(2,(DECIM - fb_width)));
        c = cos(fa);
        ofs << c << (i == fa_table_entries - 1 ?"};\n" : ",") << endl;

	DBG_CODE {
          cout << "i = " << i << ", fa = " <<  (fa * 180 / M_PI) << endl;
          cout << "cos(fa) = " << cos(fa) << endl;
	}
    }

    DBG_CODE {
      cout << endl << "Starting fb cos:" << endl;
    }

    ofs << "  ap_fixpt<" << W_OUT <<", " << IW_OUT << "> cos_fb_lut[" << fb_table_entries << "] = {" << endl;    
    for (unsigned i = 0; i < fb_table_entries; i++){
	double fb = i / pow(2, DECIM); //(i >> decim);
        c = cos(fb);
        ofs << c << (i == fb_table_entries - 1 ?"};\n" : ",") << endl;

	DBG_CODE {
          cout << "i = " << i << ", fb = " <<  (fb * 180 / M_PI) << endl;
          cout << "cos(fb) = " << cos(fb) << endl;
	}
    }

    ofs << "}\n}\n" << endl;

    return 0;
}
