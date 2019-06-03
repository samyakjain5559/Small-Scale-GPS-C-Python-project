#include "m3.h"
#include "m2.h"
#include <iostream>
#include <cfloat>
#include <chrono>
#include <cstdlib>
#include <vector>
#include <thread>
#include <queue>
#include "m1extra.h"
#include "m2extra.h"
#include "m3extra.h"
#include "m1.h"



using namespace std;
  
TurnType find_turn_type(unsigned street_segment1, unsigned street_segment2){
   TurnType type;   
   int id1 = getInfoStreetSegment(street_segment1).streetID;
   int id2 = getInfoStreetSegment(street_segment2).streetID;
   if(id1 == id2){
         return TurnType ::STRAIGHT;
   }
   
   unsigned curvepoints_count_seg1 =getInfoStreetSegment(street_segment1).curvePointCount;
   unsigned curvepoints_count_seg2 =getInfoStreetSegment(street_segment2).curvePointCount;
   
   signed sharing_intersection = -1;
   LatLon start_point;
   LatLon mid_point_common;
   LatLon end_point;
      
   /*
     -> x ->             // specifying the cases of cross product
    */
   if (getInfoStreetSegment(street_segment1).to == getInfoStreetSegment(street_segment2).from ){
       
       sharing_intersection = getInfoStreetSegment(street_segment1).to;
       
       if (curvepoints_count_seg1 > 0) {
           start_point = getStreetSegmentCurvePoint(curvepoints_count_seg1 - 1, street_segment1);         // start point gets the start point
       } else {
           start_point = getIntersectionPosition(getInfoStreetSegment(street_segment1).from);
       }
       
       if (curvepoints_count_seg2 > 0) {
           end_point = getStreetSegmentCurvePoint(0, street_segment2);
       } else {
           end_point = getIntersectionPosition(getInfoStreetSegment(street_segment2).to);
       }
   }
   
   
   /*
     -> x <- 
    */
   if (getInfoStreetSegment(street_segment1).to == getInfoStreetSegment(street_segment2).to ){
       
       sharing_intersection = getInfoStreetSegment(street_segment1).to;
       
       if (curvepoints_count_seg1 > 0) {
           start_point = getStreetSegmentCurvePoint(curvepoints_count_seg1 - 1, street_segment1);         // start point gets the start point
       } else {
           start_point = getIntersectionPosition(getInfoStreetSegment(street_segment1).from);
       }
       
       if (curvepoints_count_seg2 > 0) {
           end_point = getStreetSegmentCurvePoint(curvepoints_count_seg2 - 1, street_segment2);
       } else {
           end_point = getIntersectionPosition(getInfoStreetSegment(street_segment2).from);
       }
   }
   
     /*
     <- x <- 
    */
   if (getInfoStreetSegment(street_segment1).from == getInfoStreetSegment(street_segment2).to ){
       
       sharing_intersection = getInfoStreetSegment(street_segment1).from;
       
       if (curvepoints_count_seg1 > 0) {
           start_point = getStreetSegmentCurvePoint(0, street_segment1);         // start point gets the start point
       } else {
           start_point = getIntersectionPosition(getInfoStreetSegment(street_segment1).to);
       }
       
       if (curvepoints_count_seg2 > 0) {
           end_point = getStreetSegmentCurvePoint(curvepoints_count_seg2 - 1, street_segment2);
       } else {
           end_point = getIntersectionPosition(getInfoStreetSegment(street_segment2).from);
       }
   }
   
   
    /*
     <- x -> 
    */
   if (getInfoStreetSegment(street_segment1).from == getInfoStreetSegment(street_segment2).from ){
       
       sharing_intersection = getInfoStreetSegment(street_segment1).from;
       
       if (curvepoints_count_seg1 > 0) {
           start_point = getStreetSegmentCurvePoint(0, street_segment1);         // start point gets the start point
       } else {
           start_point = getIntersectionPosition(getInfoStreetSegment(street_segment1).to);
       }
       
       if (curvepoints_count_seg2 > 0) {
           end_point = getStreetSegmentCurvePoint(0, street_segment2);
       } else {
           end_point = getIntersectionPosition(getInfoStreetSegment(street_segment2).to);
       }
   }
   
   if(sharing_intersection == -1)
        return TurnType::NONE;
  
      mid_point_common = getIntersectionPosition(sharing_intersection);
        
      double x_start_point= x_from_lon (start_point.lon());
      double y_start_point= start_point.lat();
      
      double x_mid_point= x_from_lon (mid_point_common.lon());
      double y_mid_point= mid_point_common.lat();
      
      double x_end_point= x_from_lon (end_point.lon());
      double y_end_point= end_point.lat();
      
      double ax = x_mid_point - x_start_point;
      double ay = y_mid_point - y_start_point;
      double bx = x_end_point - x_mid_point;
      double by = y_end_point - y_mid_point;
            
      double cz= ax*by - ay*bx;
            
      if( cz>0){
         return TurnType ::LEFT;
       }else if(cz<=0) {
         return  TurnType ::RIGHT;
       }

}

