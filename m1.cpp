/* 
 * Copyright 2019 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "m1extra.h"
#include "m2extra.h"
#include "m1.h"

using namespace std;


vector < vector <unsigned> >  streetAndSegments; //streets and its street segments
vector <vector<unsigned> >  streetAndIntersections; //streets and intersections
multimap < string, unsigned > streetNames; //map containing all the street names and street ids
vector < vector <unsigned> > intersectionsAndSegments;//intersections of segments
vector <double long> segmentTime;//time taken to travel each segment




/*
 ----------FUNCTIONS TO INITIALIZING DATA INTO THE GLOABL VARIABLES-----------
 */





void pairStreetsandSegments(){
    unsigned numStreets = getNumStreets();
    unsigned numStreetSegments = getNumStreetSegments();
    vector<unsigned> segments;
    
    for(unsigned i =0;i<numStreets;i++){
        vector <unsigned> temp ={};
        streetAndSegments.push_back(temp);
    }
    
    for(unsigned i=0;i<numStreetSegments;i++){
        int id = getInfoStreetSegment(i).streetID;
        streetAndSegments[id].push_back(i);
    }
}

void pairStreetsandIntersections(){
    unsigned numStreets = getNumStreets();
    unsigned numStreetSegments = getNumStreetSegments();
    vector <unsigned> temp = {};
    for(unsigned i=0;i<numStreets;i++){
        streetAndIntersections.push_back(temp);
    }
    
    
    for(unsigned i=0;i<numStreetSegments;i++){
        int id = getInfoStreetSegment(i).streetID;
            unsigned from = getInfoStreetSegment(i).from;
            unsigned to = getInfoStreetSegment(i).to;
            if(find(streetAndIntersections[id].begin(), streetAndIntersections[id].end(),from) ==  streetAndIntersections[id].end()){ 
                streetAndIntersections[id].push_back(from); 
            }
             if(find(streetAndIntersections[id].begin(), streetAndIntersections[id].end(),to) ==  streetAndIntersections[id].end()){ 
                 streetAndIntersections[id].push_back(to);  
            }
    }
}


void pairstreetNamesandStreetIds(){
    for(int i= 0; i<getNumStreets(); i++){
        string s = getStreetName(i);
        transform (s.begin(),s.end(),s.begin(),::tolower);
        streetNames.insert(pair<string,unsigned>(s,i));
    }
}
       
void pairIntersectionsAndSegments(){
    int numIntersections = getNumIntersections();
    for(int i = 0 ; i<numIntersections ; i++){
        unsigned segmentsPerIntersection = getIntersectionStreetSegmentCount(i);
        vector <unsigned> streetSegments;
        for(unsigned j = 0 ; j < segmentsPerIntersection ; j++){
        streetSegments.push_back( getIntersectionStreetSegment(j,i));
        }
        intersectionsAndSegments.push_back(streetSegments);
    }
} 

/*
 ----------FUNCTIONS TO INITIALIZING DATA INTO THE GLOABL VARIABLES-----------
 */



/*
 --------HELPER FUNCTION------------
 */
void calculateTime(){
    for(int i =0; i< getNumStreetSegments();i++){
        double long time;
        time = ( find_street_segment_length(i) ) /( ( getInfoStreetSegment(i).speedLimit ) * (5.0/18) );
        segmentTime.push_back(time);
    }
}
/*
 --------HELPER FUNCTION------------
 */


bool load_map(std::string map_path) {
    bool load_successful = false; //Indicates whether the map has loaded 
                                  //successfully
    if(!loadStreetsDatabaseBIN(map_path))
        return false;
    
    //Loading map related data structures here
    pairStreetsandSegments();
    pairStreetsandIntersections();
    calculateTime();
    pairstreetNamesandStreetIds();
    pairIntersectionsAndSegments();
    load_successful = true;
    return load_successful;
}

void close_map() { 
    //Cleaning up our map related data structures here
     streetAndIntersections.clear();
     streetAndSegments.clear();
     intersectionsAndSegments.clear();
     streetNames.clear();
     segmentTime.clear();
     closeStreetDatabase();
     return; 
}

