/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m3extra.h
 * Author: baner103
 *
 * Created on March 17, 2019, 4:15 AM
 */

#pragma once 

#include <iostream>
#include <cfloat>
#include <chrono>
#include <cstdlib>
#include <vector>
#include <thread>
#include <queue>
#include <unordered_map>
#include "StreetsDatabaseAPI.h"
#include "LatLon.h"
#include <iostream>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <set>
#include <list>
#include <iterator>
#include <algorithm>
#include <quadmath.h>
#include <cmath>
#include <iomanip> 
#include <limits>

using namespace std;

struct Node{
    double bestTime = numeric_limits <double>::infinity();
    int prevsegid = -1;
    int id;
    bool visited = false;
    int prevNode = -1;
    vector <unsigned> outSegments;
    Node (int i, vector <unsigned> segments){
        id =i ;
        outSegments = segments;
    }
    Node(){
        
    }
};


vector <unsigned> djisktra(unsigned source,unsigned dest,const double right_turn_penalty,const double left_turn_penalty);
void give_directions(const std::vector<unsigned>& path);
