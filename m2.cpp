#include "m2.h"
#include "m1.h"
#include "m2extra.h"
#include "m3.h"
#include "m3extra.h"

using namespace std;
using namespace ezgl;

struct segment{
    bool highlight;
};

vector <intersection> intersections_list;
vector <feature> features_list;
vector <POI> POI_list;
vector <double> segments_angle;
vector <midpoint> mid_point;
vector <string> same_name;
vector <segment> segments_highlight;
int prevId = -1; //Stores the previous marked id
int segment_count = 0;

string featureType;
bool reload = false;
string map_input_path = "";
bool path_highlighted = false;
double max_lat = 0;
double max_lon = 0;
double min_lat = 0;
double min_lon = 0;
gdouble zoom_x = 0;
gdouble zoom_y = 0;
double zoom_global = 0;
vector <unsigned> path_found; // this is where we load the found path 
bool nightMode = false;
vector <unsigned> pathFound = {};
bool path_highlight = false;
bool first_id_bool = false;
int second_id;
int first_id;
bool highlight = false;

surface* marker = renderer::load_png("./libstreetmap/resources/map-pin.png");
surface* markerZoom = renderer::load_png("./libstreetmap/resources/pin.png");

void path_taking(string map_path){
        string path;
        cout<<"enter the path of the map to be loaded in our system"<<endl;   // taking input from console
        cin>>path;
        map_path=path;
}

bool function_same_name(string street_name){
    for(int i = 0 ; i < same_name.size() ; i++){
        if(street_name == same_name[i]){
            return true;
        }
    }
    return false;
}


//function to initialize segments
void initialize_segments(){
    int segment_count = getNumStreetSegments();
    path_found.resize(segment_count);
    for(int i = 0 ; i < segment_count ; i++){
        path_found[i]=i;
    }
}

//function to initialize global variable intersections_list
void initialize_Intersections(){
    intersections_list.resize(getNumIntersections());
    max_lat = getIntersectionPosition(0).lat();
    max_lon = getIntersectionPosition(0).lon();
    min_lat = max_lat;
    min_lon = max_lon;
    for (unsigned i=0;i<intersections_list.size();++i){
        intersections_list[i].position = getIntersectionPosition(i);
        intersections_list[i].name = getIntersectionName(i);
        intersections_list[i].highlight = false; //initially none of the intersections are highlighted
        //Updates the maximum and minimum longitude,latitude values by using max,min functions
        max_lat = std::max(max_lat,getIntersectionPosition(i).lat());
        min_lat = std::min(min_lat,getIntersectionPosition(i).lat());
        max_lon = std::max(max_lon,getIntersectionPosition(i).lon());
        min_lon = std::min(min_lon,getIntersectionPosition(i).lon());
    }
}
//Convert longitude to cartesian x co-ordinate
double x_from_lon(double point){
    return point * cos ((DEG_TO_RAD *max_lat + DEG_TO_RAD * min_lat)*0.5);
}

double lon_from_x(double point){
    return point / cos ((DEG_TO_RAD *max_lat + DEG_TO_RAD * min_lat)*0.5);
}
 // function to find if given point is within the visible world
bool within_visible_world(ezgl::renderer &g,double x ,double y){
    point2d p1(0,0);
    point2d p2(0,0);
    rectangle r1(p1,p2);
    r1 = g.get_visible_world();
    return r1.contains(x,y);
}
/*
    function to calculate the tilt of a street segment
    tan theta = opposite/adjacent
    opposite = difference between the y co-ordinates of the two points
    adjacent = difference between the x co-ordinates of the two points
    theta = arctan ( opposite/adjacent)
 */

double tiltCalculator(LatLon point1, LatLon point2){
    double x1 = x_from_lon(point1.lon());
    double y1 = point1.lat();
    double x2 = x_from_lon(point2.lon());
    double y2 = point2.lat();
    if(!isnan((y2-y1)/(x2-x1))){
    double angle = atan2((y2-y1),(x2-x1));
    return angle/DEG_TO_RAD;
    }
    return 0;
}
// calculating the segment angle
void initialize_segments_angle(){
    segments_angle.resize(getNumStreetSegments());
    double angle;
    LatLon from,to;
    for(int i = 0 ; i < segments_angle.size() ; i++){
        InfoStreetSegment segment_info = getInfoStreetSegment(i);
        from = getIntersectionPosition(segment_info.from);
        to = getIntersectionPosition(segment_info.to);
        angle = tiltCalculator(from,to);
        segments_angle[i] = angle;
    }
}