//Returns the length of the given street segment in meters
double find_street_segment_length(unsigned segment_id){
    unsigned intersection1_id;
     unsigned intersection2_id; 
    unsigned curvepoints_count;
    LatLon diffrent;
    LatLon all;
    unsigned id=0;
    LatLon all2;
    LatLon first_intersection;
    LatLon second_intersection;
    LatLon all3;
    LatLon all4;
    double long lenght=NULL;
 
    intersection1_id = getInfoStreetSegment(segment_id).from;
    // get the street segment
    intersection2_id = getInfoStreetSegment(segment_id).to;
    getNumStreetSegments();  
    // get first intersection;
    first_intersection=getIntersectionPosition(intersection1_id); 
    //get second intersection
    second_intersection=getIntersectionPosition(intersection2_id);
    curvepoints_count =getInfoStreetSegment(segment_id).curvePointCount;
            // extra
    
    if (curvepoints_count == 0){
        //finding distance between 2 intersection as no curve point
        lenght = find_distance_between_two_points(first_intersection,second_intersection);
    }
    if (curvepoints_count != 0){
        diffrent=getStreetSegmentCurvePoint(id,segment_id);
        lenght = find_distance_between_two_points(first_intersection,diffrent);
        for(id=1; id < curvepoints_count; id++){
            //distance between all curve point
            all=getStreetSegmentCurvePoint(id-1,segment_id);
            all2=getStreetSegmentCurvePoint(id,segment_id);
            lenght=lenght+find_distance_between_two_points(all,all2);
    }
        // finding distance between cuvepopint and last intersection
        all3=getStreetSegmentCurvePoint(id-1,segment_id);
        all4=second_intersection;
        lenght=lenght+find_distance_between_two_points(all3,all4);
    }
return lenght;     // return lenght
}

//Returns the length of the specified street in meters
double find_street_length(unsigned street_id){                           
    unsigned segment_count;
    unsigned iterator;
    long double lenght = NULL;
    // started the variable
    vector<unsigned> streetsegments;
    //store street segments
    streetsegments= find_street_street_segments(street_id);
    segment_count = streetsegments.size();
    for(iterator=0; iterator<segment_count; iterator++){
     //distance between street segments
     lenght =lenght+ find_street_segment_length(streetsegments[iterator]);
    }
    return lenght;
}  

//Returns the travel time to drive a street segment in seconds 
//(time = distance/speed_limit)
double find_street_segment_travel_time(unsigned street_segment_id){
    return segmentTime[street_segment_id];
}

//Returns the nearest point of interest to the given position
unsigned find_closest_point_of_interest(LatLon poi){
    
    unsigned long long intersection_count=getNumPointsOfInterest();
    unsigned long long iterator;
    LatLon intersection_point;
    LatLon first_intersection_point;
    unsigned long long id;
    double update;
    // created update so that distance is updated
    first_intersection_point=getIntersectionPosition(0);
    // setting the default
    double leastdistance=find_distance_between_two_points(poi,first_intersection_point);
    for(iterator=1; iterator<intersection_count; iterator++){
        //iterating through all intersections
        intersection_point=getPointOfInterestPosition(iterator); 
        update=find_distance_between_two_points(poi,intersection_point);
        // compare and update the shortest distance of travel
        if(update<leastdistance){
            leastdistance=update;     
            id=iterator;
        }
    } 
  return id;
}
  
//Returns the street segments for the given intersection 
std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id){
    return intersectionsAndSegments[intersection_id];
}

//Returns the street names at the given intersection (includes duplicate street 
//names in returned vector)
std::vector<std::string> find_intersection_street_names(unsigned intersection_id){
    vector <unsigned> streetSegments = find_intersection_street_segments(intersection_id);
    vector <string> streetNames1;
    InfoStreetSegment segmentInfo;
    int count = 0;
    for(auto it = streetSegments.begin() ; it != streetSegments.end() ; it++){
        segmentInfo = getInfoStreetSegment(streetSegments[count]);
        streetNames1.push_back(getStreetName(segmentInfo.streetID));
        count++;
    }
    return streetNames1;
}

//Returns true if you can get from intersection1 to intersection2 using a single 
//street segment (hint: check for 1-way streets too)
//corner case: an intersection is considered to be connected to itself
bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2){
    vector <unsigned>segmentId1 = find_intersection_street_segments(intersection_id1);
    vector <unsigned>segmentId2 = find_intersection_street_segments(intersection_id2);
    bool sameSegment = false;
    int lenght1 = segmentId1.size();
    int lenght2 = segmentId2.size();
    int sameSegmentId;
    for(int i = 0 ; i<lenght1 ; i++){
        for(int j = 0 ; j<lenght2 ; j++){
            if(segmentId1[i] == segmentId2[j]){
                sameSegment = true;
                sameSegmentId = segmentId1[i];
            }
        }
    }
    if(sameSegment){
        InfoStreetSegment segment1Info = getInfoStreetSegment(sameSegmentId);
        if(segment1Info.oneWay){
            if(segment1Info.from = intersection_id1){
                return true;
            }
            else{
                return false;
            }
        }
        else{
            return true;
        }
    }
    else{
        return false;
    }
}

