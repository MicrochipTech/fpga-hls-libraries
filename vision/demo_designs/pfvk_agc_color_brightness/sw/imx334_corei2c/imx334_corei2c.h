/*
 * imx334_corei2c.h
 *
 *  Created on: 18-Feb-2019
 *      Author: I30392
 */

#ifndef APPLICATION_IMX334_COREI2C_IMX334_COREI2C_H_
#define APPLICATION_IMX334_COREI2C_IMX334_COREI2C_H_

#include "common/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*Pattern generator in camera - 1-enabled/0-disabled*/
/*If enabled, overrides all other camera settings*/
#define CAMERA_PATTERN_GEN_EN 0

/*Camera Configuration-
 * 0 - 500Mbps data rate, 1080p resolution
 * 1 - 1188Mbps data rate, 4k resolution */
#define CAM_CONFIG_4K_1_2M	1

/*Number of lane
* 1 - four lane
* 0 - eight lane  */
#define CAM_CONFIG_lane 1

#define CAM2_RST GPIO_0
#define CAM2_CLK_EN GPIO_1

/*I2C Addresses*/
#define IMX334_1_DEV_REG  (0x1A )	//After adjusting for shift
#define IMX334_2_DEV_REG  (0x10)	//After adjusting for shift

void imx334_cam_reginit();
void imx334_cam_init();
void gain_setting(uint16_t in_gain);

#endif /* APPLICATION_IMX334_COREI2C_IMX334_COREI2C_H_ */

#ifdef __cplusplus
}
#endif
