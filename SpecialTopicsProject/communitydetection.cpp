
/**
 * @file
 * @author  Doga Dikbayir
 * @version 1.0
 *
 * This program is the vertex-centric implementation of the Louvain fast modularity unfolding community detection
 * method. The code is written over GraphChi's application template
 *
 *
 */

#include <cmath>
#include <map>
#include <string>

#include "graphchi_basic_includes.hpp"
#include "util/labelanalysis.hpp"

using namespace graphchi;

#define GRAPHCHI_DISABLE_COMPRESSION

 //Vertex with the low vid_t will be used as the source in an edge.
struct bidirectional_label {
    vid_t smaller_one;
    vid_t larger_one;
    //double weight; 
};

struct community {
    double internal;
    double total;
    double size;
    //vid_t commId;
};
/*
struct vertex_data{
    vid_t vertex_id;
    vid_t commId;
};
*/
vid_t & neighbor_label(bidirectional_label & bidir, vid_t myid, vid_t nbid) {
    if (myid < nbid) {
        return bidir.larger_one;
    } else {
        return bidir.smaller_one;
    }
}

vid_t & my_label(bidirectional_label & bidir, vid_t myid, vid_t nbid) {
    if (myid < nbid) {
        return bidir.smaller_one;
    } else {
        return bidir.larger_one;
    }
}

 
typedef vid_t VertexDataType;       // vid_t is the vertex id type
typedef bidirectional_label EdgeDataType;  // Note, 8-byte edge data

void parse(bidirectional_label &x, const char * s) { } // Do nothing


std::map <vid_t, community> communities;
size_t totalGraph;

/**
 * GraphChi programs need to subclass GraphChiProgram<vertex-type, edge-type> 
 * class. The main logic is usually in the update function.
 */
struct LouvainProgram : public GraphChiProgram<VertexDataType, EdgeDataType> {
    
