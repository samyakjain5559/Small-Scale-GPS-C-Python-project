/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m2extra.h
 * Author: baner103
 *
 * Created on March 17, 2019, 1:42 AM
 */

#pragma once

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "StreetsDatabaseAPI.h"
#include "LatLon.h"
#include <iostream>
#include <cfloat>
#include <chrono>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <cmath>
#include <math.h>
#include <sstream>
#include "OSMID.h"



using namespace std;
using namespace ezgl;

// loaded data structure
struct intersection{
    LatLon position;
    string name;
    bool highlight;
};

struct feature{
    string name;
    FeatureType type;
    int numberOfPoints;
    string bodyType; //open,closed or just a point
    color featureColor = WHITE;
    vector <point2d> featurePoints;
};
 
struct POI{  
    LatLon position;
    string name;
    string type;
    bool highlight;
};

// define of data structures
struct segments{
    LatLon postion_from;
    LatLon postion_to;
    int number_curvepoints;
    bool major;
    bool highlight;
    double angle;
};
//calculate the midpoint
struct midpoint{
    double x;
    double y;
};

// different vectors and variables definition
extern vector <intersection> intersections_list;
extern vector <feature> features_list;
extern vector <POI> POI_list;
extern vector <double> segments_angle;
extern vector <midpoint> mid_point;
extern vector <string> same_name;
extern int prevId; //Stores the previous marked id
extern int segment_count;
extern bool reload;
extern vector <unsigned> pathFound;
extern bool path_higlight;

extern string map_input_path;
extern string featureType;


extern double max_lat;
extern double max_lon ;
extern double min_lat ;
extern double min_lon ;
extern gdouble zoom_x ;
extern gdouble zoom_y ;
extern double zoom_global;

//flag to set night mode 
extern bool nightMode;

extern vector <unsigned> path_found;

extern surface* marker ;
extern surface* markerZoom ;


bool function_same_name(string street_name);
void initialize_Intersections();
double x_from_lon(double point);
double lon_from_x(double point);
double tiltCalculator(LatLon point1, LatLon point2);
void initialize_segments_angle();
void initialize_mid_point();
int tell_zoom_level(ezgl::renderer &g);
void colorCode(feature &temp);
void colorCodeNight(feature &temp);
void initialize_Features();
void assignPOI_Type(POI& a);
void initialize_POI();
void plot_intersections(renderer &g);
void draw_features(renderer &g);
void zoom_to_point(GtkWidget *widget, ezgl::application *application);
int find(unsigned id1, unsigned id2);
void find_button(GtkWidget *widget, ezgl::application *application);
void filter_button(GtkWidget *widget, ezgl::application *application);
void clear_button(GtkWidget *widget, ezgl::application *application);
void night_button(GtkWidget *widget, ezgl::application *application);
void map_load_button(GtkWidget *widget, ezgl::application *application);
void draw_POI(ezgl::renderer &g);
void draw_street(ezgl::renderer &g);
void draw_street_name(ezgl::renderer &g);