double compute_path_travel_time(const std::vector<unsigned>& path,const double right_turn_penalty,const double left_turn_penalty){
    TurnType turn;
    int leftcount =0;
    int rightcount =0;
    double time = 0;
    double timetotal = 0;
    if(path.size() == 0)
        return -1;
       for(auto it = path.begin() ;it != path.end() ; ++it){
        if(it != path.end() - 1){
            turn=find_turn_type(*it,*(it + 1));
            if (turn == TurnType::LEFT){
                leftcount++;
            }
            if (turn == TurnType::RIGHT){
                rightcount++; 
            } 
       }
        time = time + find_street_segment_travel_time(*it);
       }
       timetotal = time+leftcount*left_turn_penalty+rightcount*right_turn_penalty;
    return timetotal;
}

vector <unsigned> traceBack(unordered_map <unsigned,Node> &data, unsigned source, unsigned dest){
    vector <unsigned> result;
    
    unsigned current_node = dest;
    
    while (current_node != source) {
        result.insert(result.begin(), data[current_node].prevsegid);
        current_node = data[current_node].prevNode;
    }
    
    return result;
}

bool not_making_one_way_violation(unsigned intersection, unsigned seg) {
    //if it is an one way, it HAS to go from 'from' to 'to'
    return !(getInfoStreetSegment(seg).to == intersection && getInfoStreetSegment(seg).oneWay);
}

