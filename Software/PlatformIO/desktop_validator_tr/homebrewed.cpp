#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv;

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
        Mat sourceImage;
        capture >> sourceImage;
        if(sourceImage.empty()){
            printf("End of capture stream");
            break;
        }

        imshow("camera", sourceImage);

        int keyboard = waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;

    }
    return 0;
}