void initialize_mid_point(){
    mid_point.resize(getNumStreetSegments());
    unsigned long first_intersection_id, second_intersection_id;
    unsigned number_of_curvepoints;
    double x,y;
    double lenghtmax;
    double lenght;
    LatLon p1,p2;
    // calculating the mid point of street segment
    for(int segment_id = 0; segment_id < mid_point.size();segment_id++){
        number_of_curvepoints = getInfoStreetSegment(segment_id).curvePointCount;
        first_intersection_id = getInfoStreetSegment(segment_id).from;
        second_intersection_id = getInfoStreetSegment(segment_id).to;
        if(number_of_curvepoints == 0){
            p1 = getIntersectionPosition(first_intersection_id);
            p2 = getIntersectionPosition(second_intersection_id);
            x = (x_from_lon(p1.lon()) + x_from_lon(p2.lon()))/2;
            y = (p1.lat() + p2.lat())/2;
            mid_point[segment_id].x = x;
            mid_point[segment_id].y = y;
        }
        else{
            p1 = getIntersectionPosition(first_intersection_id);
            p2 = getStreetSegmentCurvePoint(0,segment_id);
            x = (x_from_lon(p1.lon()) + x_from_lon(p2.lon()))/2;
            y = (p1.lat() + p2.lat())/2;
            lenghtmax = find_distance_between_two_points(p1,p2);
            for(int i = 1; i < number_of_curvepoints;i++){
                p1 = getStreetSegmentCurvePoint(i-1,segment_id);
                p2 = getStreetSegmentCurvePoint(i,segment_id);
                lenght = find_distance_between_two_points(p1,p2);
                // comparing the length
                if(lenght > lenghtmax){
                    x = (x_from_lon(p1.lon()) + x_from_lon(p2.lon()))/2;
                    y = (p1.lat() + p2.lat())/2;
                }
            }
            mid_point[segment_id].x = x;
            mid_point[segment_id].y = y;
        }
    }
}

//  The following function returns the zoom  
// maximum zoom = 6 , minimum zoom = 0;
// tells zoom level of display
int tell_zoom_level(ezgl::renderer &g){
    point2d p1(0,0);
    point2d p2(0,0);
    rectangle r1(p1,p2);
    r1 = g.get_visible_world();
    double area =  r1.area();
    if(area >= 0.26){
        return 0;
    }
    else if(area < 0.26 && area > 0.09){
        return 1;
    }
    else if(area < 0.09 && area > 0.03){
        return 2;
    }
    else if(area < 0.03 && area > 0.01){
        return 3;
    }
    else if(area < 0.01 && area > 0.004){
        return 4;
    }
    else if(area < 0.004 && area > 0.001){
        return 5;
    }
    else if(area < 0.001 && area > 0.0005
 ){
        return 6;
    }
    else if(area < 0.0005 && area > 0.0002 ){
        return 7;
    }
    else if(area < 7.45212e-05 && area > 2.68276e-05 ){
        return 9;
    }
    else if(area < 2.68276e-05 && area > 9.65795e-06 ){
        return 10;
    }
    else if(area < 9.65795e-06  && area > 3.47686e-06 ){
        return 11;
    }
    else if(area < 3.47686e-06  && area > 1.25167e-06){
        return 12;
    }
    else if(area < 1.25167e-06 )
        return 13;
    else
        return 8;
}

/*  Color Scheme 
    credits : https://www.schemecolor.com/google-map-basic-colors.php
*/
void colorCode(feature &temp){
    
    //Park : (27,162,97)
    if(temp.type == Park){
         temp.featureColor.red = 165;
        temp.featureColor.green = 215;
        temp.featureColor.blue =  33;
    }
    
    //Beach: (230,177,117))
    if(temp.type == Beach){
         temp.featureColor.red = 250;
        temp.featureColor.green = 240;
        temp.featureColor.blue =  107;
    }
    
    //Lake: (86,131,255)
    if(temp.type == Lake){
        temp.featureColor.red = 9;
        temp.featureColor.green = 112;
        temp.featureColor.blue =  210;
    }
    extern vector <unsigned> pathFound;
    //River: (38,194,240)
    if(temp.type == River){
        temp.featureColor.red = 38;
        temp.featureColor.green = 194;
        temp.featureColor.blue =  240;
    }
    
    //Island: (140,95,61)
    if(temp.type == Island){
         temp.featureColor.red = 198;
        temp.featureColor.green = 134;
        temp.featureColor.blue =  66;
    }
    
    //Building: (255,29,71)
    if(temp.type == Building){
         temp.featureColor.red = 130;
        temp.featureColor.green = 130;
        temp.featureColor.blue =  132;
    }
    
    //Greenspace: (195,236,178)
    if(temp.type == Greenspace){
         temp.featureColor.red = 214;
        temp.featureColor.green = 250;
        temp.featureColor.blue =  140;
    }
    
    //Golfcourse: (255,208,75)
    if(temp.type == Golfcourse){
         temp.featureColor.red = 93;
        temp.featureColor.green = 135;
        temp.featureColor.blue =  0;
    }
    
    //Stream: (170,218,255)
    if(temp.type == Stream){
         temp.featureColor.red = 14;
        temp.featureColor.green = 142;
        temp.featureColor.blue =  233;
    }
}

