/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m4extra.h
 * Author: baner103
 *
 * Created on April 1, 2019, 4:24 PM
 */

#pragma once



unsigned nearest_depot_from_fix_pickup();

double geometricDistanceaprox (int id1 , int id2 );
void nearest_depot_any_pickup ( const std::vector<DeliveryInfo> deliveries, const std::vector<unsigned> depots ,int &id1 , int & id2); 
void nearest_depot_any_dropoff ( const std::vector<DeliveryInfo> deliveries, const std::vector<unsigned> depots ,int &id1 , int & id2); 

struct Delivery{
    bool picked_up = false;
    bool delivered = false;
    unsigned pickup_id;
    unsigned dropOff_id;
    float weight;
    Delivery (unsigned i, unsigned j,float w){
        pickup_id = i;
        dropOff_id = j;
        weight = w;
    }
    Delivery(){
        
    }
};

