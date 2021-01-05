Project:     Image scene classification using GStreamer on TurboX C610


dependencies :
   1) Ubuntu System 18.04 or above
   2) Install Adb tool (Android debugging bridge)
   3) Install Tensorflow 1.13 or higher
   4) Flash latest image of c610 board
   5) Setting up the Application SDK on the host system as given in the tundercomm document

// To setup cross compile environment:

   1) Download the Application SDK from below url:
        https://thundercomm.s3.ap-northeast-1.amazonaws.com/shop/doc/1593776185472315/Turbox-C610_Application-SDK_v1.0.tar.gz
  
   2) unpack the sdk 
        tar -xzvf Turbox-C610_Application-SDK_v1.0.tar.gz

   3) execute the below script file it will ask the default target directory, press Enter and type Y
     ./oecore-x86_64-armv7ahf-neon-toolchain-nodistro.0.sh
 
    this complete the environment setup.


// To run this project 

       Step-1 : Enter below command to set up the build environment on the host system.

          $ source /usr/local/oecore-x86_64/environment-setup-armv7ahf-neon-oe-linux-gnueabi

       Step-2 : Building the binary for gstreamer image classification source code, run below command on host system.                                            
 
         $ $CC image_classification.c  classification.h -o classific `pkg-config --cflags --libs gstreamer-1.0`

		Step-3 : Create directory named  c610 in the data directory of c610 target board.

          $ adb shell
          $ adb root
          $ adb remount
          $ adb shell  mount -o remount,rw /
          # mkdir /data/c610
          # exit

		Step-4 : Push the AI models, labels and binary file to the board from the host system.

         $ adb push model/model.tflite /data/c610
         $ adb push model/labelmap.txt /data/c610
         $ adb push classific /bin/
         
		Step-5 : Execute the binary file in the adb shell environment.

         $ adb shell 
         # chmod +x /bin/classific
         # classific  dsp /data/c610/model.tflite  /data/c610/labelmap.txt 

         to stop inferencing press Ctrl + c

        step -6 : to get the inference video output   
          Using adb pull command you can get the output video with classification label on it
         
         $  adb pull /data/c610/video.mp4  


Note : SNPE models did not work with gstreamer as qtimlesnpe element is not available in GStreamer.

		