void colorCodeNight(feature &temp){
    
    //Park : (27,162,97)
    if(temp.type == Park){
         temp.featureColor.red = 165;
        temp.featureColor.green = 215;
        temp.featureColor.blue =  33;
    }
    
    //Beach: (230,177,117))
    if(temp.type == Beach){
         temp.featureColor.red = 250;
        temp.featureColor.green = 240;
        temp.featureColor.blue =  107;
    }
    
    //Lake: (86,131,255)
    if(temp.type == Lake){
        temp.featureColor.red = 9;
        temp.featureColor.green = 112;
        temp.featureColor.blue =  210;
    }
    
    //River: (38,194,240)
    if(temp.type == River){
        temp.featureColor.red = 38;
        temp.featureColor.green = 194;
        temp.featureColor.blue =  240;
    }
    
    //Island: (140,95,61)
    if(temp.type == Island){
         temp.featureColor.red = 198;
        temp.featureColor.green = 134;
        temp.featureColor.blue =  66;
    }
    
    //Building: (255,29,71)
    if(temp.type == Building){
        temp.featureColor = RED;
    }
    
    //Greenspace: (195,236,178)
    if(temp.type == Greenspace){
         temp.featureColor.red = 214;
        temp.featureColor.green = 250;
        temp.featureColor.blue =  140;
    }
    
    //Golfcourse: (255,208,75)
    if(temp.type == Golfcourse){
         temp.featureColor.red = 93;
        temp.featureColor.green = 135;
        temp.featureColor.blue =  0;
    }
    
    //Stream: (170,218,255)
    if(temp.type == Stream){
         temp.featureColor.red = 14;
        temp.featureColor.green = 142;
        temp.featureColor.blue =  233;
    }
}

//function to initialize global variable features_list
void initialize_Features(){
    features_list.resize(getNumFeatures());
    
    for(int i =0; i<getNumFeatures();++i){
        features_list[i].name = getFeatureName(i); 
        features_list[i].type = getFeatureType(i);
        features_list[i].numberOfPoints = getFeaturePointCount(i);
       
        //checking if the feature is a closed body,open body or just a point.
        if(getFeaturePointCount(i) <= 1){
            features_list[i].bodyType = "point";
        }
        else{
            //if start point and end point are the same, it is a closed body
            if( getFeaturePoint(0,i).lat() == getFeaturePoint(getFeaturePointCount(i)-1,i).lat()
                && getFeaturePoint(0,i).lat() == getFeaturePoint(getFeaturePointCount(i)-1,i).lat())
                features_list[i].bodyType = "closed";
            else
                features_list[i].bodyType = "open";
        }
        
        point2d point(0,0);
        
        for(int j = 0; j < getFeaturePointCount(i); j++){
            point.x= x_from_lon(getFeaturePoint(j,i).lon() );
            point.y= getFeaturePoint(j,i).lat();
            features_list[i].featurePoints.push_back(point);
        }
    }
}

// assigning the POI and classifying into diffrent types 
void assignPOI_Type(POI& a){
    if(a.type == "childcare" || a.type == "clinic" || a.type == "dentist" || a.type == "doctors" || a.type == "hospital" || a.type == "pharmacy" || a.type == "dentist" || a.type == "veterinary")
        a.type = "Medical";
    else if(a.type == "atm" || a.type == "bank" || a.type == "bureau_de_change")
        a.type = "Bank";
    else if(a.type == "cafe" || a.type == "fast_food" || a.type == "food_court" || a.type == "restaurant" || a.type == "vending_machine" )
        a.type = "Restaurant";
    else if(a.type == "bar" || a.type == "cinema" || a.type == "pub" || a.type == "spa" || a.type == "stripclub" || a.type == "theatre")
        a.type = "Entertainment";
    else if(a.type == "college" || a.type == "kindergarten" || a.type == "tutoring")
        a.type = "Education";
    else 
        a.type = "Others";
    
}

// initializing the different POI
void initialize_POI(){
    POI_list.resize(getNumPointsOfInterest());
    for (int i =0 ; i<getNumPointsOfInterest(); ++i){
        POI_list[i].name = getPointOfInterestName(i);
        POI_list[i].type = getPointOfInterestType(i);
        assignPOI_Type(POI_list[i]);
        POI_list[i].position = getPointOfInterestPosition(i);
        POI_list[i].highlight = false;   //initally all the POIs are not highlighted
    }
}

//  plotting the 
void plot_intersections(renderer &g){
    unsigned zoom = tell_zoom_level(g);
    for(unsigned i=0;i<intersections_list.size();++i){
        double x = x_from_lon (intersections_list[i].position.lon() );
        double y = intersections_list[i].position.lat();
        if(intersections_list[i].highlight && zoom <=8){
            //credits : https://www.flaticon.com/authors/roundicons-freebies
            surface* pointer = renderer::load_png("./libstreetmap/resources/pointer.png");
            g.draw_surface(pointer,{x,y});
            g.free_surface(pointer); 
            pointer = NULL;
        }else if(intersections_list[i].highlight){
            g.set_color(RED);
        }
        else{ 
            g.set_color(GREY_75);
        }
        if(zoom >= 7 && zoom <10){
            g.fill_elliptic_arc({x,y},0.00005,0.00005,0,360);
        }else if(zoom >= 10 && zoom<11 ){
            g.fill_elliptic_arc({x,y},0.00003,0.00003,0,360);
        }else if(zoom >=11){
            g.fill_elliptic_arc({x,y},0.00002,0.00002,0,360);
        }
    }
} 

