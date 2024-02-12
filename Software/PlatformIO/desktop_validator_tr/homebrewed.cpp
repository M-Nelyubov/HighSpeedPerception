#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <iostream>
#include <stdio.h>

#include "object_recognition.hpp"

using namespace cv;

#define IMG_SIZE Size(IMAGE_COLS, IMAGE_ROWS)
#define DISP_SIZE Size(5*IMAGE_COLS, 5*IMAGE_ROWS)



void matToFrameC(Mat mat, uint8_t frame[IMAGE_SIZE * COLOR_CHANNELS]){
    // Transfer pixel data from the image buffer to the 2D array
    for(int i=0; i< IMAGE_SIZE * COLOR_CHANNELS; i++){
        frame[i] = mat.data[i];
    }
    // for (int row = 0; row < IMAGE_ROWS; row++) {
    //     for (int col = 0; col < IMAGE_COLS; col++) {
    //         for (int c = 0; c < COLOR_CHANNELS; c++){
    //             int index = ((row * IMAGE_COLS) + col)* COLOR_CHANNELS + c;     // Calculate the index in the 1D buffer
    //             frame[index] = mat.data[index];          // Copy the pixel value to the 2D array and put a 1 if above threshold, otherwise 0
    //         }
    //     }
    // }
}

void frameToMatG(Mat mat, uint8_t frame[IMAGE_ROWS * IMAGE_COLS]){
    // Transfer pixel data from the image buffer to the 2D array
    for (int row = 0; row < IMAGE_ROWS; row++) {
        for (int col = 0; col < IMAGE_COLS; col++) {
            int index = (row * IMAGE_COLS) + col;     // Calculate the index in the 1D buffer
            mat.data[index] = frame[index];
        }
    }
}

void frameToMatC(Mat mat, uint8_t frame[IMAGE_ROWS * IMAGE_COLS]){
    // Transfer pixel data from the image buffer to the 2D array
    for (int row = 0; row < IMAGE_ROWS; row++) {
        for (int col = 0; col < IMAGE_COLS; col++) {
            int index = (row * IMAGE_COLS) + col;     // Calculate the index in the 1D buffer
            mat.data[COLOR_CHANNELS*index+0] = frame[index];          // B
            mat.data[COLOR_CHANNELS*index+1] = frame[index];          // G
            mat.data[COLOR_CHANNELS*index+2] = frame[index];          // R
        }
    }
}

void drawCentroid(Mat mask, int x){
    int col = x;
    for(int row = 0; row < IMAGE_ROWS; row++){
        int index = COLOR_CHANNELS *( (row * IMAGE_COLS) + col) + 1; // (1) green channel
        mask.data[index] = 0x80;                   // max brightness
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

    auto inputFrame = new uint8_t [IMAGE_SIZE * COLOR_CHANNELS];
    auto outputFrame = new uint8_t [IMAGE_SIZE];

    while (true) {
        // Initialization and image capture
        Mat srcImg, smlCol, bigCol, smlGry, bigGry;
        capture >> srcImg;
        
        resize(srcImg, smlCol, IMG_SIZE);      // color small square
        resize(smlCol, bigCol, DISP_SIZE);     // color big   square
        cvtColor(smlCol, smlGry, COLOR_BGR2GRAY); // gray small square
        cvtColor(bigCol, bigGry, COLOR_BGR2GRAY); // gray small square

        Mat outputRedMask = smlCol.clone();  // the output mask of the red detection layer
        Mat displayRedMask = bigCol.clone();  // the output mask of the red detection layer

        if(srcImg.empty()){
            printf("End of capture stream");
            break;
        }


        // Processing
        matToFrameC(smlCol, inputFrame);
        extractRed(inputFrame, outputFrame);
        frameToMatC(outputRedMask, outputFrame);

        int x = computeCentroidX(outputFrame);
        printf("Drawing centroid at %d\t",x);
        drawCentroid(outputRedMask, x);

        // Display
        resize(outputRedMask, displayRedMask, DISP_SIZE,0,0,INTER_NEAREST);


        imshow("camera", srcImg);
        imshow("input", bigCol);
        imshow("Red mask", displayRedMask);


        // End of cycle wait for reset
        int keyboard = waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;

    }
    return 0;
}
