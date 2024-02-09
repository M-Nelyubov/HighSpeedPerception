#include "object_recognition.hpp"

#define RED_CHANNEL_IDX 2
#define RST 1.2f // red significance threshold - how much red has to be greater than the pixel brightness magnitude to count as significant

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

        outputFrame[oi] = (red > RST * mag) ? (red - RST * mag) : 0;
    }
}
