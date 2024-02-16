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
#include <iostream>
using std::cout;
using std::endl;
using std::string;
using std::to_string;

// Change these parameters as needed:
#define TABLE_SIZE 16 

// Output Width
#define W 20
#define IW 0

int cordic_angle_gentable(int size) {

  create_dir("generated_tables");

  double c;

  const string filename = "generated_tables/cordic_angle_hyp_table.hpp";
 //cout << "Generating table of size: " << size
 //     << " in file: " << filename << endl;

  std::ofstream ofs(filename, std::ofstream::out);
  ofs << "#pragma once" << endl << endl;

  ofs << "namespace hls{\nnamespace math{\nconst int HYP_TABLE_SIZE = " << TABLE_SIZE << ";\n";
  ofs << "  hls::ap_ufixpt<" << W ", " << IW << "> cordicHypTable[HYP_TABLE_SIZE] = {" << endl << "1," << endl;

  for (int i = 1; i < size; i++){
      c = atanh(pow(2, -i));
//    cout << c << endl;
      ofs << c << (i == size - 1 ? "};\n" : ",") << endl;
  }

  ofs << "}}" << endl;
//cout << c << endl;
  return 0;
}

int main(){
	cordic_angle_gentable(TABLE_SIZE);
}