void draw_features(renderer &g){
    for(unsigned i =0;i<features_list.size();++i){
         if(nightMode)
            colorCodeNight(features_list[i]);
        else
            colorCode(features_list[i]);
        if(features_list[i].bodyType == "closed"){
            g.set_color(features_list[i].featureColor);
            g.fill_poly(features_list[i].featurePoints);
        }
        else{
            if(features_list[i].bodyType == "open"){
               for(unsigned j=0;j<features_list[i].featurePoints.size()-1; ++j){
                    g.set_color(features_list[i].featureColor);
                    g.draw_line(features_list[i].featurePoints[j],features_list[i].featurePoints[j+1]);  
               } 
            }
            else{
                g.set_color(features_list[i].featureColor);        
                g.fill_elliptic_arc(features_list[i].featurePoints[0],0.0001,0.0001,0,360);
            }
        }
    } 
}

void zoom_to_point(GtkWidget *widget, ezgl::application *application){
    cout<<zoom_global<<endl;
    if(zoom_x!=0 && zoom_y!=0 && zoom_global <3){
        application->change_button_text("Can not zoom","Zoom To Point");
        std::string main_canvas_id = application->get_main_canvas_id();
        auto canvas = application->get_canvas(main_canvas_id);
        ezgl::zoom_in(canvas,{zoom_x,zoom_y},300.0 / 3.0);
    }else if(zoom_x!=0 && zoom_y!=0 && zoom_global >=3 && zoom_global <=5){
        application->change_button_text("Can not zoom","Zoom To Point");
        std::string main_canvas_id = application->get_main_canvas_id();
        auto canvas = application->get_canvas(main_canvas_id);
        ezgl::zoom_in(canvas,{zoom_x,zoom_y},92.0 / 3.0);
    }else if(zoom_x!=0 && zoom_y!=0 && zoom_global >5 && zoom_global <8){
        application->change_button_text("Can not zoom","Zoom To Point");
        std::string main_canvas_id = application->get_main_canvas_id();
        auto canvas = application->get_canvas(main_canvas_id);
        ezgl::zoom_in(canvas,{zoom_x,zoom_y},42.0 / 3.0);
    }else if(zoom_x!=0 && zoom_y!=0 && zoom_global == 8){
        application->change_button_text("Can not zoom","Zoom To Point");
        std::string main_canvas_id = application->get_main_canvas_id();
        auto canvas = application->get_canvas(main_canvas_id);
        ezgl::zoom_in(canvas,{zoom_x,zoom_y},18.0 / 3.0);
    }
    else{
        application->update_message("Sorry can not zoom ");
    }
    zoom_x = 0;
    zoom_y =0;
}

void act_on_mouse_click(ezgl::application* app,GdkEventButton* event,double x, double y){
  LatLon click = LatLon( y, lon_from_x(x));
  LatLon nearest = getPointOfInterestPosition(find_closest_point_of_interest(click));
  int id = find_closest_intersection(click);
  if(!first_id_bool){
      first_id = id;
      first_id_bool = true;
  }
  else if(first_id_bool){
      second_id = id;
      path_found = find_path_between_intersections(first_id,second_id,0,0);
      first_id_bool = false;
      highlight = true;   
  }
  LatLon pos = getIntersectionPosition(id);
  zoom_x = event->x;
  zoom_y = event->y;
  if(zoom_global < 9 && POI_list[find_closest_point_of_interest(click)].type == featureType){
    app->update_message("Lat: " + to_string(pos.lat()) + "    Lon: " + to_string(pos.lon()) + "   Closest Intersection: " + intersections_list[id].name);
    cout<<"Lat: " << to_string(pos.lat()) << "    Lon: " << to_string(pos.lon()) << "   Closest Intersection: " << intersections_list[id].name<<endl;
  }
  else if (zoom_global >=9 && abs(pos.lat() - nearest.lat() ) < 0.0005 &&  abs(pos.lon() - nearest.lon() ) <0.0005 ){
      int id = find_closest_point_of_interest(pos);
      app->update_message("Lat: " + to_string(nearest.lat()) + "    Lon: " + to_string(nearest.lon()) + "   Type: " +  getPointOfInterestType(id) + "   Name: "+ getPointOfInterestName(id)
      + "   Closest Intersection: " + intersections_list[id].name);
      cout<< "Lat: " << to_string(nearest.lat()) << "    Lon: " << to_string(nearest.lon()) << "   Type: " <<  getPointOfInterestType(id) << "   Name: " << getPointOfInterestName(id)
      << "   Closest Intersection: " << intersections_list[id].name <<endl;
  }else{
       app->update_message("Lat: " + to_string(pos.lat()) + "    Lon: " + to_string(pos.lon()) + "   Closest Intersection: " + intersections_list[id].name);
       cout<<"Lat: " << to_string(pos.lat()) << "    Lon: " << to_string(pos.lon()) << "   Closest Intersection: " << intersections_list[id].name <<endl;
  }
  if(prevId != id){
    intersections_list[id].highlight = true;
     intersections_list[prevId].highlight = false;
  }
  prevId = id; //storing the intersection id to unmark
  app->refresh_drawing();
}

