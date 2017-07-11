
# coding: utf-8

# In[3]:

#!/usr/bin/env python
from tkinter import *
from PIL import Image, ImageEnhance
from PIL import ImageTk
from tkinter import filedialog
import cv2
from skimage.segmentation import felzenszwalb, slic, quickshift
from skimage.segmentation import mark_boundaries
from skimage.segmentation import find_boundaries
from scipy.misc import toimage
import numpy as np
import glob

import os


# In[4]:

class OrientationGUI:
    
    def __init__(self,master):
        master.title("Orientation Tool")
        self.panelA = None
        self.angle=0
        self.main_window()
        
    def main_window(self):
        btnSelectImage = Button(root, text="Select an image", command=self.open_image)
        btnRotateLeft = Button(root, text=">>>", command=self.rotate_Left_Image)
        btnRotateRight = Button(root, text="<<<", command=self.rotate_Right_Image)
        btnRotateLeft_small = Button(root, text=" > ", command=self.rotate_Left_Image_small)
        btnRotateRight_small = Button(root, text=" < ", command=self.rotate_Right_Image_small)
        btnSave = Button(root, text=" Save ", command=self.save)
        
        self.panelA = Canvas(root, width=900, height=600, background="bisque")
        
        btnSelectImage.grid(row=0,column=1,columnspan=2)
        self.panelA.grid(row=1,column=0,columnspan=4,rowspan=4)
        btnRotateLeft.grid(row=6,column=2,columnspan=2)
        btnRotateRight.grid(row=6,column=0,columnspan=2)
        btnRotateLeft_small.grid(row=7,column=2,columnspan=2)
        btnRotateRight_small.grid(row=7,column=0,columnspan=2)
        btnSave.grid(row=8,column=1,columnspan=2)
        
        # This is what enables using the mouse for move the image in the window
        self.panelA.bind("<ButtonPress-1>",self.move_start)
        self.panelA.bind("<B1-Motion>",self.move_move)
        
        
        
    #move the image in the window
    def move_start(self,event):
        if(self.path!=None):
            self.previous_x=event.x
            self.previous_y=event.y
        
    def move_move(self,event):
        if(self.path!=None):
            x = self.previous_x - event.x
            y = self.previous_y - event.y
            self.angle = self.angle - (y*0.08)
            M = cv2.getRotationMatrix2D(self.center, self.angle, 1)
            rotated = cv2.warpAffine(self.image, M, (self.width_original, self.height_original))
            imageOUT=toimage(rotated)
            imageOUT = ImageTk.PhotoImage(imageOUT)
            self.panelA.create_image(self.x_pos_panel, self.y_pos_panel, image = imageOUT, anchor = NW)
            self.panelA.image = imageOUT
            self.previous_x=event.x
            self.previous_y=event.y
        
            
    def path_to_image(self):
        path = filedialog.askopenfilename()
        return path
    
    #find the index of the actual image selected
    def find_index_selected(self,images,path):
        i=0
        for pathImage in self.images:
            if(path==pathImage):
                indexSelected=i
            i=i+1
        return indexSelected
    
    def rotate_Right_Image(self):
        if(self.path!=None):
            self.angle+=5
            M = cv2.getRotationMatrix2D(self.center, self.angle, 1)
            rotated = cv2.warpAffine(self.image, M, (self.width_original, self.height_original))
            imageOUT=toimage(rotated)
            imageOUT = ImageTk.PhotoImage(imageOUT)
            self.panelA.create_image(self.x_pos_panel, self.y_pos_panel, image = imageOUT, anchor = NW)
            self.panelA.image = imageOUT
    
    def rotate_Left_Image(self):
        if(self.path!=None):
            self.angle-=5
            M = cv2.getRotationMatrix2D(self.center, self.angle, 1)
            rotated = cv2.warpAffine(self.image, M, (self.width_original, self.height_original))
            imageOUT=toimage(rotated)
            imageOUT = ImageTk.PhotoImage(imageOUT)
            self.panelA.create_image(self.x_pos_panel, self.y_pos_panel, image = imageOUT, anchor = NW)
            self.panelA.image = imageOUT
        
    def rotate_Right_Image_small(self):
        if(self.path!=None):
            self.angle+=0.5
            M = cv2.getRotationMatrix2D(self.center, self.angle, 1)
            rotated = cv2.warpAffine(self.image, M, (self.width_original, self.height_original))
            imageOUT=toimage(rotated)
            imageOUT = ImageTk.PhotoImage(imageOUT)
            self.panelA.create_image(self.x_pos_panel, self.y_pos_panel, image = imageOUT, anchor = NW)
            self.panelA.image = imageOUT
        
    def rotate_Left_Image_small(self):
        if(self.path!=None):
            self.angle-=0.5
            M = cv2.getRotationMatrix2D(self.center, self.angle, 1)
            rotated = cv2.warpAffine(self.image, M, (self.width_original, self.height_original))
            imageOUT=toimage(rotated)
            imageOUT = ImageTk.PhotoImage(imageOUT)
            self.panelA.create_image(self.x_pos_panel, self.y_pos_panel, image = imageOUT, anchor = NW)
            self.panelA.image = imageOUT
        
    def open_image(self):
        path=self.path_to_image()
        self.path=path
        # ensure a file path was selected
        if len(path) > 0:
            #get the root of the path
            pathRoot=path[:path.rfind('/')] + '/*'+path[path.rfind('.'):]
            #open all the images in the path
            self.images = glob.glob(pathRoot)
            #find the index of the image selected for trace the previous and next images
            self.index_image=self.find_index_selected(self.images,path)
            image = cv2.imread(path)
            #convert to RGB from BGR
            image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
            self.height_original,self.width_original=image.shape[:2]
            self.center=(self.width_original/2, self.height_original/2)
            self.image_original=image
            #init the mask for the image
            self.mask = np.zeros(self.image_original.shape[:3], dtype = "uint8")
            self.image=image
            # convert the images to PIL format...
            image=toimage(image)
            # ...and then to ImageTk format
            imageOUT = ImageTk.PhotoImage(image)
            centro_panel=(450,300)
            self.x_pos_panel=(450-(self.width_original/2))
            self.y_pos_panel=(300-(self.height_original/2))
            
            self.panelA.create_image(self.x_pos_panel, self.y_pos_panel, image = imageOUT, anchor = NW)
            self.panelA.image = imageOUT
            #self.text.delete(1.0,END)
            #self.text.insert(INSERT,self.path)
            
            
    def save(self):
        if(self.path!=None):
            Fname=(self.path[self.path.rfind('/')+1 : self.path.rfind('.')] + 'MaskRotationAngle')

            f =  filedialog.asksaveasfile(mode='wb',initialfile=Fname, defaultextension=".txt", filetypes=(("txt file", "*.txt"),("All Files", "*.*")))

            if f:
                file = open(os.path.abspath(f.name),"w")
                file.write(str(self.angle))
    
    

    
#The master of the GUI
root=Tk()
#call the class for the SegmentationGUI
app=OrientationGUI(root)
#start the GUI loop
root.mainloop()
    
        

    
    
    


# In[ ]:




# In[ ]:



