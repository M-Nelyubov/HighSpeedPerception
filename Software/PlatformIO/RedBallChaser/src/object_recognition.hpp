#include <iostream>
#include <stdio.h>
#include "camera_config.hpp"

void extractRed(uint8_t inputFrame [IMAGE_SIZE * COLOR_CHANNELS], uint8_t outputFrame [IMAGE_SIZE]);

// int computeCentroidX(uint8_t outputFrame [IMAGE_SIZE]);
void computeCentroidX(uint8_t magnitudeFrame [IMAGE_SIZE], float *x, float *magIn);
