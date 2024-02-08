#include "optical_flow.hpp"
#include <iostream>

#define max(a,b) ((a>b)?a:b)
#define min(a,b) ((a<b)?a:b)

void generatePyramidLayer(uint8_t *frame, uint8_t *shrunkFrame, int rows, int cols);
uint8_t **generatePyramid(uint8_t frame[IMAGE_ROWS * IMAGE_COLS], int pyramidSize);
void destroyPyramid(uint8_t** pyramid, int pyramidSize);
int getPyramidSize(uint8_t frame[IMAGE_ROWS * IMAGE_COLS]);
void drawPyramid(uint8_t **pyramid, int16_t *canvas, int layer, int r_offset=0, int c_offset=0, uint8_t bias=0x80);

void drawFullyStackedPyramid(uint8_t **n_pyramid, int16_t *u_frame, int layers = 8);
void computeGrad(uint8_t *p_frame, uint8_t *n_frame, int8_t *gradient, int rows, int cols, int dx, int dy, int dt);

void computeUV(uint8_t *p_frame, uint8_t *n_frame, int16_t *u_frame, int16_t *v_frame, int rows, int cols);
void solveMatrixATAATb(int16_t sIxx, int16_t sIxy, int16_t sIyy, int16_t sIxt, int16_t sIyt, int8_t *uv);

void augment_frame(uint8_t *frame, int16_t *u_augment_mask, int16_t *v_augment_mask, int rows, int cols);

void initZero(uint8_t *frame, int rows, int cols);
void initZero(int16_t *frame, int rows, int cols);

void upscale(int16_t *src, int16_t *dst, int src_rows, int src_cols);
void printComposition(int16_t *data, int length);

void computeUVpyramid   (uint8_t **p_pyramid, uint8_t **n_pyramid, int16_t *u_frame, int16_t *v_frame, int rows=IMAGE_ROWS, int cols=IMAGE_COLS, int depth=PYRAMID_DEPTH, int layer=0);
void computeUVlayerBelow(uint8_t **p_pyramid, uint8_t **n_pyramid, int16_t *u_frame, int16_t *v_frame, int rows=IMAGE_ROWS, int cols=IMAGE_COLS, int depth=PYRAMID_DEPTH, int layer=0);

void computeFlow(
    uint8_t p_frame[IMAGE_ROWS * IMAGE_COLS], 
    uint8_t n_frame[IMAGE_ROWS * IMAGE_COLS], 
    int16_t u_frame[IMAGE_ROWS * IMAGE_COLS], 
    int16_t v_frame[IMAGE_ROWS * IMAGE_COLS],
    uint8_t corners[IMAGE_ROWS * IMAGE_COLS]
){
    initZero(u_frame, IMAGE_ROWS, IMAGE_COLS);
    initZero(v_frame, IMAGE_ROWS, IMAGE_COLS);

    // // temporary initialization for the sake of having something to show on U,V
    // for (int row = 0; row < IMAGE_ROWS; row++) {
    //     for (int col = 0; col < IMAGE_COLS; col++) {
    //         u_frame[row*IMAGE_COLS + col] = p_frame[row*IMAGE_COLS + col]/2 - 0x40;
    //         v_frame[row*IMAGE_COLS + col] = n_frame[row*IMAGE_COLS + col] - 0x80;
    //     }
    // }

    int pyramid_size = getPyramidSize(n_frame);
    uint8_t **n_pyramid = generatePyramid(n_frame, pyramid_size);
    uint8_t **p_pyramid = generatePyramid(p_frame, pyramid_size);

    // // Diagnosing that the pyramid works
    // drawFullyStackedPyramid(n_pyramid, u_frame);
    // printf("Entering the great pyramid\n");
    computeUVpyramid(p_pyramid,n_pyramid, u_frame, v_frame);

    // Cleanup
    destroyPyramid(n_pyramid, pyramid_size);
    destroyPyramid(p_pyramid, pyramid_size);

    // mask - disabling UV for all non-corner values
    for(int i=0; i< IMAGE_COLS*IMAGE_ROWS;i++){
        if(corners[i]*u_frame[i] || corners[i]*v_frame[i]){
            // printf("OF_%d:[u%d,v%d,c%d] ", i, u_frame[i], v_frame[i], corners[i]);
        }
        u_frame[i] *= corners[i];
        v_frame[i] *= corners[i];
    }
}

