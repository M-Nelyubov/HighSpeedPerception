#include "object_recognition.hpp"

#define BCI 0   // blue channel index
#define GCI 1   // green channel index
#define RCI 2   // red channel index

void extractRed(uint8_t inputFrame [IMAGE_SIZE * COLOR_CHANNELS], uint8_t outputFrame [IMAGE_SIZE]){
    for(int i=0; i<IMAGE_SIZE; i++){
        int ii = i * COLOR_CHANNELS;  // input index
        int oi = i;                   // ouptut index

        int r = inputFrame[ii + RCI];
        int g = inputFrame[ii + GCI];
        int b = inputFrame[ii + BCI];

        outputFrame[oi] = (r > g+b) ? (r - (3*(g+b)/4)) : 0;
    }
}

void computeCentroidX(uint8_t magnitudeFrame [IMAGE_SIZE], float *x, float *magIn){
    /**
     * Output pointers to variables x and magnitude
    */
    float xfx = 0;
    float mag = 0.1f;
    for (int row = 0; row < IMAGE_ROWS; row++) {
        for (int col = 0; col < IMAGE_COLS; col++) {
            int index = (row * IMAGE_COLS) + col;     // Calculate the index in the 1D buffer
            xfx += col * magnitudeFrame[index];
            mag += magnitudeFrame[index];
        }
    }

    x[0] = (xfx / mag);
    magIn[0] = mag;
}
