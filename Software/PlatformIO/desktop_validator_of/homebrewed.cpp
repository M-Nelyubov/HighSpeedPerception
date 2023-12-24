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

void upscaleControl(int control[4], Mat ctrl_img){
    // ctrl_img is IMAGE_ROWS by IMAGE_COLS
    for (int row = 0; row < IMAGE_ROWS; row++) {
        for (int col = 0; col < IMAGE_COLS; col++) {
            int index = (row * IMAGE_COLS) + col;       // Calculate the index in the 1D buffer
            int screenHalf = (2* col) / IMAGE_COLS;    // which half of the screen to pull from
            int src = control[1+2*screenHalf];        // half to source
            ctrl_img.data[index] = 200 * src;  // brighten {0,1} to {0,200}
        }
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

    // imshow("Bigger", bigCol1);
    // waitKey(0);

    // // Used to determine new frame state
    // printf("(%d,%d)\r\n", prvs.rows, prvs.cols);

    auto p_frame = new uint8_t [IMAGE_ROWS * IMAGE_COLS];   // NxM bytes
    auto n_frame = new uint8_t [IMAGE_ROWS * IMAGE_COLS];   // NxM bytes
    auto corners = new uint8_t [IMAGE_ROWS * IMAGE_COLS];   // NxM bytes

    Mat control_sig = smlCol1.clone(); // todo

    Mat cvUOut = bigCol1.clone();
    Mat cvVOut = bigCol1.clone();

    Mat cornerOut = smlCol1.clone();
    Mat uOut = smlCol1.clone();
    Mat vOut = smlCol1.clone();

    Mat cornerBig = bigCol1.clone();
    Mat uBig = bigCol1.clone();
    Mat vBig = bigCol1.clone();
    auto u_frame = new int16_t [IMAGE_ROWS * IMAGE_COLS];   // NxM bytes
    auto v_frame = new int16_t [IMAGE_ROWS * IMAGE_COLS];   // NxM bytes

    auto motor_rule = new int16_t [IMAGE_COLS * IMAGE_ROWS];
    Mat activation_mat = smlCol1.clone();
    Mat big_activation_mat = bigCol1.clone();
    Mat ctrl_img = prvs.clone();  // greyscale
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


        findCorners(p_frame, corners);
        frameToMat(cornerOut,corners);
        resize(cornerOut, cornerBig, DISP_SIZE,0,0,INTER_NEAREST);
        imshow("corners", cornerBig);

        computeFlow(p_frame, n_frame, u_frame, v_frame);


        Mat flow(prvs.size(), CV_32FC2);
        openCV_OF(prvs, next, flow);


        auto control = new int[4];
        for(int i=0; i<IMAGE_COLS * IMAGE_ROWS;i++){
            motor_rule[i] = 20*rule(u_frame[i], v_frame[i]);
        }
        frameToMat(activation_mat, motor_rule);
        resize(activation_mat, big_activation_mat, DISP_SIZE,0,0, INTER_NEAREST);
        
        motorControl(u_frame, v_frame, control);
        upscaleControl(control, ctrl_img);


        // Convert homebrewed version back to OCV Matrix for display
        frameToMat(uOut, u_frame);
        frameToMat(vOut, v_frame);

        resize(uOut, uBig, DISP_SIZE,0,0,INTER_NEAREST);
        resize(vOut, vBig, DISP_SIZE,0,0,INTER_NEAREST);
        resize(next, inRep, DISP_SIZE,0,0,INTER_NEAREST);
        imshow("frame U", uBig);
        imshow("frame V", vBig);
        imshow("source", frame2);
        imshow("input", inRep);
        imshow("activation layer", big_activation_mat);
        imshow("Control signals", ctrl_img);

        int keyboard = waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;

        prvs = next;
    }

}
