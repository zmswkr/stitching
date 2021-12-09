import numpy as np
import cv2 as cv
import os
import inspect
from PIL import Image

path = './images/'

images = []  
print(inspect.getfile(os))

for root, directories, files in os.walk(path):
    
    for file in files:
        if '.jpg' in file:
 
            img_input =cv.imread(os.path.join(root, file))
            #img_input =cv.resize(img_input,(0,0),None,0.5,0.5)
            #img_input = cv.rotate(img_input, cv.ROTATE_90_COUNTERCLOCKWISE)
            images.append(img_input)
            


stitcher = cv.Stitcher.create(cv.Stitcher_PANORAMA)
status, pano = stitcher.stitch(images)

if status != cv.Stitcher_OK:
    print("Can't stitch images, error code = %d" % status)
    exit(-1)
    
stitched_copy = pano.copy()
#pano = cv.rotate(pano, cv.ROTATE_90_CLOCKWISE)
cv.imshow('result', pano)
#pano =cv.resize(pano,(3840,1080),None,0.5,0.5)
cv.imwrite('result/result2.jpg', stitched_copy)
cv.waitKey(0)
print("stitching completed successfully.")