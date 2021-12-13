import cv2
import numpy as np
import matplotlib.pyplot as plt
import glob, os

path = './sk2'
img_list = [ ]
IMG_NAME = 'boat'

vidcap = cv2.VideoCapture('./124.avi')

count =0;
 
while(vidcap.isOpened()):
    ret, image = vidcap.read()
    
    if(int(vidcap.get(1)) % 10 == 0):
        print('Saved frame number : ' + str(int(vidcap.get(1))))
        cv2.imwrite("./sk2/%d.jpg" % count, image)
        print('Saved frame%d.jpg' % count)
        count += 1
    if count==18:
        break;    


for root, directories, files in os.walk(path):
    
    for file in files:
        if '.jpg' in file:
            img_list.extend(glob.glob(os.path.join(root, file)))

img_list = sorted(img_list)

print(img_list)

imgs = []


for i, img_path in enumerate(img_list):
    img = cv2.imread(img_path)
    #img = cv2.rotate(img, cv2.ROTATE_90_COUNTERCLOCKWISE)
    imgs.append(img)
    
    
mode = cv2.STITCHER_PANORAMA
# mode = cv2.STITCHER_SCANS

if int(cv2.__version__[0]) == 3:
    stitcher = cv2.createStitcher(mode)
else:
    stitcher = cv2.Stitcher_create(mode)
    
status, stitched = stitcher.stitch(imgs)

if status == 0:
    cv2.imwrite(os.path.join('result/result123.jpg'), stitched)
    gray = cv2.cvtColor(stitched, cv2.COLOR_BGR2GRAY)
    thresh = cv2.bitwise_not(cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY)[1]) #이미지 반전
    thresh = cv2.medianBlur(thresh, 5)  # 노이즈 없애주는 이미지를 뭉개어 노이즈를없애줌

    stitched_copy = stitched.copy()
    thresh_copy = thresh.copy()
    
    while np.sum(thresh_copy) > 0:  #검은색 부분만 남으면 모든 합이0 이므로  while문 탈출
        thresh_copy = thresh_copy[1:-1, 1:-1]
        stitched_copy = stitched_copy[1:-1, 1:-1]
    
    #stitched_copy = stitched_copy[1:-1, 1:-1]
    stitched_copy =cv2.resize(stitched_copy,(1920,1080),None,0.5,0.5)
    cv2.imwrite(os.path.join('result/result_crop123.jpg'), stitched_copy)
    cv2.namedWindow('result', cv2.WINDOW_NORMAL)
    cv2.imshow('result', stitched_copy)

    cv2.waitKey(0)
    print("stitching completed successfully.")
else:
    print('failed... %s' % status)
    
