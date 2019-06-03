#include <iostream>
#include <cstdlib>
#include <vector>
#include "m3extra.h"
#include "m1.h"
#include "m4.h"
#include "m3.h"
#include "m2extra.h"
#include "m2.h"
#include "m4extra.h"
#include "m1extra.h"
#include "math.h"
#include <limits>



double geometricDistanceaprox (int id1 , int id2 ){  
    double x1 = x_from_lon ( getIntersectionPosition(id1).lon() ) ;
    double y1 = getIntersectionPosition(id1).lat() ;
    double x2 = x_from_lon ( getIntersectionPosition(id2).lon() ) ;
    double y2 = getIntersectionPosition(id2).lat() ;
    double dist = pow ( x2 - x1 , 2) + pow (y2 - y1, 2);
    return pow (dist,0.5);
}

void nearest_depot_any_pickup (const std::vector<DeliveryInfo> &deliveries, const std::vector<unsigned> &depots, int &id1 , int & id2 , int & Vertex){
    double minDist = numeric_limits <double>::infinity();
    for ( unsigned i =0 ; i < deliveries.size() ; i++ ){
        for ( unsigned j = 0 ; j < depots.size() ; j++){
            //cout<<"!!!!!!"<<endl;
            //cout<<deliveries[i].pickUp<<" "<<depots[j]<<endl;
            double distance  = geometricDistanceaprox (deliveries[i].pickUp, depots[j]);
            if(distance < minDist ){
                minDist = distance;
                Vertex = i;
                id1 = deliveries[i].pickUp;
                id2 = depots[j];
            }
        }
    }
}

unsigned nearest_depot_from_dropoff(const std::vector<DeliveryInfo>& deliveries , const std::vector<unsigned>& depots , unsigned id){
    int M = depots.size();
    double result_distance = numeric_limits <double>::infinity();
    unsigned result_id;
     
    for(int i=0; i < M; i++){
      double min_distance1 = geometricDistanceaprox(id,depots[i]);
        if(min_distance1 < result_distance){
           result_distance =  min_distance1 ;
           result_id= depots[i];
        }   
    }
    return result_id;
}


void find_nearest_point ( int currId, map <unsigned,unsigned >& allPoints, int &index , int &id ){
    double mindistance = numeric_limits <double>::infinity();
    for (auto it = allPoints.begin() ; it!=allPoints.end() ; it ++){
        double newdistance =geometricDistanceaprox (currId , it->second );
        
        if(newdistance < mindistance){
            mindistance = newdistance;
            index = it->first; // returns 2*index or 2*index+1
            id = it->second; //returns intersection id
        }
    }
}

bool check_capacity (const float current_capacity1, const float truck_capacity,map <unsigned, Delivery >& allDeliveries,int id,const float current_capacity ){
    if((current_capacity1 ) < truck_capacity){
        return true;
    }
    return false;
}

bool check_legal_dropOff(map <unsigned, Delivery > allDeliveries , int index ){
    //index is 2*index or 2*index+1
    auto it = allDeliveries.find((index - 1) /2 );
    if( it != allDeliveries.end() )
        if(it->second.picked_up && !it->second.delivered)
            return true;
    return false;
}

void find_nearest_legal_point (int currId, map <unsigned, Delivery >& allDeliveries,map <unsigned,unsigned > allPoints, int &index , int &id,float current_capacity, const float truck_capacity,const std::vector<DeliveryInfo>& deliveries,float &weight ){
    
    if(allPoints.size() == 0){
        index == -1;
        id = currId ; 
    }
        
    
    while(allPoints.size() > 0){
        int temp_id = 0;
        int temp_index = 0;
        find_nearest_point ( currId,allPoints,temp_index ,temp_id); // index is 2*index or 2*index + 1
        
        if(temp_index % 2 == 0) {  //its a pickup point
            if(check_capacity (current_capacity + deliveries[temp_index/2].itemWeight ,truck_capacity,allDeliveries,temp_index/2, current_capacity)){
                id = temp_id;
                index = temp_index;
                weight = deliveries[index/2].itemWeight;
                break;
            }
            else
                allPoints.erase(temp_index);
        }else {
            if(check_legal_dropOff(allDeliveries ,temp_index )){
                id = temp_id;
                index = temp_index;
                weight = deliveries[(index - 1)/2].itemWeight;
                break;
            }
            else
                allPoints.erase(temp_index);
        }
    }
}