int find(unsigned id1, unsigned id2){
    vector <unsigned> result = find_intersection_ids_from_street_ids(id1,id2);
    path_found = find_path_between_intersections(result[0],result.size() - 1,15.000,25.0000);
    give_directions(path_found);
    if(result.size()!=0){
        cout<<"The intersections found are: "<<endl;
        for(auto it = result.begin();it!=result.end();++it){
            intersections_list[*it].highlight = true;
            cout<<"Name " << intersections_list[*it].name <<" Position:"<<intersections_list[*it].position<<endl;
        }
    }
    return result.size();
}

void find_button(GtkWidget *widget, ezgl::application *application){
    GtkEntry* text_entry1 = (GtkEntry *) application->get_object("Find1");
    string street1 = gtk_entry_get_text(text_entry1);
    GtkEntry* text_entry2 = (GtkEntry *) application->get_object("Find2");
    string street2 = gtk_entry_get_text(text_entry2);
    path_highlight = true;
    unsigned intersect_id_start;
    unsigned itersect_id_end;
    if(street1.size() == 0 || street2.size() == 0){
        application->update_message("No Common Intersections Found");
        return;
    }
    vector <unsigned> results1 = find_street_ids_from_partial_street_name(street1);
    vector <unsigned> results2 = find_street_ids_from_partial_street_name(street2);
    if( results1.size()!=0 && results2.size()!=0){
        int count = find(results1[0],results2[0]);
        if(count!=0){
            cout<<count<<" intersections found"<<endl;
            application->update_message("Common Intersections found");
            application->refresh_drawing();
        }
    }else{
        application->update_message("No Common Intersections Found");
    }
}

void clear_button(GtkWidget *widget, ezgl::application *application){
    for(auto it = intersections_list.begin();it!=intersections_list.end();++it){
        it->highlight = false;
    }
    zoom_x =0;
    zoom_y =0;
    application->update_message("Marker Cleared");
    application->refresh_drawing();
    path_highlight = false;
}

void night_button(GtkWidget *widget, ezgl::application *application){
    if(nightMode){
        nightMode = false;
        application->change_button_text("Day Mode","Night Mode");
    }
    else{
        nightMode = true;
        application->change_button_text("Night Mode","Day Mode");
    }
    application->refresh_drawing();
}

void filter_button (GtkWidget *widget, ezgl::application *application) {
    GtkComboBoxText* feature = (GtkComboBoxText*) application->get_object("FeatureList");
    featureType = gtk_combo_box_text_get_active_text (feature);
    application->refresh_drawing(); 
}

 void map_load_button (GtkWidget *widget, ezgl::application *application){
//    GtkEntry* text_entry1 = (GtkEntry *) application->get_object("MapName");
//    const char* street1 = gtk_entry_get_text(text_entry1);
//    map_input_path = street1;
//    map_input_path = street1;
//    map_input_path = "/cad2/ece297s/public/maps/"+map_input_path+".streets.bin";
//    intersections_list.clear();
//    features_list.clear();
//    POI_list.clear();
//    segments_angle.clear();
//    mid_point.clear();
//    same_name.clear();
//    prevId = -1; //Stores the previous marked id
//    segment_count = 0;
//    featureType = "";
//    max_lat = 0;
//    max_lon = 0;
//    min_lat = 0;
//    min_lon = 0;
//    zoom_x = 0;
//    zoom_y = 0;
//    zoom_global = 0;
//    nightMode = false;
//    reload = true;
//    close_map();
//    application->quit();
}
 
void act_on_key_press(ezgl::application *application, GdkEventKey *event, char *key_name){
   GtkEntry* text_entry1 = (GtkEntry *) application->get_object("Find1");
    const char* street1 = gtk_entry_get_text(text_entry1);
    GtkEntry* text_entry2 = (GtkEntry *) application->get_object("Find2");
    const char* street2 = gtk_entry_get_text(text_entry2);
    vector <unsigned> results1 = find_street_ids_from_partial_street_name(street1);
    vector <unsigned> results2 = find_street_ids_from_partial_street_name(street2);
    if(gtk_widget_is_focus( (GtkWidget*)text_entry1 ) ){
        for(auto it = results1.begin();it!=results1.end();it++){
            cout<<getStreetName(*it)<<endl;
        }
    }
    if(gtk_widget_is_focus( (GtkWidget*)text_entry2 ) ){
        for(auto it = results2.begin();it!=results2.end();it++){
            cout<<getStreetName(*it)<<endl;
        }
    }
    
    std::string main_canvas_id = application->get_main_canvas_id();
    canvas* canvas = application->get_canvas(main_canvas_id);

    //keys to move in map
       if(strcmp(key_name,"Up") == 0){
           translate_up(canvas,3);
       }
    
       if(strcmp(key_name,"Down") == 0){
           translate_down(canvas,3);
       }
       
       if(strcmp(key_name,"Left") == 0){
           translate_left(canvas,3);
       }
    
      if(strcmp(key_name,"Right") == 0){
           translate_right(canvas,3);
       }
    
}

