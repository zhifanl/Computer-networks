// CPSC441 A3 Zhifan Li 30089428
// run it by typing g++ -Wall graph.cpp 
// then type ./a.out
// Citation: Dijkstra algorithm learned from Geeksforgeeks


#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
using namespace std;


int count_vertices; // count num of vertices
int name_to_int_count; // count how many different addresses are scanned
unordered_map<string,vector<pair<string,int> > >map; // map of string as key, vector of pair of string and int as value
// for storing dest, and its adjacency list with distance between src and dest

unordered_map<string,int> name_to_int; // map for mapping dest name to an int
unordered_map<int,string> int_to_name; // map for mapping an int to dest name string

void printFullPath(vector<string>parent, string j)
{
       
    // Base case: If j is source
    if (parent[name_to_int.at(j)] == "")
        return;
    // recursive call to use string at parent's array index of string j
    printFullPath(parent, parent[name_to_int.at(j)]);
   
    std::cout<<"->"<<j;
    // now print the parent path out, then print kid path out recursively
}

// function to call recursive printFullPath function
void printAll(string dest,vector<int> dist, int n, 
                      vector<string> parent)
{
    
    int dest_index=name_to_int.at(dest);
    // print tuple
    std::cout<<"YYC"<<"->" <<int_to_name.at(dest_index)<<"   "<< dist[dest_index]<<"         "<<"YYC";
    printFullPath(parent, int_to_name.at(dest_index)); // call recursive function
    // new line
    cout<<endl;
}

//find shortest path with the dest name
void shortestPath(string dest)
{
    //create set of processed vertices
    set< pair<int, string> > processedVertices;
 
    //create an array of distances, and initialize them to infinite first
    vector<int> dist(name_to_int_count, INT_MAX);
    // parent array of string
    vector<string> parent(name_to_int_count);
    // first one is null
    parent[0]="";
    // bool to show whether the vertex has been checked or no.
    bool checkedVertex[name_to_int_count];
    for(int i=0;i<name_to_int_count;i++){
        checkedVertex[i]=false; // initialize all to false 
    }
    
    processedVertices.insert(make_pair(0,"YYC"));// first one is itself: YYC
    dist[0] = 0;// first one is 0

    vector<string>path; // path, first element is YYC as root
    path.push_back("YYC");
 
    //loop thru the processedVertices until  it is empty
    while (!processedVertices.empty())
    {
        // The first vertex in Set is the minimum distance
        // vertex, extract it from set.
        pair<int, string> tmp = *(processedVertices.begin());
        processedVertices.erase(processedVertices.begin()); // erase it
 
        //second is name of dest, first the distance because property of set will sort it for you.
        string u = tmp.second; // u is the address at processedVertices
 
        // iterator to iterate thru the array
        vector<pair<string,int>  >::iterator i;
        // map is dest as first element of pair, distance as second element
        for (i = map[u].begin(); i != map[u].end(); ++i)
        {
            
            string v = (*i).first; // adj vertice name
            int distance = (*i).second; // get distance
 
            //    If there is shorter path to v through u.
            if (dist[name_to_int.at(v)] > dist[name_to_int.at(u)] + distance)
            {
                    //  If distance of v is not INF then it must be in
                    // our set, so removing it and inserting again
                    // with updated less distance. 
                   
                if (dist[name_to_int.at(v)] != INT_MAX) // if not modified yet
                    processedVertices.erase(processedVertices.find(make_pair(dist[name_to_int.at(v)], v)));
                    // erase it 

                //now update distance
                dist[name_to_int.at(v)] = dist[name_to_int.at(u)] + distance;
                processedVertices.insert(make_pair(dist[name_to_int.at(v)], v));
                // insert to set, automatically sorted
                parent[name_to_int.at(v)] = u;
                // update on parent path array.
                
            }
        }
    }
 
    // after all nodes processed, print the result to user.
    printAll(dest,dist, name_to_int_count, parent);
}


int main(){
    string text_line;// line
    string text_1; // string of src
    string text_2; // string of dest
    string text_3;  // string of distance
    
    count_vertices=0;
    ifstream myFile("topologyfile.txt"); // file describer
    
    name_to_int_count=0; // count num of unique nodes
    // Use a while loop together with the getline() function to read the file line by line
    while (getline (myFile, text_line)) {
        text_1=text_line.substr(0,3);
        text_2=text_line.substr(4,3);

        // if it is not in the map, add to map (int_to_name, name_to_int)
        if(name_to_int.find(text_1)==name_to_int.end()){
            int_to_name[name_to_int_count]=text_1;
            name_to_int[text_1]=name_to_int_count;
            name_to_int_count++; //update
        }

        // if it is not in the map, add to map (int_to_name, name_to_int)
        if(name_to_int.find(text_2)==name_to_int.end()){
            int_to_name[name_to_int_count]=text_2;
            name_to_int[text_2]=name_to_int_count;
            name_to_int_count++;//update
        }
        


        text_3=text_line.substr(8);
        int distance=atoi( text_3.c_str() );    // string distance to int
        // std::cout<<text_1<<" "<<text_2<<" "<<text_3<<endl;
        count_vertices++;
        map[text_1].push_back(make_pair(text_2,distance)); //insert to map
        map[text_2].push_back(make_pair(text_1,distance)); //insert to map
    }

    // Close the file
    myFile.close();
    printf("Vertex     Distance      Path\n");
    shortestPath("YYC");
    shortestPath("YEG");
    shortestPath("YQR");
    shortestPath("YXE");
    shortestPath("YVR");
    shortestPath("YWG");
    shortestPath("YYZ");
    shortestPath("YUL");
    shortestPath("YYJ");
    shortestPath("YOW");
    shortestPath("YQB");
    shortestPath("YHZ");
    shortestPath("YQM");
    shortestPath("YFC");
    shortestPath("YQX");
    shortestPath("YYT");
    shortestPath("YXY");
    shortestPath("YZF");
    shortestPath("YFB");
    shortestPath("YXX");
    shortestPath("YXS");
}