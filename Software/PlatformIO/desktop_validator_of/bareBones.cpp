// built off of: https://learnopencv.com/optical-flow-in-opencv/

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include "optical_flow.hpp"
#include "motor_control.hpp"
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
                b = 0xD0 + (d<<shamt);
                g = 0xD0 + (d<<shamt);
                r = 0xFF;
            }
            if(d>0){ // positive
                b = 0xFF;
                g = 0xD0 - (d<<shamt);
                r = 0xD0 - (d<<shamt);
            }
            if(d==0){ // no flow --> white
                r=g=b=0xFF;
            }

            // Error case: flow is more than some large threshold
            int largeNum = 20;
            if(d > largeNum || d < -largeNum){
                r=b=0;
                g = 0xFF;
                // printf("Strange reading: ry:%d\tcx:%d\tmag:%d\n", row,col,d);
            }
            mat.data[3*index+0] = b;clip(d); // Copy the pixel value to the 2D array and put a 1 if above threshold, otherwise 0
            mat.data[3*index+1] = g;clip(d); // Copy the pixel value to the 2D array and put a 1 if above threshold, otherwise 0
            mat.data[3*index+2] = r;clip(d); // Copy the pixel value to the 2D array and put a 1 if above threshold, otherwise 0
        }
    }
    // printf("Hi Clips: %d\tLo Clips: %d\n", hiClips, loClips);
}

void frameToMat(Mat mat, uint8_t frame[IMAGE_ROWS * IMAGE_COLS]){
    auto proxy_frame = new int16_t [IMAGE_COLS * IMAGE_ROWS];
    for(int i=0; i<IMAGE_ROWS * IMAGE_COLS; i++) proxy_frame[i] = frame[i] * 20; // scale up for visibility on output render
    frameToMat(mat, proxy_frame);
    delete proxy_frame; 
}

void zero(int8_t *data, int len){
    for(int i=0;i<len;i++){
        data[i]=0;
    }
}

void upscaleControl(int control[2], Mat ctrl_img){
    // ctrl_img is IMAGE_ROWS by IMAGE_COLS

    for (int row = 0; row < IMAGE_ROWS; row++) {
        for (int col = 0; col < IMAGE_COLS; col++) {
            int index = (row * IMAGE_COLS) + col;     // Calculate the index in the 1D buffer
            int screenHalf = (2* col) / IMAGE_COLS;   // which half of the screen to pull from
            int src = control[screenHalf];            // source data
            int pow_ref = (IMAGE_ROWS/2 - row) * 100 / IMAGE_ROWS;   // power reference on the scale of {-100, 100}
            ctrl_img.data[index] = (src > pow_ref) ? 255 : 0;   // indicate motor power
        }
    }
    // draw median line
    int row = IMAGE_ROWS/2;
    for (int col = 0; col < IMAGE_COLS; col++) {
        int index = (row * IMAGE_COLS) + col;     // Calculate the index in the 1D buffer
        ctrl_img.data[index] = 0x80; //half-power zero line
    }
}

void openCV_OF(Mat prvs, Mat next, Mat flow){
    // https://learnopencv.com/optical-flow-in-opencv/
    // OpenCV Version of compute flow
    // calcOpticalFlowPyrLK(prvs, next, flow), ;
    calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

    Mat flow_parts[2];
    split(flow, flow_parts);

    // from of-demo
    Mat magnitude, angle, magn_norm;
    cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
    normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);
    angle *= ((1.f / 360.f) * (180.f / 255.f));

    //build hsv image
    Mat _hsv[3], hsv, hsv8, bgr;
    _hsv[0] = angle;
    _hsv[1] = Mat::ones(angle.size(), CV_32F);
    _hsv[2] = magn_norm;
    merge(_hsv, 3, hsv);
    hsv.convertTo(hsv8, CV_8U, 255.0);
    cvtColor(hsv8, bgr, COLOR_HSV2BGR);

    Mat bigger;
    resize(bgr, bigger, DISP_SIZE,0,0,INTER_NEAREST);
    imshow("frame2", bigger);


    // resize(flow_parts[0], cvUOut, DISP_SIZE,0,0,INTER_NEAREST);
    // resize(flow_parts[1], cvVOut, DISP_SIZE,0,0,INTER_NEAREST);

    // imshow("OpenCV U", cvUOut);
    // imshow("OpenCV V", cvVOut);
    ///////////////////////////////////////////////////// end of of-demo version
}

