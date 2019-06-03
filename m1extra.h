/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m1extra.h
 * Author: baner103
 *
 * Created on March 16, 2019, 11:49 PM
 */

#pragma once


#include <unordered_map>
#include "StreetsDatabaseAPI.h"
#include "LatLon.h"
#include <iostream>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <set>
#include <iterator>
#include <algorithm>
#include <quadmath.h>
#include <cmath>
#include <iomanip> 

using namespace std;


/*
 ------------- GLOBAL VARIABLES --------------
 */
extern vector < vector <unsigned> >  streetAndSegments; //streets and its street segments
extern vector <vector<unsigned> >  streetAndIntersections; //streets and intersections
extern multimap < string, unsigned > streetNames; //map containing all the street names and street ids
extern vector < vector <unsigned> > intersectionsAndSegments;//intersections of segments
extern vector <double long> segmentTime;//time taken to travel each segment



/*
 ------------- GLOBAL VARIABLES --------------
 */

void pairStreetsandSegments();
void pairStreetsandIntersections();
void pairstreetNamesandStreetIds();
void pairIntersectionsAndSegments();
void calculateTime();
