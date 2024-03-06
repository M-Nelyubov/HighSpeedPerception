#ifndef cameraConfig_hpp
#define cameraConfig_hpp

#define CAMERA_FRAME_SIZE FRAMESIZE_96X96
// Frame names from sensor.h:
    // FRAMESIZE_96X96,    // 96x96
    // FRAMESIZE_QQVGA,    // 160x120
    // FRAMESIZE_QCIF,     // 176x144
    // FRAMESIZE_HQVGA,    // 240x176
    // FRAMESIZE_240X240,  // 240x240
    // FRAMESIZE_QVGA,     // 320x240 // largest 12.5 fps
    // FRAMESIZE_CIF,      // 400x296
    // FRAMESIZE_HVGA,     // 480x320
    // FRAMESIZE_VGA,      // 640x480
    // FRAMESIZE_SVGA,     // 800x600
    // FRAMESIZE_XGA,      // 1024x768
    // FRAMESIZE_HD,       // 1280x720
    // FRAMESIZE_SXGA,     // 1280x1024
    // FRAMESIZE_UXGA,     // 1600x1200


// Image Dimensions
#define IMAGE_COLS 96
#define IMAGE_ROWS 96
#define IMAGE_SIZE (IMAGE_ROWS * IMAGE_COLS)
#define COLOR_CHANNELS 3
#define IMAGE_WIDTH  IMAGE_COLS
#define IMAGE_HEIGHT IMAGE_ROWS
#define PIXEL_SIZE 2 // bytes



#endif