    /**
     *  Vertex update function.
     */
    void update(graphchi_vertex<VertexDataType, EdgeDataType> &vertex, graphchi_context &gcontext) {
        
        assert(gcontext.scheduler != NULL);
        //vid_t newlabel;
        if (gcontext.iteration == 0) {
            //In the first iteration, collect and assign the parameters required to calculate modularity gain
            community com = {0,vertex.num_edges(),1.0};
            communities.insert(std::pair<vid_t, community>(vertex.id(),com));

            vid_t data = vertex.id(); // initially each vertex has its own community
            printf("%s\n", "test" );
            vertex.set_data(data); //set data
            gcontext.scheduler->add_task(vertex.id());         //schedules itself for the next iteration
            
        } else {
            if (vertex.num_edges() == 0) return; // ignore nodes with no connections
            //initialize modularity gain
            double bestGain = 0;
            vid_t bestLabel = vertex.id();
            //look for modularity gain and update the map
            double curN = vertex.num_edges(); // get number of neighbors

            //double noConns=0;
            std::map<vid_t,double> nComs; //neighbour communities

            //initialize current community connection map entry
            community vCom = communities.find(vertex.get_data())->second; //current community
            
            if(vCom.size == 1){
                nComs.insert(std::pair<vid_t,double>(vertex.get_data(),0.0));
            }

            //calculate the number of connections from source to the community
            for(int i=0; i<curN;i++){
                bidirectional_label edgelabel = vertex.edge(i)->get_data();
                //get neighbor id
                vid_t nblabel = neighbor_label(edgelabel, vertex.id(), vertex.edge(i)->vertex_id());
                std::map<vid_t,double>::iterator existing = nComs.find(nblabel);
                if(existing == nComs.end()){
                    nComs.insert(std::pair<vid_t, double>(nblabel,1.0));
                }
                else{
                    existing->second += 1.0; 
                }
            }
            double noConnsCur = nComs.find(vertex.get_data())->second; //connections inside current community
            

            //calculate best modularity gain, details of the formula are in the source paper
            for(int k=0; k<curN;k++){
                bidirectional_label edgelabel = vertex.edge(k)->get_data();
                //get neighbor id
                vid_t nblabel = neighbor_label(edgelabel, vertex.id(), vertex.edge(k)->vertex_id());
                community nCom = communities.find(nblabel)->second;  //get the neigh community

                double nIn = nCom.internal; //get internal connection count
                double nTotal = nCom.total; //get total connections of neighCom
                double noConns = nComs.find(nblabel)->second; //connections from vertex to neigh community
                double m2 = totalGraph *2; //total connections in the graph

                //modularity gain
                double curGain = (nIn + noConns)/(m2) - pow(((nTotal+curN)/m2),2);
                curGain = curGain - (nIn/m2) - pow((nTotal/m2),2) - pow((curN/m2),2);
                
                //keep track of the best community to insert the vertex, note ties are broken automatically here
                if(curGain > bestGain){
                    printf("ok\n");
                    bestGain = curGain;
                    bestLabel = nblabel; 

                }
            }
            //remove vertex from its current community and place into the best modularity gain community
            //
            if(bestLabel != vertex.get_data()){
                vertex.set_data(bestLabel);
                //we still need to inform the edges about the community change
                for(int i=0; i<vertex.num_edges(); i++) {
                    bidirectional_label labels_on_edge = vertex.edge(i)->get_data();
                    my_label(labels_on_edge, vertex.id(), vertex.edge(i)->vertex_id()) = bestLabel;
                    vertex.edge(i)->set_data(labels_on_edge);
                    //gcontext.scheduler->add_task(vertex.id());
                }
                communities.find(bestLabel)->second.internal += nComs.find(bestLabel)->second * 2;
                communities.find(bestLabel)->second.total += vertex.num_edges();
                communities.find(bestLabel)->second.size += 1.0;
                
                vCom.internal -= noConnsCur*2;
                vCom.total -= vertex.num_edges();
                vCom.size -= 1.0;
                for(int i=0; i<vertex.num_edges(); i++) {
                    if (gcontext.iteration > 0)
                        gcontext.scheduler->add_task(vertex.edge(i)->vertex_id());  
                }
                
            }        
        }
    }
    
    /**
     * Called before an iteration starts.
     */
    void before_iteration(int iteration, graphchi_context &info) {
        totalGraph = info.nedges; //get total weight sum
    }
    
    /**
     * Called after an iteration has finished.
     */
    void after_iteration(int iteration, graphchi_context &info) {
        //info.scheduler->add_task_to_all();
    }
    
    /**
     * Called before an execution interval is started.
     */
    void before_exec_interval(vid_t window_st, vid_t window_en, graphchi_context &ginfo) {        
    }
    
    /**
     * Called after an execution interval has finished.
     */
    void after_exec_interval(vid_t window_st, vid_t window_en, graphchi_context &ginfo) {        
    }
    
};

int main(int argc, const char ** argv) {
    /* GraphChi initialization will read the command line 
     arguments and the configuration file. */
    graphchi_init(argc, argv);
    
    /* Metrics object for keeping track of performance counters
     and other information. */
    metrics m("community-detection");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file");  // Base filename
    int niters           = get_option_int("niters", 10); // Number of iterations (max)
    bool scheduler       = true;    // Always run with scheduler
        
    /* Process input file - if not already preprocessed */
    int nshards             = convert_if_notexists<EdgeDataType>(filename, get_option_string("nshards", "auto"));

    if (get_option_int("onlyresult", 0) == 0) {
        /* Run */
        LouvainProgram program;
        graphchi_engine<VertexDataType, EdgeDataType> engine(filename, nshards, scheduler, m); 
        engine.run(program, niters);
    }
    
    /* I use the community detection analysis tool in GraphChi for outputting */
    m.start_time("label-analysis");
    
    analyze_labels<vid_t>(filename);
    
    m.stop_time("label-analysis");
    
    /* Report execution metrics */
    metrics_report(m);
    return 0;
}