vector <unsigned> djisktra (const unsigned source,const unsigned dest,const double right_turn_penalty,const double left_turn_penalty){
    map <double,Node> waveFront; //stores Node, we need to visit next. We use a map since it sorts based on travel time and lets us 
                                 //to go to Nodes with shorter travel times first
    unordered_map <unsigned, Node> data; //stores all the Nodes we have visited or seen
    
    Node src = Node(source , intersectionsAndSegments[source]);
    src.bestTime = 0;
    waveFront[src.bestTime] = src;
    data[source] = src;
    
    while (waveFront.size() > 0) {
        Node current_node = waveFront.begin()->second;
        waveFront.erase(waveFront.begin());
        data[current_node.id].visited = true; //set visited to true
        
        if (current_node.id == dest) {
            return traceBack(data, source, dest); //if we reached destination
        }
        
        for (auto it = current_node.outSegments.begin(); it != current_node.outSegments.end(); it++) {
            if (not_making_one_way_violation(current_node.id, *it)) { //check that we are not going the wrong way down an one way
                unsigned to_node_id = getInfoStreetSegment(*it).from == current_node.id ? getInfoStreetSegment(*it).to : getInfoStreetSegment(*it).from;
                unordered_map<unsigned, Node>::iterator data_reference = data.find(to_node_id); //check if Node is already there in data
                if (data_reference == data.end()) { //check if the node has not been seen
                    //if mot seen, we initialize data
                    Node to_node(to_node_id, intersectionsAndSegments[to_node_id]);
                    to_node.prevsegid = *it;
                    to_node.prevNode = current_node.id;
                    to_node.bestTime = find_street_segment_travel_time(*it) + current_node.bestTime;
                    
                    if (current_node.id != source) {
                        TurnType turn = find_turn_type(current_node.prevsegid, *it);
                        if (turn == TurnType::LEFT) {
                            to_node.bestTime += left_turn_penalty;
                        } else if (turn == TurnType::RIGHT) {
                            to_node.bestTime += right_turn_penalty;
                        }
                    }
                    
                    //storing the node in data for future use
                    data[to_node_id] = to_node;
                    waveFront[to_node.bestTime] = to_node;
                    
                } else if (!data_reference->second.visited) { //if it has been seen, check if it has been visited
                    double this_travel_time = find_street_segment_travel_time(*it) + current_node.bestTime;
                    
                    if(current_node.prevsegid != -1){
                        TurnType turn = find_turn_type(current_node.prevsegid, *it);
                            if (turn == TurnType::LEFT) {
                             this_travel_time += left_turn_penalty;
                            } else if (turn == TurnType::RIGHT) {
                                this_travel_time += right_turn_penalty;
                            }
                    }
                    
                    double previous_travel_time = data_reference->second.bestTime;
                    
                    //compare new time with previous time and update accordingly
                    if (this_travel_time < previous_travel_time) {
                        waveFront.erase(previous_travel_time);
                        data.erase(data_reference);
                        
                        Node to_node(to_node_id, intersectionsAndSegments[to_node_id]);
                        to_node.prevsegid = *it;
                        to_node.prevNode = current_node.id;
                        to_node.bestTime = this_travel_time;
                        
                        data[to_node_id] = to_node;
                        waveFront[this_travel_time] = to_node;
                    }
                }
            }
        }
    }
    return {};
}


void give_directions(const std::vector<unsigned>& path){
    TurnType turn;
    int i = 1;
    for(auto it = path.begin() ;it != path.end(); ++it){
        if(i== 1){
            cout<<i<<") ";
            cout<<"Walk "<< find_street_segment_length(*it) <<" meters Straight Down "<< getStreetName (getInfoStreetSegment(*it).streetID )<<endl;
        } else if (it != path.end() - 1){
            turn=find_turn_type(*it,*(it + 1));
            if (turn == TurnType::LEFT){
                cout<<i<<") ";
                  cout<<"Walk "<< find_street_segment_length(*it) <<" meters Straight Down "<< getStreetName (getInfoStreetSegment(*it).streetID )<<endl;
                cout << "   Turn Left onto " << getStreetName (getInfoStreetSegment(*(it+1)).streetID ) <<endl;
            }
            else if (turn == TurnType::RIGHT){
                cout<<i<<") ";
                cout<<"Walk "<< find_street_segment_length(*it) <<" meters Straight Down "<< getStreetName (getInfoStreetSegment(*it).streetID )<<endl;
                cout<< "   Turn Right onto " << getStreetName (getInfoStreetSegment(*(it+1)).streetID ) <<endl;
            }
            else if(turn == TurnType::STRAIGHT){
                cout<<i<<") ";
                double total_distance = find_street_segment_length(*it);
                it++;
                while(it!=path.end() - 1){
                    if(getInfoStreetSegment(*(it+1)).streetID != getInfoStreetSegment(*(it)).streetID ){
                        --it;
                     break;
                    }
                    total_distance += find_street_segment_length(*it);
                    ++it;
                }
                cout<<"Walk "<< total_distance <<" meters Straight Down "<< getStreetName (getInfoStreetSegment(*(it)).streetID )<<endl;
            }
        }
        i++;
    }
}

std::vector<unsigned> find_path_between_intersections(const unsigned intersect_id_start,  const unsigned intersect_id_end,const double right_turn_penalty, const double left_turn_penalty){
    if(intersect_id_start == intersect_id_end)
        return {};
     return djisktra(intersect_id_start,intersect_id_end,right_turn_penalty,left_turn_penalty);
}

