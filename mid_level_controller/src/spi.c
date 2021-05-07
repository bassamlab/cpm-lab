// MIT License
// 
// Copyright (c) 2020 Lehrstuhl Informatik 11 - RWTH Aachen University
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// This file is part of cpm_lab.
// 
// Author: i11 - Embedded Software, RWTH Aachen University

#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "spi.h"
#include "../../low_level_controller/vehicle_atmega2560_firmware/crc.h"

/**
 * \file spi.c
 * \ingroup vehicle
 */

/**
 * \brief Checks whether the crc value provided by the low_level_controller is correct.
 * \param spi_miso_data Data, which has been sent from low_level_controller.
 * \ingroup vehicle
 */
static bool check_CRC_miso(spi_miso_data_t spi_miso_data) { 
    uint16_t mosi_CRC = spi_miso_data.CRC;
    spi_miso_data.CRC = 0;
    return mosi_CRC == crcFast((uint8_t*)(&spi_miso_data), sizeof(spi_miso_data_t));
}

/**
 * \brief The purpose of this variable is just to make sure that for-loop in busy wait
 *        is not optimized away.
 * \ingroup vehicle
 */
static volatile int dummy = 0;

/**
 * \brief Busy Wait in terms of for-loop cycles.
 * \param n The number of for-loop cycles which should be done to wait.
 * \ingroup vehicle
 */
static void busy_wait(int n) {
    for (int i = 0; i < n; ++i)
    {
        dummy++;
    }
}

void spi_init() {
    if (!bcm2835_spi_begin()) {
        printf("bcm2835_spi_begin failed. Are you running as root??\n");
        exit(EXIT_FAILURE);
    }

    crcInit();

    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);

    // enable CS pin
    bcm2835_gpio_fsel(RPI_GPIO_P1_24, BCM2835_GPIO_FSEL_OUTP);
    usleep(1000);

    // Default CS to high
    bcm2835_gpio_set(RPI_GPIO_P1_24);
    usleep(1000);
}


void spi_transfer(
    spi_mosi_data_t spi_mosi_data,
    spi_miso_data_t *spi_miso_data_out,
    int *n_transmission_attempts_out,
    int *transmission_successful_out
)
{
    *transmission_successful_out = 0;

    spi_mosi_data.CRC = crcFast((uint8_t*)(&spi_mosi_data), sizeof(spi_mosi_data_t));

    // CS low => transmission start
    bcm2835_gpio_clr(RPI_GPIO_P1_24);

    for (int i = 1; i < 4; ++i) // Try transmission 3 times at most
    {
        uint8_t SPI_recv_buffer[SPI_BUFFER_SIZE];
        uint8_t* mosi_data_ptr = (uint8_t*)(&spi_mosi_data);

        /*
        printf("\n\nDATA IN\n");
        printf("%d\n", (int)spi_mosi_data.pi_tick);
        printf("%d\n", (int)spi_mosi_data.motor_pwm);
        printf("%d\n", (int)spi_mosi_data.servo_command);
        printf("%d\n", (int)spi_mosi_data.CRC);
        printf("%d\n", (int)spi_mosi_data.motor_mode);
        printf("%d\n", (int)spi_mosi_data.vehicle_id);
        */
        usleep(200);
        //busy_wait(2000);

        for (int i = 0; i < SPI_BUFFER_SIZE; ++i)
        {
            SPI_recv_buffer[i] = bcm2835_spi_transfer(mosi_data_ptr[i]);
            usleep(300);
        }

        usleep(200);

        *n_transmission_attempts_out = i;

        memcpy(spi_miso_data_out, SPI_recv_buffer, sizeof(spi_miso_data_t));
        
        /*
        printf("\n\nDATA OUT\n");
        printf("%d\n", (int) spi_miso_data_out->tick );
        printf("%d\n", (int) spi_miso_data_out->odometer_steps );
        printf("%d\n", (int) spi_miso_data_out->imu_yaw );
        printf("%d\n", (int) spi_miso_data_out->imu_yaw_rate );
        printf("%d\n", (int) spi_miso_data_out->imu_acceleration_forward );
        printf("%d\n", (int) spi_miso_data_out->imu_acceleration_left );
        printf("%d\n", (int) spi_miso_data_out->imu_acceleration_up );
        printf("%d\n", (int) spi_miso_data_out->speed );
        printf("%d\n", (int) spi_miso_data_out->battery_voltage );
        printf("%d\n", (int) spi_miso_data_out->motor_current );
        printf("%d\n", (int) spi_miso_data_out->CRC );
        printf("%d\n", (int) spi_miso_data_out->status_flags );
        */

        if(check_CRC_miso(*spi_miso_data_out) && !(spi_miso_data_out->status_flags & 2))
        {
            *transmission_successful_out = 1;
            break;
        }
    }

    // CS high => transmission end
    bcm2835_gpio_set(RPI_GPIO_P1_24);
}
