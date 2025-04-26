# [meatbags](https://www.mit.edu/people/dpolicar/writing/prose/text/thinkingMeat.html)
openFrameworks Hokuyo 2D LiDAR blob detection

This project implements the Hokuyo UST-LX LiDAR protocol and clusters data to be used as blob detection for creative applications.

## installation

You can download a built binary from the releases sidebar (Mac OS only) or you can compile the software using the [openFrameworks](https://openframeworks.cc/download/) architecture.

If compiling from source code, I recommend using the [projectGenerator](https://openframeworks.cc/learning/01_basics/create_a_new_project/) software to import the project into your IDE of choice.

This project was developed with openFrameowrks v0.12.0, it uses the following openFrameworks addons; all included in the main package.

- ofxNetwork
- ofxGui
- ofxOsc

## use

After opening and closing the application Meatbags will create a batch of JSON files that are automatically loaded on startup and saved on exit. These hold the configuration state of the application and allow easy startup for art install scenarios. 

## external code

This project uses the DBSCAN algorithm for clustering the LiDAR measurements received from Hokuyo sensor. A performant C++ implementaion of DBSCAN by [Eleobert](https://github.com/Eleobert/dbscan) is used to cluster the streaming LiDAR data.

A C++11 implementation of C++20's std::span is by [tcbrindle](https://github.com/tcbrindle/span
) also used to enable Eleobert's dbscan implementation.

This project uses the very cool font [Hack](https://github.com/source-foundry/Hack).

## tested sensors

- Hokuyo UST-10LX
- Hokuyo UST-20LX

## todo

- add video mute

![meatbags](https://github.com/user-attachments/assets/34e793e9-e960-452f-a8a4-a0066824f212)


