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

def radialPercent(image, x,y,r):
    # Computes what percent of the pixels within a given "radius" bounding box are actually red (the object of interest)
    # Values less than pi/4 indicate that the box is too big, and r should be shrunk
    # Values greater than pi/4 indicate that a part of the ball is clipped off-screen, and r should be increased
    uY = max(0, y - r)
    lY = min(DISPLAY_ROWS, y + r)
    lX = max(0, x - r)
    uX = min(DISPLAY_COLS, x + r)
    s = 0
    # print(image[lX:uX, uY:lY])
    # print(np.sum(image))
    for x in range(lX, uX):
        for y in range(uY, lY):
            if image[y,x] > 0:
                s+=1

    p = s / ((uX - lX) * (lY - uY))
    # print(f"r:{r} s:{s} p:{p} uY:{uY} lY:{lY} uX:{uX} lX:{lX}", end="\t")
    return p


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

        rmap = relu(rs - 0.65 * (gs+bs))
        print(f"R min:{np.min(rs)} max:{np.max(rs)} avg:{np.mean(rs)}", end="\t")
        print(f"G min:{np.min(gs)} max:{np.max(gs)}", end="\t")
        print(f"B min:{np.min(bs)} max:{np.max(bs)}", end="\t")
        print("Rmap shape=", rmap.shape, "Rmap avg=", np.mean(rmap), "Rmap sum=", np.sum(rmap), end="\t")

        print("")
        # Generate cmap
        cmap = grid.copy()
        for j in range(3):
            cmap[:,:,j] = rmap

        magnitude_threshold = 1000
        if np.sum(rmap) > magnitude_threshold:

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

            r = 90 # 5 pixel buffer around the image
            p = 0
            explored = {}
            # p == 0 and
            while r not in explored and r > 0:   # assumes concave distribution of p across argument r
                # normalizing p by the expected portion for an ideal circle, the p acts as an error signal
                p = radialPercent(rmap, meanX, meanY, r) - np.pi / 4
                explored[r] = p
                # print(f"rad {r} -> {p}")
                if p > 0:
                    r+=1
                if p < 0:
                    r-=1

            # Draw the bounding box
            upperY = max(0, meanY - r)
            lowerY = min(DISPLAY_ROWS, meanY + r)
            lowerX = max(0, meanX - r)
            upperX = min(DISPLAY_COLS, meanX + r)
            # cmap[:,lowerX,2] = 255
            # cmap[:,upperX,2] = 255
            for x in range(lowerX, upperX):
                for y in range(upperY, lowerY):
                    cmap[y,x,1] = 128

        # End of the big if - rendering segment

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
        