void initial_setup(ezgl::application *application)
{

  // Create a Test button and link it with test_button callback from.
   application->create_button("Filter", 8, filter_button);
  application->create_button("Find", 9, find_button);
  application->create_button("Clear Marker", 10, clear_button);
  application->create_button("Zoom To Point", 11, zoom_to_point);
  application->create_button("Night Mode", 12, night_button);
  application->create_button("Load Map", 13, map_load_button);
  application->refresh_drawing();
}

void draw_POI(ezgl::renderer &g){
    unsigned zoom=tell_zoom_level(g);
    string poiName;
     g.set_color(MAGENTA);
    if(zoom >4 && zoom <8){
       for(int i =0; i< POI_list.size(); ++i){
           if(POI_list[i].type == featureType || featureType == "All" ){
            double x = x_from_lon ( POI_list[i].position.lon() );
            double y = POI_list[i].position.lat();        
            g.draw_surface(markerZoom,{x,y});
            poiName = getPointOfInterestName(i);
           }
       }
    } else if(zoom >=8){
        for(int i =0; i< POI_list.size(); ++i){
            if(POI_list[i].type == featureType || featureType == "All" ){
                double x = x_from_lon ( POI_list[i].position.lon() );
                double y = POI_list[i].position.lat();        
                g.draw_surface(marker,{x+.00006,y+.00006});
                poiName = getPointOfInterestName(i);
                point2d p1(x,y);
                g.set_color(BLUE);
                if(zoom >= 10){
                    g.set_text_rotation(0);
                    g.draw_text(p1,poiName);
                    g.set_font_size(15);
                }
            }

        }
   }
}



void draw_street(ezgl::renderer &g){
          
          unsigned zoom=tell_zoom_level(g);
          unsigned long first_intersection_id;
          unsigned long second_intersection_id;
          unsigned curvepoint_count;
          unsigned number = getNumStreetSegments();
          unsigned streetID;
          string streetName;
          double x,y;
          for (unsigned street_segment_id = 0; street_segment_id < number; street_segment_id++) {
            unsigned speedLimit = getInfoStreetSegment(street_segment_id).speedLimit;  
            if(zoom >= 6){
             g.set_line_width(4);
             if(nightMode){
                 g.set_color(58,66,79);
             }
             else{
                 g.set_color(211,211,211); 
             }
             
             curvepoint_count = getInfoStreetSegment(street_segment_id).curvePointCount;
             first_intersection_id = getInfoStreetSegment(street_segment_id).from;
             second_intersection_id = getInfoStreetSegment(street_segment_id).to;
             double xistart;
             double ystart;
             double  xend ;
             double  yend ;
             double xstartlongi, xfinishlongi;
             //find the position of the two intersections
             xstartlongi = getIntersectionPosition(first_intersection_id).lon();
             xistart=x_from_lon (xstartlongi);
             ystart = getIntersectionPosition(first_intersection_id).lat();
             xfinishlongi = getIntersectionPosition(second_intersection_id).lon();
             xend = x_from_lon (xfinishlongi);
             yend = getIntersectionPosition(second_intersection_id).lat();
        if(curvepoint_count == 0){                                          
          g.draw_line({xistart, ystart},{ xend ,  yend });
        }
        else{
       
          LatLon firstcurvepoint = getStreetSegmentCurvePoint(0,street_segment_id);
       
       
          g.draw_line({xistart, ystart},{x_from_lon(firstcurvepoint.lon()),firstcurvepoint.lat()});
         for (int i = 1; i < curvepoint_count; i++) {
            segment_count++;
            LatLon curvepoint1 = getStreetSegmentCurvePoint(i - 1 ,street_segment_id);
            LatLon curvepoint2 = getStreetSegmentCurvePoint(i,street_segment_id );
            g.draw_line({x_from_lon(curvepoint1.lon()),curvepoint1.lat()},{x_from_lon(curvepoint2.lon()),curvepoint2.lat()});
        }
     
          LatLon lastcurvepoint = getStreetSegmentCurvePoint( curvepoint_count - 1,street_segment_id);
       
          g.draw_line({x_from_lon(lastcurvepoint.lon()),lastcurvepoint.lat()},{ xend ,  yend });
          
         
        }  
    }
            else if(zoom <=5 && zoom >3){                      // depending on zoom the streets appear
        g.set_line_width(2);
           if(nightMode){
                 g.set_color(58,66,79);
             }
           else{
                 g.set_color(211,211,211); 
             }
        if(speedLimit > 50 && speedLimit <80){
            curvepoint_count =getInfoStreetSegment(street_segment_id).curvePointCount;
            first_intersection_id = getInfoStreetSegment(street_segment_id).from;
            second_intersection_id = getInfoStreetSegment(street_segment_id).to;
            double xistart, ystart,  xend ,  yend ;
            double xstartlongi, xfinishlongi;
            //find the position of the two intersections
            xstartlongi = getIntersectionPosition(first_intersection_id).lon();
            xistart=x_from_lon (xstartlongi);
            ystart = getIntersectionPosition(first_intersection_id).lat();
            xfinishlongi = getIntersectionPosition(second_intersection_id).lon();
            xend =x_from_lon (xfinishlongi);
            yend  = getIntersectionPosition(second_intersection_id).lat();
      if (curvepoint_count == 0) {
       
 
          g.draw_line({xistart, ystart},{ xend ,  yend });
        }else{

          LatLon firstcurvepoint = getStreetSegmentCurvePoint(0,street_segment_id);
       
       
          g.draw_line({xistart, ystart},{x_from_lon(firstcurvepoint.lon()),firstcurvepoint.lat()});
          for (unsigned i = 1; i < curvepoint_count; i++) {
         
          LatLon curvepoint1 = getStreetSegmentCurvePoint(i - 1 ,street_segment_id);
          LatLon curvepoint2 = getStreetSegmentCurvePoint(i,street_segment_id );
     
   
          g.draw_line({x_from_lon(curvepoint1.lon()),curvepoint1.lat()},{x_from_lon(curvepoint2.lon()),curvepoint2.lat()});  
          }
         
          LatLon lastcurvepoint = getStreetSegmentCurvePoint( curvepoint_count - 1,street_segment_id);
       
         
          g.draw_line({x_from_lon(lastcurvepoint.lon()),lastcurvepoint.lat()},{ xend ,  yend });  
        }
      }
    }
        // Always draws major street SEGMENTS
        g.set_line_width(5);
        g.set_color(255, 224, 71); 
        curvepoint_count = getInfoStreetSegment(street_segment_id).curvePointCount;
        first_intersection_id = getInfoStreetSegment(street_segment_id).from;
        second_intersection_id = getInfoStreetSegment(street_segment_id).to;
        if( speedLimit >= 80){
             curvepoint_count =getInfoStreetSegment(street_segment_id).curvePointCount;
             first_intersection_id = getInfoStreetSegment(street_segment_id).from;
             second_intersection_id = getInfoStreetSegment(street_segment_id).to;
             double xistart, ystart,  xend , yfinish;
             double xstartlongi, xfinishlongi;
             //find the position of the two intersections
             xstartlongi = getIntersectionPosition(first_intersection_id).lon();
             xistart=x_from_lon (xstartlongi);
             ystart = getIntersectionPosition(first_intersection_id).lat();
             xfinishlongi = getIntersectionPosition(second_intersection_id).lon();
             xend =x_from_lon (xfinishlongi);
             yfinish = getIntersectionPosition(second_intersection_id).lat();
      if (curvepoint_count == 0) {
       
 
          g.draw_line({xistart, ystart},{ xend , yfinish});
        }else{

          LatLon firstcurvepoint = getStreetSegmentCurvePoint(0,street_segment_id);
       
       
          g.draw_line({xistart, ystart},{x_from_lon(firstcurvepoint.lon()),firstcurvepoint.lat()});
          for (unsigned i = 1; i < curvepoint_count; i++) {
         
          LatLon curvepoint1 = getStreetSegmentCurvePoint(i - 1 ,street_segment_id);
          LatLon curvepoint2 = getStreetSegmentCurvePoint(i,street_segment_id );
     
   
          g.draw_line({x_from_lon(curvepoint1.lon()),curvepoint1.lat()},{x_from_lon(curvepoint2.lon()),curvepoint2.lat()});  
          }
         
          LatLon lastcurvepoint = getStreetSegmentCurvePoint( curvepoint_count - 1,street_segment_id);
       
         
          g.draw_line({x_from_lon(lastcurvepoint.lon()),lastcurvepoint.lat()},{ xend , yfinish});  
        }
      }
    }    
    } 