//Returns all intersections reachable by traveling down one street segment 
//from given intersection (hint: you can't travel the wrong way on a 1-way street)
//the returned vector should NOT contain duplicate intersections
std::vector<unsigned> find_adjacent_intersections(unsigned intersection_id){
    vector<unsigned>streetSegments = find_intersection_street_segments(intersection_id);
    vector<unsigned> adjacentIntersections;
    int count = 0;
    //int numberOfSegments = streetSegments.size();
    for(auto it = streetSegments.begin() ; it != streetSegments.end() ; it++){
        InfoStreetSegment segmentInfo = getInfoStreetSegment(streetSegments[count]);
        if(segmentInfo.oneWay){
            if(segmentInfo.from == intersection_id){
                if(find(adjacentIntersections.begin(),adjacentIntersections.end(),segmentInfo.to) == adjacentIntersections.end()){
                    adjacentIntersections.push_back(segmentInfo.to);
                }   
            }
        }
        else{
            if(intersection_id == segmentInfo.from){
                if(find(adjacentIntersections.begin(),adjacentIntersections.end(),segmentInfo.to) == adjacentIntersections.end()){
                    adjacentIntersections.push_back(segmentInfo.to);
                }
            }
            else{
                if(find(adjacentIntersections.begin(),adjacentIntersections.end(),segmentInfo.from) == adjacentIntersections.end()){
                    adjacentIntersections.push_back(segmentInfo.from);
                }
            }
        }
    count++;
    }
    return adjacentIntersections;
}

//Returns all street segments for the given street
std::vector<unsigned> find_street_street_segments(unsigned street_id){
         return streetAndSegments[street_id];
}

//Returns all intersections along the a given street
std::vector<unsigned> find_all_street_intersections(unsigned street_id){
    return streetAndIntersections[street_id];
}

//Return all intersection ids for two intersecting streets
//This function will typically return one intersection id.
std::vector<unsigned> find_intersection_ids_from_street_ids(unsigned street_id1, unsigned street_id2)
{
    vector <unsigned> results = {};
    vector <unsigned> intersections1 = find_all_street_intersections(street_id1);
    vector <unsigned> intersections2 = find_all_street_intersections(street_id2);
    
    for(auto it = intersections2.begin();it!=intersections2.end();it++){
        if(find(intersections1.begin(),intersections1.end(),*it) != intersections1.end())
            results.push_back(*it);
    }
    return results;
}

//Returns the distance between two coordinates in meters
double find_distance_between_two_points(LatLon point1, LatLon point2)
{
    double x1 = DEG_TO_RAD * point1.lon() * cos ((DEG_TO_RAD * point1.lat() + DEG_TO_RAD * point2.lat())*0.5);
    double y1 = DEG_TO_RAD * point1.lat();
    double x2 = DEG_TO_RAD * point2.lon() * cos ((DEG_TO_RAD * point1.lat() + DEG_TO_RAD * point2.lat())*0.5);
    double y2 = DEG_TO_RAD * point2.lat();
    double Y = pow ( (y1 - y2) , 2 );
    double X = pow ( (x1 - x2) , 2);
    double result = EARTH_RADIUS_IN_METERS * sqrt (X + Y);
    return result;
 
    return 0;
}

//Returns the nearest intersection to the given position
unsigned find_closest_intersection(LatLon my_position) {
  unsigned long long intersection_count=getNumIntersections();
    unsigned long long iterator; 
    
    unsigned long long id;
   LatLon intersection_point;
    LatLon first_intersection_point;
    double update;
    
    first_intersection_point=getIntersectionPosition(0);
    double leastdistance=find_distance_between_two_points(my_position,first_intersection_point);
    for(iterator=1; iterator<intersection_count; iterator++){
        intersection_point=getIntersectionPosition(iterator);   // getting intersection position
        update=find_distance_between_two_points(my_position,intersection_point); // getting the distance 
        if(update<leastdistance){
            leastdistance=update;
            id=iterator;             // compare and update the distance
        }
    } 
   
  return id;   
}

//Returns all street ids corresponding to street names that start with the given prefix
//The function should be case-insensitive to the street prefix. For example, 
//both "bloo" and "BloO" are prefixes to "Bloor Street East".
//If no street names match the given prefix, this routine returns an empty (length 0) 
//vector.
//You can choose what to return if the street prefix passed in is an empty (length 0) 
//string, but your program must not crash if street_prefix is a length 0 string.
std::vector<unsigned> find_street_ids_from_partial_street_name(std::string street_prefix){
   if(street_prefix.length() == 0 )
        return vector <unsigned> {};
    vector<unsigned> ids = {};
    bool status = true;
    transform(street_prefix.begin(), street_prefix.end(), street_prefix.begin(), ::tolower);
    auto it = streetNames.lower_bound(street_prefix);
        for(it; status ; it++){
            if(it == streetNames.end())
                break;
            string s = it->first.substr( 0, street_prefix.length());
            if( street_prefix == s )
                ids.push_back( it -> second);
            else
                status = false;
        }
    return ids;
} 
