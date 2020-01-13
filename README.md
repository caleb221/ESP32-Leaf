# ESP32-Leaf
2nd Half of Thesis project, implementation of MTCNN onto an ESP32
The board is an ESP-Cam from AI-Thinker with 4mb of PSRAM.

# Description
This code implements P-Net and R-Net on the ESP32 using weights from 
the MTCNN model trained in <a href="https://github.com/caleb221/MTCNN-Leaf"> MTCNN-Leaf</a>
# Weights
The weights are translated by translateWeights2ESP_lib.py. This goes from Caffe's convention to the ESP-FACE library.
# Components Used
--> ESP-Camera 

--> ESP-FACE

# Images


ESP32-Cam by AI-Thinker


<img src="https://github.com/caleb221/ESP32-Leaf/blob/master/img/esp32NoCam.png" width ="150" height ="120">


FTDI Programming:


<img src="https://github.com/caleb221/ESP32-Leaf/blob/master/img/ESP32-CAM-wiring-FTDI1.png" width ="250" height ="170">

Current Output:

<img src="https://github.com/caleb221/ESP32-Leaf/blob/master/img/rnetFound1.png" width ="500" height ="250">


# In Development
The current state of the model works and provides output, but cannot save the jpeg image to an SD Card.
More details on this issue can be found here: <a href="https://github.com/espressif/esp-idf/issues/4495"> Issue 4495</a>

O-Net will be implemented in the near future.

# Documentation
A full explanation can be read in my thesis, please email me if you are interested in seeing it. As there were many steps involved I would be happy to help on any developments in the future, Please file an issue or send me an email.


the process in its simplest form goes like this:


1. pick a small-ish model ( I would probably suggest some version of mobilenets or mobilenetsV2) but ESP-Leaf is MTCNN
more on this model (and how it pertains to ESP-Leaf) is found here: https://github.com/caleb221/MTCNN-Leaf
train the model however you like ( I used caffe but the process is framework independent, you just need to know the data layout)


2. extract the model's weights and save them into a usable format (text, in the caffe example it gave me a numpy array which was nice)

3. transform the models weights into the ESP-Face library layout (see translateWeights2ESP_lib.py)
  **SETUP FOR CAFFE, just do a quick check what your framework uses and transpose accordingly.


This also creates a C header file that you'll use for inference, as a necessary feature you can cut the weights up into however many sections you need. (that was a bad explanation, what i mean is you won't have enough memory to load all your weights in at once, so separate functions are created to call upon the weights that you want to load at a given point, this is best illustrated in the R-Net section of the ESP-Leaf)


4. Finally! all that preparation work is done...now to implement your chosen model onto the ESP-32. Put all the generated C Header files into your project folder so you can use them in your code. Implement your chosen model as you see fit in C, and always test for heap failures.


some extra notes:


 check the closed issues section of ESP-Face for conversations of problems (i closed them but they might be useful)
 split convolutions where you need or your model won't fit
 
 
 make sure the raw text weights and the libraries your'e using all fit onto the ESP ( for O-Net the weights  would compile and upload just fine but the program would never start)


use heap tracing
