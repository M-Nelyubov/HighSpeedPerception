#include "object_recognition.hpp"
#define min(a,b)  ((a<b)? a:b)
#define max(a,b)  ((a>b)? a:b)

#define BCI 0   // blue channel index
#define GCI 1   // green channel index
#define RCI 2   // red channel index

void extractRed(uint8_t inputFrame [IMAGE_SIZE * COLOR_CHANNELS], uint8_t outputFrame [IMAGE_SIZE]){
    for(int i=0; i<IMAGE_SIZE; i++){
        int ii = i * COLOR_CHANNELS;  // input index
        int oi = i;                   // ouptut index

        // Calculate greyscale pixel value
        // int mag = 0;
        // for(int c=0;c<COLOR_CHANNELS;c++){
        //     mag += inputFrame[ii+c];
        // }
        // mag /= COLOR_CHANNELS;
        int r = inputFrame[ii + RCI];
        int g = inputFrame[ii + GCI];
        int b = inputFrame[ii + BCI];

        outputFrame[oi] = (r > g+b) ? (r - (g+b)) : 0;
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

void determinePixelCluster(uint8_t red_islands_frame[IMAGE_SIZE], uint8_t red_clusters_frame[IMAGE_SIZE], int *highestCluster, int row, int col, int stackDepth=0){
    if(row < 0) return;
    if(col < 0) return;
    if(col == IMAGE_COLS) return;
    if(row == IMAGE_ROWS) return;

    int i = (row * IMAGE_COLS) + col;     // Calculate the index in the 1D buffer
    
    // only do this calculation when the red value of the frame is non-zero
    if(red_islands_frame[i] == 0) return;

    // Inherit the same cluster as the point above you or to your left
    int lCluster = 0;
    int uCluster = 0;
    int rCluster = 0;
    int dCluster = 0;

    // get cluster of pixel above
    int iu = ((row-1) * IMAGE_COLS) + col;
    if(row - 1 > 0){
        uCluster = red_clusters_frame[iu];
    }
    
    // get cluster of pixel below
    int id = ((row+1) * IMAGE_COLS) + col;
    if(row + 1 > 0){
        dCluster = red_clusters_frame[id];
    }

    // get cluster of pixel to the left
    int il = ((row) * IMAGE_COLS) + col-1;
    if(col - 1 > 0){
        lCluster = red_clusters_frame[il];
    }

    // get cluster of pixel to the right
    int ir = ((row) * IMAGE_COLS) + col+1;
    if(col + 1 > IMAGE_COLS){
        rCluster = red_clusters_frame[ir];
    }

    int localMax = max(max(uCluster,dCluster), max(lCluster, rCluster));

    if(uCluster == 0) uCluster = localMax;
    if(dCluster == 0) dCluster = localMax;
    if(lCluster == 0) lCluster = localMax;
    if(rCluster == 0) rCluster = localMax;

    int localMin = min(min(uCluster,dCluster), min(lCluster, rCluster));

    if(localMin == localMax){
        if(localMin == 0){ // in the middle of nowhere -> new cluster
        red_clusters_frame[i] = ++highestCluster[0];
        } else {           // in the middle of a cluster -> join it
            red_clusters_frame[i] = lCluster;
        }
        return;
    }
    // printf("R%dC%d min:%d max: %d d:%d\t", row, col, localMin, localMax, stackDepth);

    // if there are different clusters merging here, merge to the lower number
    // accept the lower cluster for yourself
    red_clusters_frame[i] = localMin;

    // call the function to reevaluate all potentially affected neighbors
    determinePixelCluster(red_islands_frame, red_clusters_frame, highestCluster, row-1, col, stackDepth+1);
    // determinePixelCluster(red_islands_frame, red_clusters_frame, highestCluster, row+1, col, stackDepth+1);
    determinePixelCluster(red_islands_frame, red_clusters_frame, highestCluster, row, col-1, stackDepth+1);
    // determinePixelCluster(red_islands_frame, red_clusters_frame, highestCluster, row, col+1, stackDepth+1);

}

void clusterPatches(uint8_t red_islands_frame[IMAGE_SIZE], uint8_t red_clusters_frame[IMAGE_SIZE]){
    /**
     * Red islands frame - the red channel that is "submerged" by the neutral white light, leaving the red outliers like islands in the sea.  Topology varies with integer value.
     * red clusters frame - the clustering of continuous pixel islands into labels
    */
    int highestCluster = 0;

    // reset the clusters frame to 0 - nothing there yet
    for(int i=0; i<IMAGE_SIZE; i++) red_clusters_frame[i]=0;

    // Flag each patch as belonging to some cluster
    for (int row = 0; row < IMAGE_ROWS; row++) {
        for (int col = 0; col < IMAGE_COLS; col++) {
            int i = (row * IMAGE_COLS) + col;     // Calculate the index in the 1D buffer
            // printf("R%dC%d", row,col);
            determinePixelCluster(red_islands_frame, red_clusters_frame, &highestCluster, row, col);
            // printf("V%d\n", red_clusters_frame[i]);
        }
    }

}