std::vector<CourierSubpath> traveling_courier ( const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& depots, const float right_turn_penalty, 
 const float left_turn_penalty, const float truck_capacity) {
   
    
    int N = deliveries.size();
    vector <CourierSubpath> results; // to store all results
    
    CourierSubpath obj; // an item in the results vector
    map <unsigned, Delivery > allDeliveries;
    map <unsigned,unsigned > allPoints; // 2*index + 1 for dropOff and 2*index for 
    
    
    for (unsigned i =0 ;i < deliveries.size() ; i++){
        Delivery obj1 (deliveries[i].pickUp , deliveries[i].dropOff, deliveries[i].itemWeight);
        allDeliveries.insert(pair <unsigned, Delivery> (i,obj1));
        allPoints.insert(pair <unsigned,unsigned> (2*i,deliveries[i].pickUp)); // EVEN KEY FOR PICKUP POINT
        allPoints.insert(pair <unsigned,unsigned> (2*i + 1 , deliveries[i].dropOff));
    }
    

    int first_depot;
    int first_pickup;
    int vertex;
    nearest_depot_any_pickup (deliveries , depots ,first_pickup , first_depot , vertex);
    float capacity = 0;
    int current_index = vertex; //cur_index stores in "decrypted" format, i.e index
    int current_id = first_pickup;
    bool is_pickup = true;//after depot we always go to a pickup
       
    //go to first pickup
    obj.pickUp_indices = {};
    obj.subpath = find_path_between_intersections(first_depot, first_pickup,right_turn_penalty,left_turn_penalty);
    obj.start_intersection =first_depot;
    obj.end_intersection = first_pickup;
    allPoints.erase(2*current_index);
    results.push_back (obj);
    capacity += deliveries[current_index].itemWeight;   
    
    if(deliveries.size() == 1){
        CourierSubpath delivery;
        delivery.pickUp_indices.push_back (current_index);
        delivery.subpath = find_path_between_intersections(current_id,deliveries[0].dropOff,right_turn_penalty,left_turn_penalty);
        delivery.start_intersection = current_id;
        delivery.end_intersection = deliveries[0].dropOff;
        results.push_back(delivery);
        allDeliveries[current_index].picked_up = true;
        current_index = 0;   
    }
     
    while(allPoints.size() > 0 && deliveries.size() != 1){
        CourierSubpath delivery;
        int next_index;
        int next_id ;
        float weight = 0;
        
        find_nearest_legal_point (current_id, allDeliveries,allPoints,next_index,next_id,capacity,truck_capacity,deliveries,weight ); // next_index is 2*index or 2*index +1
        
        if(next_index == -1){
            break;
        }
           
        
        if(is_pickup ){
            delivery.pickUp_indices.push_back (current_index);
            delivery.subpath = find_path_between_intersections(current_id,next_id,right_turn_penalty,left_turn_penalty);
            delivery.start_intersection = current_id;
            delivery.end_intersection = next_id;
            results.push_back(delivery);
            allDeliveries[current_index].picked_up = true;
            float free_space = truck_capacity - capacity;
          // cout<<"Pickup : "<<current_index<<endl;    
        }else{
            delivery.pickUp_indices = {};
            delivery.subpath = find_path_between_intersections(current_id,next_id,right_turn_penalty,left_turn_penalty);
            delivery.start_intersection = current_id;
            delivery.end_intersection = next_id;
            results.push_back(delivery);
            allDeliveries[current_index].delivered = true;
            float free_space = truck_capacity - capacity;
           // cout<<"DropOff : "<<current_index<<endl;
        }
         
        //need to "decrypt" next_index
        if(next_index % 2 == 0){
            current_index = next_index / 2 ;
            current_id = next_id;
            is_pickup = true;
            allPoints.erase(next_index);
            capacity += weight;
        }else{
            //cout<<"!!!!!! "<<allPoints.size()<<"  "<<current_index<<"   "<<next_index<<endl;
            current_index = ( next_index - 1) / 2 ;
            current_id = next_id;
            is_pickup = false;
            allPoints.erase(next_index);
            capacity -= weight;
        }
        
    }
    
    
    
    //delivery to  last depot   
   // cout<<"Current capacity is "<<capacity<<endl;
    CourierSubpath last;
    int last_depot =  nearest_depot_from_dropoff (deliveries,depots,deliveries[current_index].dropOff);
    last.pickUp_indices = {};
    last.pickUp_indices.clear();
    last.subpath = find_path_between_intersections(deliveries[current_index].dropOff, last_depot,right_turn_penalty,left_turn_penalty);
    last.start_intersection = deliveries[current_index].dropOff;
    last.end_intersection = last_depot;
    results.push_back(last);
//    cout<<"done"<<endl;  
    //unsigned size = results.size();
   // for(int i = 0 ; i < size ; i++){
        
   // }
    return results;
}