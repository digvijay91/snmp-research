#include "pap.cpp" // Ordering of includes matter due to dependency on log.hpp
#include "data.hpp"
#include <fstream>

/*
  Compile using: g++ main.cpp pap.cpp -lpq -lpqxx
*/
void add_all_info(std::vector<std::pair<int,std::pair<std::string,std::string> > > & all_info, sn::graph & g);

int main(){
  using namespace std;
  sn::data d; // for using the database
  vector<sn::log> all_logs; // for logs
  cerr<<"Getting data from database!\n";
  d.get_data("2014-10-13","2014-10-19",all_logs); // read from database
  cerr<<"Done\n";
  sn::graph g_proximity,g_movement[10]; // graph declarations. g_movement is an array where the index represent the time window between movements happen
  
  cerr<<"Getting info for users!\n";
  vector<pair<int,pair<string,string> > > all_info;
  d.get_person_info(all_info); // get person info - Privacy breach :P
  cerr<<"Done\n";
  add_all_info(all_info,g_proximity); // add info to graph with the data gotten from the db
                                      // this essentially creates the nodes for graph
  for(int i=0;i<10;i++){
    add_all_info(all_info,g_movement[i]);
  }
  
  cerr<<"Now processing and populating proximity graph!\n";
  sn::process_n_populate(all_logs,g_proximity); // determine and add weighted egdes. Decent algorithm - possibly optimal
  cerr<<"Now processing movements and populating graph!\n";
  int t=1;
  for(int i=0;i<10;i++){
    cerr<<"For t = "<<t<<endl;
    sn::process_movement(all_logs,g_movement[i],t); // similar to above. Possibly optimal too :P
    t = t * 2;
  }
  
  cerr<<"Now creating friendship graph\n";
  sn::graph g_friendship[10]; // graph array to make using from proximity and movements graphs
  for(int i =0 ;i<10;i++){
    add_all_info(all_info,g_friendship[i]); // add nodes and info
  }

  vector<sn::node> nodes;
  g_proximity.get_all_nodes(nodes);
  t=1;
  // No idea how the following works. TBD: Add comments on how the below algorithm works :P
  for(int i=0;i<10;i++){
    cerr<<"For t = "<<t<<endl;
    t = t * 2;
    int visited[15000]={0};
    for(vector<sn::node>::iterator it = nodes.begin();
        it!= nodes.end();
        it++){
      int it_id = it->id;
      visited[it_id] = true;
      vector<sn::node> neighbors,neighbors2;
      g_proximity.get_neighbors_of(*it,neighbors);
      g_movement[i].get_neighbors_of(*it,neighbors2);
      for(vector<sn::node>::iterator neighbor = neighbors.begin();
          neighbor != neighbors.end();
          neighbor++){
        int id = neighbor->id;
        if(visited[id])continue;
        long int a = g_proximity.get_weight(*it,*neighbor);
        if(a==-1)a=1;
        long int N = g_proximity.get_total_edge_weight(*it);
        g_friendship[i].add_edge(*it,*neighbor,(a * 100)/N);
      }
      for(vector<sn::node>::iterator neighbor2 = neighbors2.begin();
          neighbor2 != neighbors2.end();
          neighbor2++){
        int id = neighbor2->id;
        if(visited[id])continue;
        long int b = g_movement[i].get_weight(*it,*neighbor2);
        if(b==-1)b=1;
        long int temp = g_friendship[i].get_weight(*it,*neighbor2);
        if(temp ==-1){
          b=1;
        }
        else {
          b = temp * b;
        }
        g_friendship[i].add_edge(*it,*neighbor2,b - temp); // b-temp increase weight to b
      }
    }
    std::string file_name = "friendship_t"+std::to_string(t)+".gdf";
    ofstream out(file_name,std::fstream::out);
    out << g_friendship[i].get_gdf();
    out.close();
  }
/*  g_proximity.print();
  g_movement.print();*/
  return 0;
}

void add_all_info(std::vector<std::pair<int,std::pair<std::string,std::string> > > & all_info, sn::graph & g){
  for(int i=0;i<all_info.size();i++){
    std::string rollno = all_info[i].second.first;
    std::string email = all_info[i].second.second;
    g.add_node_info(all_info[i].first,rollno,email);
  }
}

