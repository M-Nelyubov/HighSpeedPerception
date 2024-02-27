import numpy as np
import os
import cv2

COLS, ROWS = (96, 96)
ratio = COLS/ROWS

DISPLAY_ROWS = 400
DISPLAY_COLS = int(ratio * DISPLAY_ROWS)

base = "D:/raw/img"
base = "C:/Users/maste/Pictures/esp32/sequences/96x96/redTargetting1"
files = os.listdir(base)

def relu(x):
    return np.clip(16*x, 0, 254)

key = 0
i=0
while True:
    i = i % len(files)
    file = files[i]
# for file in files: # ["col0.bytes"]: # files:
    fullPath = f"{base}/{file}"
    with open(fullPath, "rb") as imageBytes:
        bytes = bytearray(imageBytes.read())
        arr = np.array(bytes, dtype="uint8")
        grid = arr.reshape((ROWS,COLS,3))
        # grid = cv2.resize(grid, (ROWS//4, COLS//4))
        grid = cv2.resize(grid, (DISPLAY_ROWS, DISPLAY_COLS))

        # rescale back up to full intensity
        b = grid.astype("int16"); b[:,:,1]=0; b[:,:,2] = 0; bs = b[:,:,0]
        g = grid.astype("int16"); g[:,:,0]=0; g[:,:,2] = 0; gs = g[:,:,1]
        r = grid.astype("int16"); r[:,:,1]=0; r[:,:,0] = 0; rs = r[:,:,2]

        rmap = relu(rs - 0.75 * (gs+bs))
        print(f"R min:{np.min(rs)} max:{np.max(rs)} avg:{np.mean(rs)}", end="\t")
        print(f"G min:{np.min(gs)} max:{np.max(gs)}", end="\t")
        print(f"B min:{np.min(bs)} max:{np.max(bs)}", end="\t")
        print("Rmap shape=", rmap.shape, "Rmap avg=", np.mean(rmap), end="\t")
        
        print(f"Read length: {file}")
        # img = cv2.imdecode(grid, cv2.IMREAD_UNCHANGED) # https://www.geeksforgeeks.org/python-opencv-imdecode-function/
        cv2.imshow("frame", grid)
        cv2.imshow("b", bs.astype("uint8"))
        cv2.imshow("g", gs.astype("uint8"))
        cv2.imshow("r", rs.astype("uint8"))
        cv2.imshow("rmap", rmap.astype("uint8"))

        # wait and escape sequence from homebrewed
        key = cv2.waitKey(5000)
        if key == 97: # 'a'
            i-=1
            continue

        if key > 0:
            print(key)
            i+=1
        
        if key == 113 or key == 27:  # 113 = q, 27 = ctrl c
            break
