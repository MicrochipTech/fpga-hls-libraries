#pragma once

#include "hls/ap_fixpt.hpp"
#include "hls/ap_int.hpp"
#include "math.h"

namespace hls {
namespace vision {

template <unsigned RANGE>
struct GaussianIntensityTable {
    ap_int<8> Table[RANGE];
    GaussianIntensityTable(float sigma) {
        float TmpTable[RANGE];
        float max_value = 0;

        #pragma unroll
        for(int i=0; i < RANGE; i++) {
            TmpTable[i] = Gaussian(i, sigma);
            if (TmpTable[i] > max_value) 
                max_value = TmpTable[i];
        }
        float factor = (255.0f / max_value);
    
        // Scale and round the table values
        #pragma unroll
        for(int i=0; i < RANGE; i++) {
            Table[i] = (uint8_t)(TmpTable[i] * factor + 0.5f);
        }
    };

    float __attribute__((always_inline)) Gaussian(int x, float sigma) {
        #pragma HLS function inline
        return exp(-(pow(x, 2)) / (2 * pow(sigma, 2))) /
               (2 * M_PI * pow(sigma, 2));
    }
};

template <unsigned SIZE>
struct GaussianSpaceTable {
    ap_int<8> Table[SIZE][SIZE];
    GaussianSpaceTable(float sigma) {
        float TmpTable[SIZE][SIZE];
        float max_value = 0;

        #pragma unroll
        for (int i = 0; i < SIZE; i++) {
            #pragma unroll
            for (int j = 0; j < SIZE; j++) {
                TmpTable[i][j] =
                    Gaussian(dist(i, j, (SIZE - 1) / 2, (SIZE - 1) / 2), sigma);
            }
        }

        // Scale and round the table values
        #pragma unroll
        for (int i = 0; i < SIZE; i++) {
            #pragma unroll
            for (int j = 0; j < SIZE; j++) {
              Table[i][j] = (uint8_t)(TmpTable[i][j] + 0.5f);
            }
        }
    };

    float __attribute__((always_inline)) Gaussian(float x, float sigma) {
        #pragma HLS function inline
        float FACTOR = 256 * 1024;
        return FACTOR * exp(-(pow(x, 2)) / (2 * pow(sigma, 2))) /
               (2 * M_PI * pow(sigma, 2));
    }

    float __attribute__((always_inline)) dist(int x, int y, int i, int j) {
        #pragma HLS function inline
        return float(sqrt(pow(x - i, 2) + pow(y - j, 2)));
    }
};

template <unsigned RANGE, unsigned SIZE> 
struct Gaussian {
    const GaussianIntensityTable<RANGE> GI;
    const GaussianSpaceTable<SIZE> GS;
    Gaussian(float sigmaIntensity, float sigmaSpace)
        : GI(sigmaIntensity), GS(sigmaSpace) {
            // printTables();
        }
    uint8_t getIntensity(int i) const { return GI.Table[i]; }
    uint8_t getSpace(int i, int j) const { return GS.Table[i][j]; }

    // For debugging purposes
    void printTables() {
        printf("\n----------[Intensity]-------------\n");
        for(int i=0; i<RANGE; i++) {
            printf("%d:%d\n", i, (uint8_t)getIntensity(i));
        }

        printf("\n----------[Space]-------------\n");
        for(int i=0; i<SIZE; i++) {
            for(int j=0; j<SIZE; j++) {
                printf("[%d,%d]:%d\n", i, j, (uint8_t)getSpace(i,j));
            }
        }
    }
};

}
}