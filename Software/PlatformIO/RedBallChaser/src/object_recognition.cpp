#include "object_recognition.hpp"

#define RED_CHANNEL_IDX 2
#define RST 1.3f // red significance threshold - how much red has to be greater than the pixel brightness magnitude to count as significant

void extractRed(uint8_t inputFrame [IMAGE_SIZE * COLOR_CHANNELS], uint8_t outputFrame [IMAGE_SIZE]){
    for(int i=0; i<IMAGE_SIZE; i++){
        int ii = i * COLOR_CHANNELS;  // input index
        int oi = i;                   // ouptut index

        // Calculate greyscale pixel value
        int mag = 0;
        for(int c=0;c<COLOR_CHANNELS;c++){
            mag += inputFrame[ii+c];
        }
        mag /= COLOR_CHANNELS;
        int red = inputFrame[ii + RED_CHANNEL_IDX];

        outputFrame[oi] = (red > RST * mag) ? (red - 0 * mag) : 0;
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