void computeUVpyramid(uint8_t **p_pyramid, uint8_t **n_pyramid, int16_t *u_frame, int16_t *v_frame, int rows, int cols, int depth, int layer){
    computeUVlayerBelow(p_pyramid, n_pyramid, u_frame, v_frame, rows, cols, depth, layer+1);
    // printf("Computing top layer of pyramid\n");
    computeUV(p_pyramid[0], n_pyramid[0], u_frame, v_frame, rows, cols);
}

void computeUVlayerBelow(uint8_t **p_pyramid, uint8_t **n_pyramid, int16_t *u_frame, int16_t *v_frame, int rows, int cols, int depth, int layer){
    if(layer > depth) return;
    // printf("Going up, %d\n",layer);

    int sr = rows / (2);  // scaled-down row
    int sc = cols / (2);  // scaled-down col

    // Compute gradient for a layer of the pyramid
    auto mini_u_frame = new int16_t [sr * sc];
    auto mini_v_frame = new int16_t [sr * sc];
    initZero(mini_u_frame, sr, sc);
    initZero(mini_v_frame, sr, sc);

    // Recursive call time
    computeUVlayerBelow(p_pyramid, n_pyramid, mini_u_frame, mini_v_frame, sr, sc, depth, layer+1);
    // printf("Coming down, computing layer: %d\t dims: [%d,%d]\n", layer, rows, cols);

    // cout << "Computing layer 1 UV" << endl;
    computeUV(p_pyramid[layer], n_pyramid[layer], mini_u_frame, mini_v_frame, sr, sc);
    // printf("Getting composition of matrix: mini_u_frame\n");
    // printComposition(mini_u_frame, sr * sc);
    // cout << "Upscaling layer 1 UV" << endl;
    upscale(mini_u_frame, u_frame, sr, sc);
    upscale(mini_v_frame, v_frame, sr, sc);
    augment_frame(p_pyramid[layer-1], u_frame, v_frame, rows, cols);
    delete mini_u_frame, mini_v_frame;
}

void printComposition(int16_t *data, int length){
    int hi,lo,zero;
    hi = lo = zero = 0;

    for(int i = 0; i<length; i++){
        if(data[i] == 0){
            zero++;
            continue;
        }
        if(data[i] > 0){
            hi++;
        }
        if(data[i] < 0){
            lo++;
        }
    }
    printf("Hi: %d\tLo: %d\t0: %d\n", hi,lo,zero);
}

void upscale(int16_t *src, int16_t *dst, int src_rows, int src_cols){
    // Scales up the flow computed in a small frame up to a quadrupled frame (x2 in each dim) with the flow contribution doubled
    for(int r = 0; r< src_rows; r++){
        for (int c=0; c< src_cols; c++){
            int i = r*src_cols + c;
            int cols = 2 * src_cols;
            dst[(2*r + 0)*cols + 2*c+0] = 2*src[i];
            dst[(2*r + 0)*cols + 2*c+1] = 2*src[i];
            dst[(2*r + 1)*cols + 2*c+0] = 2*src[i];
            dst[(2*r + 1)*cols + 2*c+1] = 2*src[i];
        }
    }
}

void augment_frame(uint8_t *frame, int16_t *u_augment_mask, int16_t *v_augment_mask, int rows, int cols) {
    auto standin_frame = new uint8_t [rows * cols];
    // fill in standin with source
    for(int i=0;i<rows*cols;i++){
        standin_frame[i] = frame[i];
    }

    // copy standin back into pyramid with the augment
    for(int r=0; r<rows;r++){
        for(int c=0; c<cols; c++){
            int sfi = r*cols + c;  // stand-in frame index
            int ra = (r + v_augment_mask[sfi]); // row-augmented index
            int ca = (c + u_augment_mask[sfi]); // col-augmented index
            int pid = ra*cols + ca;       //pyramid index
            // move the data if it doesn't move it out of frame
            if(ra >= 0 && ca >= 0 && ra < rows && ca < cols){
                frame[pid] = standin_frame[sfi];
                // cout << "Moved point" << pid << "to location" << sfi << endl;
            }
        }
    }

    delete standin_frame;
}

