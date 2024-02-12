import numpy as np
import os
import cv2

ROWS = 96
COLS = 96

base = "D:/raw/img"
files = os.listdir(base)

for file in ["col0.bytes"]: # files:
    fullPath = f"{base}/{file}"
    with open(fullPath, "rb") as imageBytes:
        bytes = bytearray(imageBytes.read())
        arr = np.array(bytes, dtype="uint8")
        grid = arr.reshape((ROWS,COLS,3))
        grid = cv2.resize(grid, (4*ROWS, 4*COLS))

        # rescale back up to full intensity
        b = grid[:,:,0] * 8
        g = grid[:,:,1] * 4
        r = grid[:,:,2] * 8
        
        print(f"Read length: {file}")
        # img = cv2.imdecode(grid, cv2.IMREAD_UNCHANGED) # https://www.geeksforgeeks.org/python-opencv-imdecode-function/
        cv2.imshow("frame", grid)
        cv2.imshow("b", b)
        cv2.imshow("g", g)
        cv2.imshow("r", r)

        # wait and escape sequence from homebrewed
        key = cv2.waitKey(20000)
        if key > 0:
            print(key)
        if key == 113 or key == 27:  # 113 = q, 27 = ctrl c
            break