//          if(path_find){
//              g.set_color(RED);
//              for(int street_segment_id=0; i<path.size();i++){
//                  g.set_line_width(4);
//                  curvepoint_count = getInfoStreetSegment(street_segment_id).curvePointCount;
//                  first_intersection_id = getInfoStreetSegment(street_segment_id).from;
//                  second_intersection_id = getInfoStreetSegment(street_segment_id).to;
//                  double xistart;
//                  double ystart;
//                  double  xend ;
//                  double  yend ;
//                  double xstartlongi, xfinishlongi;
//                  //find the position of the two intersections
//                  xstartlongi = getIntersectionPosition(first_intersection_id).lon();
//                  xistart=x_from_lon (xstartlongi);
//                  ystart = getIntersectionPosition(first_intersection_id).lat();
//                  xfinishlongi = getIntersectionPosition(second_intersection_id).lon();
//                  xend = x_from_lon (xfinishlongi);
//                  yend = getIntersectionPosition(second_intersection_id).lat();
//                 if(curvepoint_count == 0){                                          
//                     g.draw_line({xistart, ystart},{ xend ,  yend });
//                }
//                else{
//                    LatLon firstcurvepoint = getStreetSegmentCurvePoint(0,street_segment_id);
//                    g.draw_line({xistart, ystart},{x_from_lon(firstcurvepoint.lon()),firstcurvepoint.lat()});
//                    for (int i = 1; i < curvepoint_count; i++) {
//                        segment_count++;
//                        LatLon curvepoint1 = getStreetSegmentCurvePoint(i - 1 ,street_segment_id);
//                        LatLon curvepoint2 = getStreetSegmentCurvePoint(i,street_segment_id );
//                        g.draw_line({x_from_lon(curvepoint1.lon()),curvepoint1.lat()},{x_from_lon(curvepoint2.lon()),curvepoint2.lat()});
//                    }
//                        LatLon lastcurvepoint = getStreetSegmentCurvePoint( curvepoint_count - 1,street_segment_id);
//                        g.draw_line({x_from_lon(lastcurvepoint.lon()),lastcurvepoint.lat()},{ xend ,  yend });
//                }
//              }
//          }
          
  //        