void computeUV(uint8_t *p_frame, uint8_t *n_frame, int16_t *u_frame, int16_t *v_frame, int rows, int cols){
    // printf("Computing U,V on (%dx%d) frame.\n",rows,cols);
    // printf("Allocating gradient matrices\n");
    auto grad_x = new int8_t [rows * cols];
    auto grad_y = new int8_t [rows * cols];
    auto grad_t = new int8_t [rows * cols];
    
    // printf("Computing gradients\n");
    computeGrad(p_frame, n_frame, grad_x, rows, cols, 1,0,0);
    computeGrad(p_frame, n_frame, grad_y, rows, cols, 0,1,0);
    computeGrad(p_frame, n_frame, grad_t, rows, cols, 0,0,1);
    
    // printf("Allocating product storage\n");
    auto IxIx = new int16_t [rows * cols];
    auto IxIy = new int16_t [rows * cols];
    auto IyIy = new int16_t [rows * cols];
    auto IxIt = new int16_t [rows * cols];
    auto IyIt = new int16_t [rows * cols];
    
    
    // printf("Calculating products\n");
    for(int i=0; i<rows * cols; i++){
        IxIx[i] = grad_x[i]*grad_x[i];
        IxIy[i] = grad_y[i]*grad_x[i];
        IyIy[i] = grad_y[i]*grad_y[i];
        IxIt[i] = grad_t[i]*grad_x[i];
        IyIt[i] = grad_t[i]*grad_y[i];

        // temporary stand-ins until better values are available
        // u_frame[i] = grad_x[i];
        // v_frame[i] = grad_y[i];
    }

    // base X and Y
    // sum of Ix Ix terms, etc.
    int sIxx, sIxy, sIyy, sIxt, sIyt;

    // compute sums at each offset
    // printf("Calculating matrix outcome\n");
    for (int by = 0; by < rows; by+=OF_FRAME_SKIPS){
        for (int bx = 0; bx < cols; bx+=OF_FRAME_SKIPS){
            sIxx = sIxy = sIyy = sIxt = sIyt= 0;  // reset for this frame
            
            int baseIdx = by * cols + bx;

            // Compute sums (ATA), (ATb)
            for(int y=0;y<OF_WINDOW && by+y <rows;y++){
                for(int x=0;x<OF_WINDOW && bx+x<cols;x++){
                    int idx = (by+y) * cols + (bx+x);
                    sIxx += IxIx[idx];
                    sIxy += IxIy[idx];
                    sIyy += IyIy[idx];

                    sIxt += IxIt[idx];
                    sIyt += IyIt[idx];
                }
            }

            // printf("Solving matrix for point: (%d,%d)\n",bx, by);
            auto uv = new int8_t[2];
            solveMatrixATAATb(sIxx, sIxy, sIyy, sIxt, sIyt, uv);

            for(int y=0;y<OF_FRAME_SKIPS;y++){
                for(int x=0;x<OF_FRAME_SKIPS;x++){
                    int idx = (by+y) * cols + (bx+x);
                    u_frame[idx] += uv[0];
                    v_frame[idx] += uv[1];
                }
            }
            delete uv;
        }
    }

    // Cleanup
    delete grad_x;
    delete grad_y;
    delete grad_t;

    delete IxIx;
    delete IxIy;
    delete IyIy;
    delete IxIt;
    delete IyIt;
}

void solveMatrixATAATb(int16_t sIxx, int16_t sIxy, int16_t sIyy, int16_t sIxt, int16_t sIyt, int8_t *uv){
    // A^T A = [[a b],[b c]]
    // A^T b = [d e]
    // det(A^T A) = ac - bb
    // k = 1/(ac-bb)
    // (A^T A)^-1 = k * [[c -b],[-b a]]
    // {u,v} = k * [[c -b],[-b a]] * [d e]
    float a = sIxx;
    float b = sIxy;
    float c = sIyy;
    float d = sIxt;
    float e = sIyt;

    float k = 1 / (a*c - b*b);

    uv[0] = (int)(k * ( c*d - b*e));
    uv[1] = (int)(k * (-b*d + a*e));
}