void loadFsToBuffer(int i, Mat m, uint8_t frame[IMAGE_ROWS * IMAGE_COLS]){
    // printf("Looking for file %d\n", i);
    char path[64];
    sprintf(path, "D:/img/img_%d.png", i);
    Mat srcImg0 = imread(path, IMREAD_GRAYSCALE);
    // imshow("file", srcImg0);
    for(int i=0; i< srcImg0.rows * srcImg0.cols; i++){
        m.data[3*i+0] = srcImg0.data[i];
        m.data[3*i+1] = srcImg0.data[i];
        m.data[3*i+2] = srcImg0.data[i];
        frame[i] = srcImg0.data[i];
    }
    // matToFrame(srcImg0, buffer);
}

int imgIdx=0;



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

    Mat srcImg1, smlCol1, prvs, bigCol1, repr;
    capture >> srcImg1;                      // color big   original aspect ratio
    resize(srcImg1, smlCol1, IMG_SIZE);      // color small square
    resize(smlCol1, bigCol1, DISP_SIZE);     // color big   square
    cvtColor(smlCol1, prvs, COLOR_BGR2GRAY); // grey  small square
    cvtColor(bigCol1, repr, COLOR_BGR2GRAY); // grey  big   square

    Mat uOut = smlCol1.clone();
    Mat uBig = bigCol1.clone();
    Mat ctrl_img = prvs.clone();  // greyscale


    Mat square_template; 
    resize(srcImg1, square_template, IMG_SIZE);      // color small square
    Mat f1, f2;
    resize(srcImg1, f1, IMG_SIZE);      // color small square
    resize(srcImg1, f2, IMG_SIZE);      // color small square

    auto p_frame = new uint8_t [IMAGE_ROWS * IMAGE_COLS];   // NxM bytes
    auto n_frame = new uint8_t [IMAGE_ROWS * IMAGE_COLS];   // NxM bytes
    auto corners = new uint8_t [IMAGE_ROWS * IMAGE_COLS];   // NxM bytes

    auto u_frame = new int16_t [IMAGE_ROWS * IMAGE_COLS];   // NxMx2 bytes
    auto v_frame = new int16_t [IMAGE_ROWS * IMAGE_COLS];   // NxMx2 bytes

    auto control = new int[2];
    control[0] = control[1] = 80; // magic number, max pow, duty cycle time

    int i=0;
    while(true){
        // Load Cycle
        capture >> srcImg1;                      // color big   original aspect ratio
        loadFsToBuffer(i,f1, p_frame);
        i = (i+1);//%30;
        if(i >38) return 0;
        loadFsToBuffer(i,f2, n_frame);


        // Computation
        findCorners(p_frame, corners);
        computeFlow(p_frame, n_frame, u_frame, v_frame, corners);
        motorControl(u_frame, v_frame, control);


        // Prepare Presentation
        frameToMat(uOut, u_frame);
        resize(uOut, uBig, DISP_SIZE,0,0,INTER_NEAREST);

        printf("L:%d R:%d\n", control[0], control[1]);
        upscaleControl(control, ctrl_img);


        // Presentation
        imshow("input", srcImg1);
        imshow("data", f2);
        imshow("frame U", uBig);
        imshow("Control signals", ctrl_img);

        // Wait between cycles
        int keyboard = waitKey(100);
        if (keyboard == 'q' || keyboard == 27)
            break;
    }
}
