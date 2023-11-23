# LR Scanner

This code uses a Harris Corner detector to identify points of interest.  SSD is then used to estimate where the image patch around this corner moved to in the next frame.  This displacement (u,v) is taken as the optical flow of the image.

The non-zero flow points are then split by whether they appear on the left or right half of the screen.  Points in each half of the screen contribute to an estimate of the average rotation occuring in that half of the screen.  

This rotation would be used to estimate motion.