void drawFullyStackedPyramid(uint8_t **n_pyramid, int16_t *u_frame, int layers){
    // draw the pyramid layer as the temporary output into u
    for(int i = 1; i< layers; i++){
        for(int j=0; j < (1<<i); j++){
            drawPyramid(n_pyramid, u_frame, i,((1<<i)-2) * IMAGE_ROWS/(1<<i), j * IMAGE_COLS/(1<<i));
        }
    }
}

void drawPyramid(uint8_t **pyramid, int16_t *canvas, int layer, int r_offset, int c_offset, uint8_t bias){
    int cols = IMAGE_COLS / (1<<layer);
    int rows = IMAGE_ROWS / (1<<layer);
    for(int i=0; i < cols*rows; i++){
        int r = (i / cols) + r_offset;
        int c = (i % cols) + c_offset;
        canvas[r * IMAGE_COLS + c] = pyramid[layer][i] - 0x80;
    }
}

int getPyramidSize(uint8_t frame[IMAGE_ROWS * IMAGE_COLS]){
    int pyramidSize = 1;
    int dim = (IMAGE_ROWS < IMAGE_COLS) ? IMAGE_ROWS : IMAGE_COLS; // smaller dimension to avoid 1x(fraction of pixel)
    while(dim > 0){
        pyramidSize++;
        dim /= 2;
    }
    
    pyramidSize -= 2; // cut off the singularity top
    // printf("pyramid size: %d\n", pyramidSize);
    return pyramidSize;
}

uint8_t **generatePyramid(uint8_t frame[IMAGE_ROWS * IMAGE_COLS], int pyramidSize){
    auto pyramid = new uint8_t* [pyramidSize]; // array of pointers to pyramids that will be generated next
    
    // create base of pyramid
    auto baseImage = new uint8_t [IMAGE_ROWS * IMAGE_COLS];
    for(int i=0; i<IMAGE_COLS*IMAGE_ROWS; i++){
        baseImage[i] = frame[i];
    }
    pyramid[0] = baseImage;

    // printf("Copied base layer\n");


    for (int i=1; i<pyramidSize; i++){
        // printf("Generating layer %d\n", i);
        int rsize = IMAGE_ROWS/(1<<i);
        int csize = IMAGE_COLS/(1<<i); 

        // unfortunately due to C++ constraints, the array has to be 1-dimensional
        auto step = new uint8_t [rsize * csize];
        pyramid[i] = step; // track the pointer for removal at a later time
        generatePyramidLayer(pyramid[i-1], pyramid[i], rsize, csize);
    }

    return pyramid;
}

void destroyPyramid(uint8_t** pyramid, int pyramidSize){
    for (int i=0; i<pyramidSize; i++){
        delete pyramid[i];
    }
    delete pyramid;
}

void generatePyramidLayer(uint8_t *frame, uint8_t *shrunkFrame, int rows, int cols){
    // rows, cols: dimensions of shrunk frame

    // printf("Pyramid generating layer of size: (%d,%d)\n", rows, cols);
    // start by clearing the buffer contents
    for (int i=0;i<rows*cols; i++){
        // printf("%d, ", i);
        shrunkFrame[i] = 0;
    }

    // printf("Cleared frame buffer of size: (%d,%d)\n", rows, cols);

    // compress data as sum
    for (int r=0; r<rows; r++){
        for (int c=0; c<cols; c++){
            int sfI = r * cols + c;
            int bfI = r * 2*cols + c;
            // printf("(%d,%d)->[%d],[%d]\n", r,c, sfI, bfI);
            // printf("sf[0] = %d\n", shrunkFrame[0]);
            // printf("bf[0] = %d\n", frame[0]);
            shrunkFrame[sfI] = frame[2*r * 2*cols + 2*c];/// +
                                // frame[r * 2*cols + (c+1)] +
                                // frame[(r+1) * 2*cols + c] +
                                // frame[(r+1) * 2*cols + (c+1)])/4;
        }
    }

    // printf("Populated frame buffer of size: (%d,%d)\n", rows, cols);
}

