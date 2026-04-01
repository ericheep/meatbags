# [meatbags](https://www.mit.edu/people/dpolicar/writing/prose/text/thinkingMeat.html)
openFrameworks 2D LiDAR blob detection for macOS and Windows
supports the Hokuyo and Orbbec line of 2D LiDAR sensors

This project clusters data to be used for blob detection purposes as a companion for creative applications.

## installation

You can download a built binary from the releases sidebar or you can compile the software using the [openFrameworks](https://openframeworks.cc/download/) architecture.

If compiling from source code, I recommend using the [projectGenerator](https://openframeworks.cc/learning/01_basics/create_a_new_project/) software to import the project into your IDE of choice.

This project was developed with openFrameowrks v0.12.1, it uses the following openFrameworks addons; all included in the main package. Be sure to add them using the projectGenerator application.

- ofxGui
- ofxOsc

This project also uses a modified version of ofxNetwork that is extended to control which ethernet adapter Meatbags uses to create its TCP connections. This modified version is included in the source code.

## use
After a configuration is set it is recommended to toggle __headless mode__ and run Meatbags as a background application. It can be set to send OSC out; the OSC schema is provided below:

    /blob index x y width height velocityX velocityY filterIndex1 filterIndex2 ...
    /blobs index1 x y index2 x y ...
    /blobsActive index1 index2 ...
    
    /filter index isInhabited blobDistanceToCentroid
    /filterBlob filterIndex blobIndex x y width height velocityX velocityY
    /filterBlobs filterIndex blobIndex1 x1 y1 blobIndex2 x2 y2 ...
    
    /generalStatus sensorIndex status
    /laserStatus sensorIndex status
    /connectionStatus sensorIndex status

__Created by artists for artists.__ This software is provided free for artists to use in their art installs and free for educational purposes. The license is provided on my [here](https://github.com/ericheep/meatbags/blob/main/LICENSE.md). For commercial purposes email me at ericheep@gmail.com.

## external code

This project uses the DBSCAN algorithm for clustering the LiDAR measurements received from Hokuyo sensor. A performant C++ implementaion of DBSCAN by [Eleobert](https://github.com/Eleobert/dbscan) is used to cluster the streaming LiDAR data.


## tested sensors
- Orbbec Pulsar SL450
- Hokuyo UST-10LX
- Hokuyo UST-20LX

## tested operating systems
- macOS Sequoia, Tahoe
- Windows 10
