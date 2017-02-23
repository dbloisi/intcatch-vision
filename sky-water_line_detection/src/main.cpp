/**
 *
 * main.cpp
 *
 * Author: Domenico Daniele Bloisi
 * email: domenico.bloisi@gmail.com
 * 
 * This file is part of a program for detecting the sky-water line
 * in images from a camera mounted on a small surface autonomous vessel
 *
 **/

#include <iostream>
#include <string>

#include "skywaterdetection.hpp"

using namespace std;

/**
 *
 * function headers
 *
 **/
void help();

/**
 *
 * main function
 *
 **/
int main(int argc, char* argv[])
{
    bool in_set = false;
    bool out_set = false;
    bool is_gui = false;
    bool is_live = false;
    float alpha = 0.2f;
	
    string cap_file = "";
    string outvideo_filename = ""; 
	
    //print help information
    help();

    //check for the input parameter correctness
    if(argc < 3) {
        cerr <<"Incorret input list" << endl;
        cerr <<"exiting..." << endl;
        return EXIT_FAILURE;
    }
    
    for (int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "-in") == 0) {
            cap_file.assign(argv[++i]);
            in_set = true;
        }
        else if(strcmp(argv[i], "-out") == 0) {
            outvideo_filename.assign(argv[++i]);
            out_set = true;
        }
        else if(strcmp(argv[i], "-live") == 0) {
            is_live = true;
        }
        else if(strcmp(argv[i], "-gui") == 0) {
            is_gui = true;
        }
        else {
            //error in reading input parameters
            cerr <<"Please, check the input parameters." << endl;
            cerr <<"Exiting..." << endl;
            return EXIT_FAILURE;
        }
    }
	
    if(!in_set) {
        //error in reading input parameters
        cerr <<"Please, check the input parameters." << endl;
        cerr <<"Exiting..." << endl;
        return EXIT_FAILURE;
    }

    SkyWaterDetector d(cap_file, alpha, outvideo_filename, is_live, is_gui);

    d.detect();
    
    return EXIT_SUCCESS;
}

/**
 *
 * function: help
 *
 **/
void help()
{
    cout
    << "--------------------------------------------------------------------------" << endl
    << "This program detect the sky-water line in images from a camera"             << endl
    << "mounted on a small surface autonomous vessel."                              << endl
                                                                                    << endl
    << "Usage:"                                                                     << endl
    << "./skywaterdetect -in <source> {-out <destination> | -gui | -live | -alpha <value>}"       << endl
    << "for example: ./skywaterdetect -in http://10.5.5.9:8080/live/amba.m3u8 -out video.avi -live" << endl
    << "or: ./skywaterdetect -in video.mp4 -gui -alpha 0.4"                         << endl
    << "--------------------------------------------------------------------------" << endl
    << endl;
}