void computeGrad(uint8_t *p_frame, uint8_t *n_frame, int8_t *gradient, int rows, int cols, int dx, int dy, int dt){
    // Computing I_x, I_y, I_t: 
    // https://youtu.be/IjPLZ3hjU1A?list=PL2zRqk16wsdoYzrWStffqBAoUY8XdvatV&t=566
    // Simplified from this video as just 
    // I_v = I(x+dx,y+dy,t+dt) - I(x,y,t)
    // I_v = I(x,y,t) - I(x-dx,y-dy,t-dt)
    for(int r = 1; r < rows; r++){
        for(int c=1; c < cols; c++){
            int idx = r * cols + c;
            int didx = (r - dy) * cols + c - dx;
            int priorv = (dt) ? p_frame[didx]: n_frame[didx]; // dt determines which frame to draw from
            gradient[idx] = n_frame[idx] - priorv;
        }
    }
}

void initZero(uint8_t *frame, int rows, int cols){
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int index = (row * cols) + col;     // Calculate the index in the 1D buffer
            frame[index] = 0;
        }
    }
}

void initZero(int16_t *frame, int rows, int cols){
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int index = (row * cols) + col;     // Calculate the index in the 1D buffer
            frame[index] = 0;
        }
    }
}

void computeEigenABBC(float a, float b, float c, float eigen[2]){
    // Computes the eigenvalues for a symmetric matrix of the form:
    // [a b
    //  b c]
    //  as eigen = [(a+c) +/-  sqrt((a+c)^2 - 4 (ac-b^2)) ]/2
    
    float A = 1.0;
    float B = -(a+c); 
    float C = a * c - b*b;

    float root = sqrt(B*B - 4 * A * C);
    float base = -B;
    float denom = 2 * A;

    eigen[0] = (base-root)/denom;
    eigen[1] = (base+root)/denom;

    // printf("λ_0 = %f, λ_1 = %f\n", eigen[0], eigen[1]);
}

void computeEigenABBC(int sIxx, int sIxy, int sIyy, float eigen[2]){
    float a = (float) sIxx;
    float b = (float) sIxy;
    float c = (float) sIyy;
    computeEigenABBC(a,b,c, eigen); // type cast and pass forward
}

void findCorners(uint8_t p_frame[IMAGE_ROWS * IMAGE_COLS], uint8_t corners[IMAGE_ROWS * IMAGE_COLS]){
    for(int i=0;i< IMAGE_COLS*IMAGE_ROWS;i++) corners[i]=0; // reset to 0

    int rows = IMAGE_ROWS;
    int cols = IMAGE_COLS;

    auto grad_x = new int8_t [rows * cols];
    auto grad_y = new int8_t [rows * cols];
    
    computeGrad(p_frame, p_frame, grad_x, rows, cols, 1,0,0);
    computeGrad(p_frame, p_frame, grad_y, rows, cols, 0,1,0);
    
    // printf("Allocating product storage\n");
    auto IxIx = new int16_t [rows * cols];
    auto IxIy = new int16_t [rows * cols];
    auto IyIy = new int16_t [rows * cols];
    
    
    // printf("Calculating products\n");
    for(int i=0; i<rows * cols; i++){
        IxIx[i] = grad_x[i]*grad_x[i];
        IxIy[i] = grad_y[i]*grad_x[i];
        IyIy[i] = grad_y[i]*grad_y[i];
    }

    // Calculate M matrix (symmetric, 3 unique elements 2x2)
    // use it as the evaluation metric
    for(int r=0;r<rows;r++){
        for(int c=0;c<cols;c++){
            int sIxx = 0;
            int sIxy = 0;
            int sIyy = 0;
         
            // sums over the window of interest for corner detection
            for(int y=0;y<CD_WINDOW && r+y <rows;y++){
                for(int x=0;x<CD_WINDOW && c+x<cols;x++){
                    int idx = (r+y) * cols + (c+x);
                    sIxx += IxIx[idx];
                    sIxy += IxIy[idx];
                    sIyy += IyIy[idx];
                }
            }

            // Evaluate the matrix eigenvalues to determine 
            // if it's a good candidate as a corner to track
            auto eigen = new float[2];

            computeEigenABBC(sIxx, sIxy, sIyy, eigen);

            int idx = r * cols + c;
            corners[idx] = (min(eigen[0],eigen[1]) > 200) ? 1:0;

            delete eigen;
        }
    }

    delete grad_x;
    delete grad_y;

    delete IxIx;
    delete IxIy;
    delete IyIy;
}
