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

i = 0
print(f"Found {len(files)} files")
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
        print("Rmap shape=", rmap.shape, "Rmap avg=", np.mean(rmap), "Rmap sum=", np.sum(rmap), end="\t")

        print("")
        # Generate cmap
        cmap = grid.copy()
        for j in range(3):
            cmap[:,:,j] = rmap

        # Compute X Mean
        sumsX = np.zeros(DISPLAY_COLS)
        xfx = np.zeros(DISPLAY_COLS)
        for c in range(DISPLAY_COLS):
            sumsX[c] = np.sum(rmap[:,c])
            xfx[c] = sumsX[c] * c
        meanX = int(np.sum(xfx) / (.0001+ np.sum(sumsX)))

        # Compute Y Mean
        sumsY = np.zeros(DISPLAY_COLS)
        yfy = np.zeros(DISPLAY_COLS)
        for r in range(DISPLAY_ROWS):
            sumsY[r] = np.sum(rmap[r,:])
            yfy[r] = sumsY[r] * r
        meanY = int(np.sum(yfy) / (.0001+ np.sum(sumsY)))

        # Draw the green line
        cmap[:,meanX,1] = 255
        cmap[meanY,:,1] = 255
        
        print(f"Read length: {file}")
        # img = cv2.imdecode(grid, cv2.IMREAD_UNCHANGED) # https://www.geeksforgeeks.org/python-opencv-imdecode-function/
        cv2.imshow("frame", grid)
        cv2.imshow("b", bs.astype("uint8"))
        cv2.imshow("g", gs.astype("uint8"))
        cv2.imshow("r", rs.astype("uint8"))
        cv2.imshow("rmap", rmap.astype("uint8"))
        cv2.imshow("cmap", cmap.astype("uint8"))

        # wait and escape sequence from homebrewed
        key = cv2.waitKey(5000)
        if key == 113 or key == 27:  # 113 = q, 27 = ctrl c
            break

        if key == 97: # 'a'
            i-=1
            print(key, i)
            continue

        if key > 0:
            print(key, i)
            i+=1
            continue
        
