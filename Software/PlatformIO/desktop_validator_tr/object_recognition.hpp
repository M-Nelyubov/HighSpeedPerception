#include <iostream>
#include <stdio.h>

#define IMAGE_ROWS 96
#define IMAGE_COLS 96
#define IMAGE_SIZE (IMAGE_ROWS * IMAGE_COLS)
#define COLOR_CHANNELS 3

void extractRed(uint8_t inputFrame [IMAGE_SIZE * COLOR_CHANNELS], uint8_t outputFrame [IMAGE_SIZE]);

void computeCentroidX(uint8_t magnitudeFrame [IMAGE_SIZE], float *x, float *magIn);

void clusterPatches(uint8_t red_islands_frame[IMAGE_SIZE], uint8_t red_clusters_frame[IMAGE_SIZE]);
