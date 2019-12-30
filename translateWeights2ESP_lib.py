#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Nov 30 11:56:37 2019

@author: caleb
"""
import numpy as np
import os
#(n, k, h, w) <-- from caffe website
net = 'rnet'
path = '/home/caleb/Downloads/DataSets/mtcnn-head-detection-master/'
#newDir='oNewMISS2/'
cutConvSmall=False
allfiles=os.listdir(os.path.join(path+net+'Out/'))
with open (path+net+'Out/'+net+'Cut16Weights.h','w') as f:
    for file in allfiles:
        print("processing: "+file)
        if '.npy' not in file:
            print("SKIPPING: "+file)
            continue
        c = np.load(path+net+'Out/'+file) #load npy file created by readCaffeModel.py
        print("orig Shape")
        print(c.shape) #printout the shape
        print('\n')

        try:
            #ESP LIB : N,H,W,C 
            # translate --> normal/tensorflow format: (H,W,C,N)
            #====== CAFFE BLOB FORMAT:                (N C H W) 
            #=======ESP deep learning:                (N H W C)
            #if 'caffe' in framework:
           
           # t=np.moveaxis(c,1,3) 
           # t=np.moveaxis(t,1,2)
            
            #USING TRANSPOSE...0
            t=c.transpose(0,2,3,1)
            
            #out: N H W C woohoo! done.... should be like (bigNum,same ,same, Channel)
            '''
            # out: ( n,w,c,h ) 
            #next -> c,h (2,3)
            t1 = np.moveaxis(t,2,3)
            #out: (n,w,h,c)
            #next -->  w,h : 1,2
            #t = np.moveaxis(t1,2,1)
            # out --> n,h,w,c woohoo! translated
            '''
            print(t.shape)
            #print('new shape\n')
            shape=t.shape
            all=np.reshape(t,-1)#single dimension for printout (C uses flat buffer array)
            
            doHalf = sum(t.shape)
            cutConv = False
            if doHalf >80:
                cutConvSmall = True
                print('Cut Smaller')
            if doHalf > 400:
                print("GOTTA SPLIT THIS CONVOLUTION")
                cutConv=True
                cutConvSmall=False
            #print(len(all))
        except:
            print("1D weight array")
            shape=c.shape
            all=np.reshape(c,-1)
            cutConv = False
            doHalf = sum(shape)
            if doHalf >130:
                cutConvSmall = True
                print('Cut Smaller')
            if doHalf > 400:
                cutConv = True
                cutConvSmall = False

                print("GOTTA SPLIT THIS CONVOLUTION")

            
        if  not cutConv and not cutConvSmall:
            f.writelines("void get"+net+"_"+file[:-4]+"(dl_matrix3d_t* out) { \n     ")
            f.writelines("// "+str(shape)+" "+file+" from caffemodel\n")
            f.writelines("float temp[] = { ")
            count = 0
            for i in all:
                f.writelines(str(i)+",")# put the translated weights into a file
                if count %4 ==0:
                    f.writelines("\n")
                count+=1
        #so i dont have to type out the function every time...             
            f.writelines("}; ")
            f.writelines("\nsize_t n = sizeof(temp) / sizeof(temp[0]);\n")
            f.writelines("  for (int i=0;i<n;i++)\n   {\n      out->item[i]=temp[i];\n   }")
            f.writelines("\n}//endFunc\n")    
            print("#elems: ",count)
        
        if cutConvSmall:
            for x in range(2):
                
                smallCuts=2
                
                f.writelines("void get"+net+"_cut"+str(x)+"_"+file[:-4]+"(dl_matrix3d_t* out) { \n     ")
                f.writelines("// "+str(shape)+" "+file+" from caffemodel\n")
                f.writelines("float temp[] = { ")
                count = 0
                
                if x == 0:
                    #do first half
                    temp = all[0:int(len(all)*1/smallCuts)]#.125)]
                if x == 1:
                    #do second half
                    temp = all[int(len(all)*1/smallCuts):int(len(all)*2/smallCuts)]#.25)]
                if x == 2:
                    '''
                    #do first half
                    temp = all[int(len(all)*2/tCuts):int(len(all)*3/tCuts)]
                if x == 3:
                    #do second half
                    temp = all[int(len(all)*3/tCuts):int(len(all)*4/tCuts)]
                if x == 4:
                    #do first half
                    temp = all[int(len(all)*4/tCuts):int(len(all)*5/tCuts)]
                if x == 5:
                    #do second half
                    temp = all[int(len(all)*5/tCuts):int(len(all)*6/tCuts)]
                if x == 6:
                    #do first half
                    temp = all[int(len(all)*6/tCuts):int(len(all)*7/tCuts)]
                if x == 7:
                    #do second half
                    temp = all[int(len(all)*7/tCuts):int(len(all)*8/tCuts)]
                if x == 8:
                    #do first half
                    temp = all[int(len(all)*8/tCuts):int(len(all)*9/tCuts)]
                if x == 9:
                    #do second half
                    temp = all[int(len(all)*9/tCuts):int(len(all)*10/tCuts)]
                if x == 10:
                    #do first half
                    temp = all[int(len(all)*10/tCuts):int(len(all)*11/tCuts)]
                if x == 11:
                    #do second half
                    temp = all[int(len(all)*10/tCuts):int(len(all)*11/tCuts)]
                if x == 12:
                    #do first half
                    temp = all[int(len(all)*11/tCuts):]#int(len(all)*11/tCuts)]
                    '''
                for i in temp:
                    
                    f.writelines(str(i)+",")# put the translated weights into a file
                    if count %4 ==0:
                        f.writelines("\n")
                    count+=1
                f.writelines("}; ")
                f.writelines("\nsize_t n = sizeof(temp) / sizeof(temp[0]);\n")
                f.writelines("  for (int i=0;i<n;i++)\n   {\n      out->item[i]=temp[i];\n   }")
                f.writelines("\n}//endFunc\n")
                print("#elems: ",count, "at:",x)  
                      
        cutConvSmall=False
        
        if cutConv:
            for x in range(16):
                
                tCuts=16
                
                f.writelines("void get"+net+"_cut"+str(x)+"_"+file[:-4]+"(dl_matrix3d_t* out) { \n     ")
                f.writelines("// "+str(shape)+" "+file+" from caffemodel\n")
                f.writelines("float temp[] = { ")
                count = 0
                
                if x == 0:
                    #do first half
                    temp = all[0:int(len(all)*1/tCuts)]#.125)]
                if x == 1:
                    #do second half
                    temp = all[int(len(all)*1/tCuts):int(len(all)*2/tCuts)]#.25)]
                if x == 2:
                    #do first half
                    temp = all[int(len(all)*2/tCuts):int(len(all)*3/tCuts)]
                if x == 3:
                    #do second half
                    temp = all[int(len(all)*3/tCuts):int(len(all)*4/tCuts)]
                if x == 4:
                    #do first half
                    temp = all[int(len(all)*4/tCuts):int(len(all)*5/tCuts)]
                if x == 5:
                    #do second half
                    temp = all[int(len(all)*5/tCuts):int(len(all)*6/tCuts)]
                if x == 6:
                    #do first half
                    temp = all[int(len(all)*6/tCuts):int(len(all)*7/tCuts)]
                if x == 7:
                    #do second half
                    temp = all[int(len(all)*7/tCuts):int(len(all)*8/tCuts)]
                if x == 8:
                    #do first half
                    temp = all[int(len(all)*8/tCuts):int(len(all)*9/tCuts)]
                if x == 9:
                    #do second half
                    temp = all[int(len(all)*9/tCuts):int(len(all)*10/tCuts)]
                if x == 10:
                    #do first half
                    temp = all[int(len(all)*10/tCuts):int(len(all)*11/tCuts)]
                if x == 11:
                    #do second half
                    temp = all[int(len(all)*10/tCuts):int(len(all)*11/tCuts)]
                if x == 12:
                    #do first half
                    temp = all[int(len(all)*11/tCuts):int(len(all)*12/tCuts)]#int(len(all)*11/tCuts)]
                if x == 13:
                    #do first half
                    temp = all[int(len(all)*12/tCuts):int(len(all)*13/tCuts)]
                if x == 14:
                    #do second half
                    temp = all[int(len(all)*13/tCuts):int(len(all)*14/tCuts)]
                if x == 15:
                    #do first half
                    temp = all[int(len(all)*14/tCuts):int(len(all)*15/tCuts)]
                if x == 16:
                    #do second half
                    temp = all[int(len(all)*15/tCuts):int(len(all)*16/tCuts)]
                
                
                
                    
                for i in temp:
                    
                    f.writelines(str(i)+",")# put the translated weights into a file
                    if count %4 ==0:
                        f.writelines("\n")
                    count+=1
                f.writelines("}; ")
                f.writelines("\nsize_t n = sizeof(temp) / sizeof(temp[0]);\n")
                f.writelines("  for (int i=0;i<n;i++)\n   {\n      out->item[i]=temp[i];\n   }")
                f.writelines("\n}//endFunc\n")
                print("#elems: ",count, "at:",x)
            
f.close()

 
           
