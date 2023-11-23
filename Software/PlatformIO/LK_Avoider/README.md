# LK Avoider

This software is based on the LK Optical Flow algorithm.  A coarse to fine image pyramid was created, but not used due to project compute constraints and increased noise.

Once flow is computed, the (u,v) data is split into left and right halves of the screen.  The amount of points above a threshold in each half of the screen is measured.  Once the amount of points on one half of the screen exceeds a threshold, the motor on the other side of the robot is disabled, causing the robot to turn in the direction of the stopped motor, thereby aiming to avoid the obstacle.

This algorithm is based on:

> N. Urieva, J. McDonald, T. Uryeva, A. S. Rose Ramos and S. Bhandari, "Collision Detection and Avoidance using Optical Flow for Multicopter UAVs," 2020 International Conference on Unmanned Aircraft Systems (ICUAS), Athens, Greece, 2020, pp. 607-614, doi: 10.1109/ICUAS48674.2020.9213957.

