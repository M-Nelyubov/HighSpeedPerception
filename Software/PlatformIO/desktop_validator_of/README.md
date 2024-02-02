# Desktop Validator of Optical Flow

This directory hosts a program that will access the optical flow module used in LK_Avoider.  It will feed the library data from the host computer's camera and display the real-time inputs and outputs to validate the outputs of the algorithm.

This requires use of the C++ compiler toolchain, CMake, and OpenCV for C++ to be installed on the system.

Use `bareBones.cpp` to analyze files from `D:`

Saving files from D: to be used for testing:
```ps1
$i=0; ls D:\raw\img\ |% FullName |%{Copy-Item $_ -Destination D:\img\test\$i.bytes;$i++}
```