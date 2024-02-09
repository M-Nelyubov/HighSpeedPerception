#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv;

#define IMAGE_ROWS 96
#define IMAGE_COLS 96

#define IMG_SIZE Size(IMAGE_COLS, IMAGE_ROWS)
#define DISP_SIZE Size(5*IMAGE_COLS, 5*IMAGE_ROWS)

#define IMAGE_SIZE (IMAGE_ROWS * IMAGE_COLS)
#define COLOR_CHANNELS 3


void matToFrameC(Mat mat, uint8_t frame[IMAGE_SIZE * COLOR_CHANNELS]){
    // Transfer pixel data from the image buffer to the 2D array
    for (int row = 0; row < IMAGE_ROWS; row++) {
        for (int col = 0; col < IMAGE_COLS; col++) {
            for (int c = 0; c < COLOR_CHANNELS; c++){
                int index = ((row * IMAGE_COLS) + col)* COLOR_CHANNELS + c;     // Calculate the index in the 1D buffer
                frame[index] = mat.data[index];          // Copy the pixel value to the 2D array and put a 1 if above threshold, otherwise 0
            }
        }
    }
}

void frameToMatG(Mat mat, uint8_t frame[IMAGE_ROWS * IMAGE_COLS]){
    // Transfer pixel data from the image buffer to the 2D array
    for (int row = 0; row < IMAGE_ROWS; row++) {
        for (int col = 0; col < IMAGE_COLS; col++) {
            int index = (row * IMAGE_COLS) + col;     // Calculate the index in the 1D buffer
            mat.data[index] = frame[index];          // Copy the pixel value to the 2D array and put a 1 if above threshold, otherwise 0
        }
    }
}


void zero(int8_t *data, int len){
    for(int i=0;i<len;i++){
        data[i]=0;
    }
}

int main(){
    VideoCapture capture(0); 

    if (!capture.isOpened()){
        //error in opening the video input
        printf("Unable to open file!");
        return 3;
    } else {
        printf("Starting camera feed!");
    }

    while (true) {
        Mat srcImg, smlCol, bigCol, smlGry, bigGry;
        capture >> srcImg;
        
        resize(srcImg, smlCol, IMG_SIZE);      // color small square
        resize(smlCol, bigCol, DISP_SIZE);     // color big   square
        cvtColor(smlCol, smlGry, COLOR_BGR2GRAY); // gray small square
        cvtColor(bigCol, bigGry, COLOR_BGR2GRAY); // gray small square

        Mat outputRedMask = bigGry.clone();  // the output mask of the red detection layer

        if(srcImg.empty()){
            printf("End of capture stream");
            break;
        }

        imshow("camera", srcImg);
        imshow("input", bigCol);
        imshow("Red mask", outputRedMask);

        int keyboard = waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;

    }
    return 0;
}