void draw_street_name(ezgl::renderer &g){
    unsigned zoom = tell_zoom_level(g);
    unsigned number = getNumStreetSegments();
    string street_name;
    unsigned street_id;
    double x,y;
    bool one_way;
    if(zoom > 7){
        for(int segment_id = 0 ; segment_id <  mid_point.size() ; segment_id++){
            street_id = getInfoStreetSegment(segment_id).streetID;
            one_way = getInfoStreetSegment(segment_id).oneWay;
            street_name = getStreetName(street_id);

            x = mid_point[segment_id].x;
            y = mid_point[segment_id].y;
            double angle = segments_angle[segment_id]; 
            if(nightMode){
                g.set_color(WHITE);
            }else{
                 g.set_color(BLACK);
            }
           
            g.set_text_rotation(angle);
            g.set_font_size(7);
            if(zoom > 11){
                g.set_font_size(11);
            }
            if(within_visible_world(g,x,y) && street_name != "<unknown>"){
                if(one_way){
                        street_name = street_name + " =>";
                        g.draw_text({x,y},street_name);
                        same_name.push_back(street_name);
                        
                    }
                    else{
                        g.draw_text({x,y},street_name);
                        same_name.push_back(street_name);
                    }
            }
        }
    }
}



void highlight_segments(vector <unsigned> segment_id,ezgl::renderer &g){
    int zoom = tell_zoom_level(g);
    // clearing already highlighted segments
    for(int i = 0 ;i < segments_highlight.size(); i++){
        if(segments_highlight[i].highlight == true){
            int first_intersection_id = getInfoStreetSegment(i).from;
            int second_intersection_id = getInfoStreetSegment(i).to;
            int xstartlongi = getIntersectionPosition(first_intersection_id).lon();
            int xistart = x_from_lon (xstartlongi);
            int ystart = getIntersectionPosition(first_intersection_id).lat();
            int xfinishlongi = getIntersectionPosition(second_intersection_id).lon();
            int xend = x_from_lon (xfinishlongi);
            int yend = getIntersectionPosition(second_intersection_id).lat();
            if(zoom >= 6)
                g.set_line_width(4);
            if(zoom <=5 && zoom >3)                      // depending on zoom the streets appear
                g.set_line_width(2);
            g.set_color(211,211,211); 
            g.draw_line({xistart, ystart},{ xend ,  yend });
        }
    }
    for(int i = 0 ; i < segment_id.size() ; i++){
        segments_highlight[segment_id[i]].highlight = true;
        int first_intersection_id = getInfoStreetSegment(segment_id[i]).from;
            int second_intersection_id = getInfoStreetSegment(segment_id[i]).to;
            int xstartlongi = getIntersectionPosition(first_intersection_id).lon();
            int xistart = x_from_lon (xstartlongi);
            int ystart = getIntersectionPosition(first_intersection_id).lat();
            int xfinishlongi = getIntersectionPosition(second_intersection_id).lon();
            int xend = x_from_lon (xfinishlongi);
            int yend = getIntersectionPosition(second_intersection_id).lat();
            if(zoom >= 6)
                g.set_line_width(4);
            if(zoom <=5 && zoom >3)                      // depending on zoom the streets appear
                g.set_line_width(2);
            g.set_color(RED); 
            g.draw_line({xistart, ystart},{ xend ,  yend });
    }
    
}

//function called to draw intersections,segments,features...........
// credits tutorial notes
void draw_main_canvas(renderer &g){
    if(nightMode){
        g.set_color(35,49,62);
    }else{
        g.set_color(WHITE);
    }
    g.fill_rectangle({x_from_lon (min_lon),min_lat},{x_from_lon (max_lon), max_lat});
    if(!nightMode){
        draw_features(g);
    }
    draw_street(g);
    plot_intersections(g);
    draw_POI(g);
    draw_street_name(g);
    zoom_global=tell_zoom_level(g);
    if(path_highlight)
        highlight_segments( pathFound,g);
}

//"main" function to setup ezgl and draw the map
void draw_map (){
FeatureType random = Beach;
//setup ezgl configuration    
application::settings settings;
settings.main_ui_resource = "libstreetmap/resources/main.ui";
settings.window_identifier = "MainWindow";
settings.canvas_identifier = "MainCanvas";
//creating the application
application application(settings);
initialize_Intersections();
initialize_Features();
initialize_POI();
initialize_segments_angle();
initialize_mid_point();
initialize_segments();
//path_taking(map_path);
//Co-ordinate system
ezgl::rectangle initial_world({x_from_lon (min_lon), min_lat},{x_from_lon (max_lon), max_lat});
//creating canvas
application.add_canvas("MainCanvas",draw_main_canvas,initial_world);
application.run(initial_setup,act_on_mouse_click,nullptr,act_on_key_press);
}



