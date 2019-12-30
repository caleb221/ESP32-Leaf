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

# In Development
The current state of the model works and provides output, but cannot save the jpeg image to an SD Card.
More details on this issue can be found here: <a href="https://github.com/espressif/esp-idf/issues/4495"> Issue 4495</a>
# Documentation
A full explanation can be read in my thesis (the implementation chapter is the most useful for a guide on how to use the code). As there were many steps involved I would be happy to help on any developments in the future, Please file an issue or send me an email.