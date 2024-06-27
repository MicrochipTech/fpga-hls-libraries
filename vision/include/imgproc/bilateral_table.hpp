#pragma once

#include "hls/ap_fixpt.hpp"
#include "hls/ap_int.hpp"
#include "math.h"

namespace hls {
namespace vision {

template <unsigned RANGE> struct GaussianIntensityTable {
    ap_int<8> Table[RANGE];
    GaussianIntensityTable(float sigma) {
        float max_value = 0;

        for (int i = 0; i < RANGE; i++) {
            const float tmp = Gaussian(i, sigma);
            max_value = fmax(tmp, max_value);
        }
        float factor = (255.0f / max_value);

        // Scale and round the table values
        for (int i = 0; i < RANGE; i++) {
            Table[i] = (uint8_t)(Gaussian(i, sigma) * factor + 0.5f);
        }
        
    };

    float __attribute__((always_inline)) Gaussian(int x, float sigma) {
        #pragma HLS function inline
        return exp(-(pow(x, 2)) / (2 * pow(sigma, 2))) /
               (2 * M_PI * pow(sigma, 2));
    }

    uint8_t getIntensity(int i) const { return Table[i]; }

    void printTables() const {
        printf("\n----------[Intensity]-------------\n");
        for(int i=0; i<RANGE; i++) {
            printf("%d:%d\n", i, (uint8_t)getIntensity(i));
        }
    }
};

template <unsigned SIZE> struct GaussianSpaceTable {
    ap_int<8> Table[SIZE][SIZE];
    int SIZE_DIV = (SIZE - 1) / 2;
    GaussianSpaceTable(float sigma) {
        float factor = 256 * 1024;
        // #pragma unroll
        for (int i = 0; i < SIZE; i++) {
            #pragma unroll
            for (int j = 0; j < SIZE; j++) {
              Table[i][j] = (uint8_t)(Gaussian(i, j, sigma) * factor + 0.5f);
            }
        }
    };

    float __attribute__((always_inline)) Gaussian(int x, int y, float sigma) {
        #pragma HLS function inline
        return exp(-(pow(x - SIZE_DIV, 2) + pow(y - SIZE_DIV, 2)) / (2 * pow(sigma, 2))) /
               (2 * M_PI * pow(sigma, 2));
    }

    uint8_t getSpace(int i, int j) const { return Table[i][j]; }

    void printTables() const {
        printf("\n----------[Space]-------------\n");
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                printf("[%d,%d]:%d\n", i, j, (uint8_t)getSpace(i, j));
            }
        }
    }
};

}
}
