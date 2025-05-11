# [meatbags](https://www.mit.edu/people/dpolicar/writing/prose/text/thinkingMeat.html)
openFrameworks Hokuyo 2D LiDAR blob detection

This project implements the Hokuyo UST-LX LiDAR protocol and clusters data to be used as blob detection for creative applications.

## installation

You can download a built binary from the releases sidebar (Mac OS only) or you can compile the software using the [openFrameworks](https://openframeworks.cc/download/) architecture.

If compiling from source code, I recommend using the [projectGenerator](https://openframeworks.cc/learning/01_basics/create_a_new_project/) software to import the project into your IDE of choice.

This project was developed with openFrameowrks v0.12.0, it uses the following openFrameworks addons; all included in the main package. Be sure to add them using the projectGenerator application.

- ofxGui
- ofxOsc

Additionally, this project uses the [ofxDropdown](https://github.com/roymacdonald/ofxDropdown) addon and must be included in your addons folder if you're buliding this project from source.

This project also uses a modified version of ofxNetwork that is extended to control which ethernet adapter Meatbags uses to create its TCP connections. This modified version is included in the source code.

## use

After opening and closing the application Meatbags will create a batch of JSON files that are automatically loaded on startup with the option to auto-save on exit. These hold the configuration state of the application and allow easy startup for art install scenarios. 

This software is provided free for artists to use in their art installs and free for educational purposes. For commercial purposes email me at ericheep@gmail.com. Read the [license](https://github.com/ericheep/meatbags/blob/main/LICENSE.md) for further terms.

## external code

This project uses the DBSCAN algorithm for clustering the LiDAR measurements received from Hokuyo sensor. A performant C++ implementaion of DBSCAN by [Eleobert](https://github.com/Eleobert/dbscan) is used to cluster the streaming LiDAR data.

A C++11 implementation of C++20's std::span is by [tcbrindle](https://github.com/tcbrindle/span
) also used to enable Eleobert's dbscan implementation.

## tested sensors

- Hokuyo UST-10LX
- Hokuyo UST-20LX

## todo

- add rectangle behind blob info
- add circle shape to filter options

![meatbags](https://github.com/user-attachments/assets/e2f610ac-3d7d-4581-a371-1cdfcdc1b13c)
