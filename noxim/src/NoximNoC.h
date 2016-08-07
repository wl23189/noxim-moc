/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file represents the top-level testbench
 */

#ifndef __NOXIMNOC_H__
#define __NOXIMNOC_H__

#include <systemc.h>
#include <map>
#include <vector>
#include <deque>
#include <algorithm>
#include "NoximApp.h"
#include "NoximTile.h"
#include "NoximGlobalRoutingTable.h"
#include "NoximGlobalTrafficTable.h"

using namespace std;

SC_MODULE(NoximNoC)
{

    // I/O Ports
    sc_in_clk clock;		// The input clock for the NoC
    sc_in < bool > reset;	// The reset signal for the NoC

    // Signals
    sc_signal <bool> req_to_east[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <bool> req_to_west[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <bool> req_to_south[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <bool> req_to_north[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

    sc_signal <bool> ack_to_east[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <bool> ack_to_west[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <bool> ack_to_south[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <bool> ack_to_north[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

    sc_signal <NoximFlit> flit_to_east[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_west[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_south[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_north[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

    sc_signal <int> free_slots_to_east[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <int> free_slots_to_west[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <int> free_slots_to_south[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <int> free_slots_to_north[MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

    // NoP
    sc_signal <NoximNoP_data> NoP_data_to_east[MAX_STATIC_DIM][MAX_STATIC_DIM];
    sc_signal <NoximNoP_data> NoP_data_to_west[MAX_STATIC_DIM][MAX_STATIC_DIM];
    sc_signal <NoximNoP_data> NoP_data_to_south[MAX_STATIC_DIM][MAX_STATIC_DIM];
    sc_signal <NoximNoP_data> NoP_data_to_north[MAX_STATIC_DIM][MAX_STATIC_DIM];

    // Matrix of tiles
    NoximTile *t[MAX_STATIC_DIM][MAX_STATIC_DIM];

    // Global tables
    NoximGlobalRoutingTable grtable;
    NoximGlobalTrafficTable gttable;

    // Variables for lifetime reliability
    double t_money;
    double price[MAX_STATIC_DIM][MAX_STATIC_DIM];
    //Mapping
    map<int, APPLICATION> app_queue;
    deque <APPLICATION> waiting_queue;

    void generate_arrivals ();
    void mapping();
    void random_mapping();
    void dist_mapping();

    // moc algorithm 
    vector <PEPrice> free_pe;
    //vector <APPLICATION> running_app;
    vector<int> running_app;
    void idle_core_price_sort(); // manage free cores
    void initial_mapping();
    void moc_mapping();  // the algorithm in the paper
    //bool MyCompare(const PEPrice& d1, const PEPrice& d2);
    void update_running_app();
    void update_money();

    //---------- Mau experiment <start>
    void flitsMonitor() {

	if (!reset.read()) {
	    //      if ((int)sc_simulation_time() % 5)
	    //        return;

	    unsigned int count = 0;
	    for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++)
		for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++)
		    count += t[i][j]->r->getFlitsCount();
	    cout << count << endl;
	}
    }
    //---------- Mau experiment <stop>

    // Constructor

    SC_CTOR(NoximNoC) {

	//---------- Mau experiment <start>
	/*
	   SC_METHOD(flitsMonitor);
	   sensitive(reset);
	   sensitive_pos(clock);
	 */
	//---------- Mau experiment <stop>

	// Build the Mesh
	buildMesh();
    SC_METHOD(mapping);
    sensitive << reset;
    sensitive << clock.pos();
    }

    // Support methods
    NoximTile *searchNode(const int id) const;


  private:

    void buildMesh();

};

#endif
