// built off of: https://learnopencv.com/optical-flow-in-opencv/

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include "optical_flow.hpp"
#include <ctime>

using namespace cv;

#define IMG_SIZE Size(IMAGE_COLS, IMAGE_ROWS)
#define DISP_SIZE Size(5*IMAGE_COLS, 5*IMAGE_ROWS)

int hiClips = 0;
int loClips = 0;

uint8_t clip(int16_t data){
    if (data > 0){
        hiClips++;
        return 0xff;
    }
    if (data < 0){
        loClips++;
        return 0;
    }
    return (int8_t) data;
}

void matToFrame(Mat mat, uint8_t frame[IMAGE_ROWS * IMAGE_COLS]){
    // Transfer pixel data from the image buffer to the 2D array
    for (int row = 0; row < IMAGE_ROWS; row++) {
        for (int col = 0; col < IMAGE_COLS; col++) {
            int index = (row * IMAGE_COLS) + col;     // Calculate the index in the 1D buffer
            frame[index] = mat.data[index];          // Copy the pixel value to the 2D array and put a 1 if above threshold, otherwise 0
        }
    }
}
void frameToMat(Mat mat, int16_t frame[IMAGE_ROWS * IMAGE_COLS]){
    // Transfer pixel data from the 2D array to the image buffer
    // mat = ;
    // cout << "Attempting to convert u,v data to CV matrix" << endl;
    hiClips = loClips = 0;
    for (int row = 0; row < IMAGE_ROWS; row++) {
        for (int col = 0; col < IMAGE_COLS; col++) {
            int index = (row * IMAGE_COLS) + col;        // Calculate the index in the 1D buffer
            int16_t d = frame[index];
            uint8_t r,g,b;
            int shamt = 3; // x pixel intensity: 2^shamt
            if(d<0){ // negative
                b = 0xFF + (d<<shamt);
                g = 0xFF + (d<<shamt);
                r = 0xFF;
            }
            if(d>0){ // positive
                b = 0xFF;
                g = 0xFF - (d<<shamt);
                r = 0xFF - (d<<shamt);
            }
            if(d==0){ // no flow --> white
                r=g=b=0xFF;
            }

            // Error case: flow is more than some large threshold
            int largeNum = 20;
            if(d > largeNum || d < -largeNum){
                r=b=0;
                g = 0xFF;
                printf("Strange reading: ry:%d\tcx:%d\tmag:%d\n", row,col,d);
            }
            mat.data[3*index+0] = b;clip(d); // Copy the pixel value to the 2D array and put a 1 if above threshold, otherwise 0
            mat.data[3*index+1] = g;clip(d); // Copy the pixel value to the 2D array and put a 1 if above threshold, otherwise 0
            mat.data[3*index+2] = r;clip(d); // Copy the pixel value to the 2D array and put a 1 if above threshold, otherwise 0
        }
    }
    // printf("Hi Clips: %d\tLo Clips: %d\n", hiClips, loClips);
}


int main(){
    fprintf(stderr, "check\n");
    cout << "Entered main()!" << endl;
    VideoCapture capture(0); 
    if (!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open file!" << endl;
        return 3;
    } else {
        cout << "Starting camera feed!" << endl;
    }

    Mat frame1, small, prvs, bigOut, repr;
    capture >> frame1;                      // color big   original aspect ratio
    resize(frame1, small, IMG_SIZE);        // color small square
    resize(small, bigOut, DISP_SIZE);       // color big   square
    cvtColor(small, prvs, COLOR_BGR2GRAY);  // grey  small square
    cvtColor(bigOut, repr, COLOR_BGR2GRAY); // grey  big   square

    // imshow("Bigger", bigOut);
    // waitKey(0);

    // // Used to determine new frame state
    // printf("(%d,%d)\r\n", prvs.rows, prvs.cols);

    auto p_frame = new uint8_t [IMAGE_ROWS * IMAGE_COLS];   // 480x640 bytes
    auto n_frame = new uint8_t [IMAGE_ROWS * IMAGE_COLS];   // 480x640 bytes

    Mat uOut = small.clone();
    Mat vOut = small.clone();
    Mat uBig = bigOut.clone();
    Mat vBig = bigOut.clone();
    auto u_frame = new int16_t [IMAGE_ROWS * IMAGE_COLS];   // 480x640 bytes
    auto v_frame = new int16_t [IMAGE_ROWS * IMAGE_COLS];   // 480x640 bytes
    // initZero(u_frame);
    // initZero(v_frame);

    while(true){
        Mat frame2, grey2, next, inRep;
        time_t t = time(0);
        // cout << "Capturing frame at t=" << t << endl;    // fps test.  For HAL-9000, fps=20
        capture >> frame2;
        if (frame2.empty()){
            cout << "End of content" << endl;
            break;
        }
        cvtColor(frame2, grey2, COLOR_BGR2GRAY);
        resize(grey2, next, IMG_SIZE);
        
        // convert the matrixes into 2d arrays for basic processing
        matToFrame(prvs, p_frame);
        matToFrame(next, n_frame);

        computeFlow(p_frame, n_frame, u_frame, v_frame);

        // Convert back to OCV Matrix for display
        frameToMat(uOut, u_frame);
        frameToMat(vOut, v_frame);

        resize(uOut, uBig, DISP_SIZE,0,0,INTER_NEAREST);
        resize(vOut, vBig, DISP_SIZE,0,0,INTER_NEAREST);
        resize(next, inRep, DISP_SIZE,0,0,INTER_NEAREST);
        imshow("frame U", uBig);
        imshow("frame V", vBig);
        imshow("source", frame2);
        imshow("input", inRep);

        int keyboard = waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;

        prvs = next;
    }

}
