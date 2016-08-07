/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the Network-on-Chip
 */

#include "NoximNoC.h"

void NoximNoC::buildMesh()
{
    // Check for routing table availability
    if (NoximGlobalParams::routing_algorithm == ROUTING_TABLE_BASED)
	assert(grtable.load(NoximGlobalParams::routing_table_filename));

    // Check for traffic table availability
    if (NoximGlobalParams::traffic_distribution == TRAFFIC_TABLE_BASED)
	assert(gttable.load(NoximGlobalParams::traffic_table_filename));
	
	// Generate application queue
	generate_arrivals ();

    // Create the mesh as a matrix of tiles
    for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++) {
	for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++) {
	    // Create the single Tile with a proper name
	    char tile_name[20];
	    sprintf(tile_name, "Tile[%02d][%02d]", i, j);
	    t[i][j] = new NoximTile(tile_name);

	    // Tell to the router its coordinates
	    t[i][j]->r->configure(j * NoximGlobalParams::mesh_dim_x + i,
				  NoximGlobalParams::stats_warm_up_time,
				  NoximGlobalParams::buffer_depth,
				  grtable);

	    // Tell to the PE its coordinates
	    t[i][j]->pe->local_id = j * NoximGlobalParams::mesh_dim_x + i;
	    t[i][j]->pe->traffic_table = &gttable;	// Needed to choose destination
	    t[i][j]->pe->never_transmit = (gttable.occurrencesAsSource(t[i][j]->pe->local_id) == 0);

	    // Map clock and reset
	    t[i][j]->clock(clock);
	    t[i][j]->reset(reset);

	    // Map Rx signals
	    t[i][j]->req_rx[DIRECTION_NORTH] (req_to_south[i][j]);
	    t[i][j]->flit_rx[DIRECTION_NORTH] (flit_to_south[i][j]);
	    t[i][j]->ack_rx[DIRECTION_NORTH] (ack_to_north[i][j]);

	    t[i][j]->req_rx[DIRECTION_EAST] (req_to_west[i + 1][j]);
	    t[i][j]->flit_rx[DIRECTION_EAST] (flit_to_west[i + 1][j]);
	    t[i][j]->ack_rx[DIRECTION_EAST] (ack_to_east[i + 1][j]);

	    t[i][j]->req_rx[DIRECTION_SOUTH] (req_to_north[i][j + 1]);
	    t[i][j]->flit_rx[DIRECTION_SOUTH] (flit_to_north[i][j + 1]);
	    t[i][j]->ack_rx[DIRECTION_SOUTH] (ack_to_south[i][j + 1]);

	    t[i][j]->req_rx[DIRECTION_WEST] (req_to_east[i][j]);
	    t[i][j]->flit_rx[DIRECTION_WEST] (flit_to_east[i][j]);
	    t[i][j]->ack_rx[DIRECTION_WEST] (ack_to_west[i][j]);

	    // Map Tx signals
	    t[i][j]->req_tx[DIRECTION_NORTH] (req_to_north[i][j]);
	    t[i][j]->flit_tx[DIRECTION_NORTH] (flit_to_north[i][j]);
	    t[i][j]->ack_tx[DIRECTION_NORTH] (ack_to_south[i][j]);

	    t[i][j]->req_tx[DIRECTION_EAST] (req_to_east[i + 1][j]);
	    t[i][j]->flit_tx[DIRECTION_EAST] (flit_to_east[i + 1][j]);
	    t[i][j]->ack_tx[DIRECTION_EAST] (ack_to_west[i + 1][j]);

	    t[i][j]->req_tx[DIRECTION_SOUTH] (req_to_south[i][j + 1]);
	    t[i][j]->flit_tx[DIRECTION_SOUTH] (flit_to_south[i][j + 1]);
	    t[i][j]->ack_tx[DIRECTION_SOUTH] (ack_to_north[i][j + 1]);

	    t[i][j]->req_tx[DIRECTION_WEST] (req_to_west[i][j]);
	    t[i][j]->flit_tx[DIRECTION_WEST] (flit_to_west[i][j]);
	    t[i][j]->ack_tx[DIRECTION_WEST] (ack_to_east[i][j]);

	    // Map buffer level signals (analogy with req_tx/rx port mapping)
	    t[i][j]->free_slots[DIRECTION_NORTH] (free_slots_to_north[i][j]);
	    t[i][j]->free_slots[DIRECTION_EAST] (free_slots_to_east[i + 1][j]);
	    t[i][j]->free_slots[DIRECTION_SOUTH] (free_slots_to_south[i][j + 1]);
	    t[i][j]->free_slots[DIRECTION_WEST] (free_slots_to_west[i][j]);

	    t[i][j]->free_slots_neighbor[DIRECTION_NORTH] (free_slots_to_south[i][j]);
	    t[i][j]->free_slots_neighbor[DIRECTION_EAST] (free_slots_to_west[i + 1][j]);
	    t[i][j]->free_slots_neighbor[DIRECTION_SOUTH] (free_slots_to_north[i][j + 1]);
	    t[i][j]->free_slots_neighbor[DIRECTION_WEST] (free_slots_to_east[i][j]);

	    // NoP 
	    t[i][j]->NoP_data_out[DIRECTION_NORTH] (NoP_data_to_north[i][j]);
	    t[i][j]->NoP_data_out[DIRECTION_EAST] (NoP_data_to_east[i + 1][j]);
	    t[i][j]->NoP_data_out[DIRECTION_SOUTH] (NoP_data_to_south[i][j + 1]);
	    t[i][j]->NoP_data_out[DIRECTION_WEST] (NoP_data_to_west[i][j]);

	    t[i][j]->NoP_data_in[DIRECTION_NORTH] (NoP_data_to_south[i][j]);
	    t[i][j]->NoP_data_in[DIRECTION_EAST] (NoP_data_to_west[i + 1][j]);
	    t[i][j]->NoP_data_in[DIRECTION_SOUTH] (NoP_data_to_north[i][j + 1]);
	    t[i][j]->NoP_data_in[DIRECTION_WEST] (NoP_data_to_east[i][j]);
	}
    }

    // dummy NoximNoP_data structure
    NoximNoP_data tmp_NoP;

    tmp_NoP.sender_id = NOT_VALID;

    for (int i = 0; i < DIRECTIONS; i++) {
	tmp_NoP.channel_status_neighbor[i].free_slots = NOT_VALID;
	tmp_NoP.channel_status_neighbor[i].available = false;
    }

    // Clear signals for borderline nodes
    for (int i = 0; i <= NoximGlobalParams::mesh_dim_x; i++) {
	req_to_south[i][0] = 0;
	ack_to_north[i][0] = 0;
	req_to_north[i][NoximGlobalParams::mesh_dim_y] = 0;
	ack_to_south[i][NoximGlobalParams::mesh_dim_y] = 0;

	free_slots_to_south[i][0].write(NOT_VALID);
	free_slots_to_north[i][NoximGlobalParams::mesh_dim_y].write(NOT_VALID);

	NoP_data_to_south[i][0].write(tmp_NoP);
	NoP_data_to_north[i][NoximGlobalParams::mesh_dim_y].write(tmp_NoP);

    }

    for (int j = 0; j <= NoximGlobalParams::mesh_dim_y; j++) {
	req_to_east[0][j] = 0;
	ack_to_west[0][j] = 0;
	req_to_west[NoximGlobalParams::mesh_dim_x][j] = 0;
	ack_to_east[NoximGlobalParams::mesh_dim_x][j] = 0;

	free_slots_to_east[0][j].write(NOT_VALID);
	free_slots_to_west[NoximGlobalParams::mesh_dim_x][j].write(NOT_VALID);

	NoP_data_to_east[0][j].write(tmp_NoP);
	NoP_data_to_west[NoximGlobalParams::mesh_dim_x][j].write(tmp_NoP);

    }

    // invalidate reservation table entries for non-exhistent channels
    for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++) {
	t[i][0]->r->reservation_table.invalidate(DIRECTION_NORTH);
	t[i][NoximGlobalParams::mesh_dim_y - 1]->r->reservation_table.invalidate(DIRECTION_SOUTH);
    }
    for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++) {
	t[0][j]->r->reservation_table.invalidate(DIRECTION_WEST);
	t[NoximGlobalParams::mesh_dim_x - 1][j]->r->reservation_table.invalidate(DIRECTION_EAST);
    }
}

NoximTile *NoximNoC::searchNode(const int id) const
{
    for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++)
	for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++)
	    if (t[i][j]->r->local_id == id)
		return t[i][j];

    return false;
}

void NoximNoC::generate_arrivals ()
{
	double cycle = NoximGlobalParams::stats_warm_up_time + DEFAULT_RESET_TIME;	
	initial_parameters();
	APPLICATION temp_app;
	for (int i=0; i<NPROCS; i++) {
		temp_app.app_id = i;
		temp_app.arrival = cycle;
		temp_app.lifetime = choose_lifetime ();
		temp_app.A = choose_parallelism ();
		temp_app.sig = choose_sigma ();
		cycle += choose_from_exponential ();
		app_queue.insert (std::pair<int, APPLICATION>((int)cycle, temp_app));
		cout << temp_app.arrival <<": "<< temp_app.app_id <<" "<<temp_app.lifetime << endl; 
  	}
  	//int s[10];
  	//for (int i = 0; i < 10000; i++)
  		//cout << choose_from_log_uniform(1, 10) << endl;
  		//cout << drandom () << endl;
}

void NoximNoC::mapping(){
	if (reset.read()){
		t_money = NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y/LN;
	}
	else{
		//random_mapping();
		//dist_mapping();
		moc_mapping();
	}
}
void NoximNoC::random_mapping(){
	/*int time = (int)sc_time_stamp().to_double() / 1000;
	int n_core = 8;
	deque <int> idle_core;

	if (app_queue.find(time) != app_queue.end()){
		waiting_queue.push_back(app_queue[time]);
	}
	while (waiting_queue.size() != 0){
		for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++) {
			for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++) {
				if (!t[i][j]->pe->occupied){
					idle_core.push_back(t[i][j]->pe->local_id);
		    	}
			}
		}

		if (idle_core.size() >= n_core){
			random_shuffle(idle_core.begin(), idle_core.end());
			deque <int> core_list;
			for (int i = 0; i < n_core; i++){
				core_list.push_back(idle_core[i]);
			}
			cout << "Time " << time;
			cout << " Mapping App:";
			cout << waiting_queue.at(0).app_id <<" ";
			int t_time = (int)waiting_queue.at(0).arrival;
			app_queue[t_time].mapToCore(core_list);
			waiting_queue.pop_front();
			for (int i = 0; i < core_list.size(); i++){
				int x = id2Coord(core_list[i]).x;
				int y = id2Coord(core_list[i]).y;
				t[x][y]->pe->mapTask(app_queue[time]);
				cout << core_list[i] <<" ";
			}
			cout << "Speedup is " << app_queue[t_time].Sa(core_list.size()) << endl;
			//cout << "Parameters: " << app_queue[t_time].A << " " << app_queue[t_time].sig<< endl;
		}
		else
			break;
	}*/
}

bool MyCompare(const PEPrice& d1, const PEPrice& d2)
{
	return d1.price > d2.price;
}

void NoximNoC::dist_mapping(){
/*	int time = (int)sc_time_stamp().to_double() / 1000;
	deque <int> idle_core;

	if (time % interval == 0){
		vector <PEPrice> pep;
		for (int i = 0; i < NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y; i++){
			int x = id2Coord(i).x;
			int y = id2Coord(i).y;
			if (t[x][y]->pe->price > 0){
				PEPrice temp;
				temp.p_id = t[x][y]->pe->local_id;
				temp.price = t[x][y]->pe->price;				
			}
			pep.push_back(temp);
		}
		std::sort(pep.begin(), pep.end(), MyCompare);
		cout << "Sorting... " << endl;
		for (int i = 0; i < NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y; i++){
			cout << pep[i].p_id << " " << pep[i].price << endl;
		}
	}

	if (app_queue.find(time) != app_queue.end()){
		
	}*/
}

void NoximNoC::initial_mapping(){
	int time = (int)sc_time_stamp().to_double() / 1000;
	if (app_queue.find(time) != app_queue.end()){
		waiting_queue.push_back(app_queue[time]);
	}

	// initial mapping when new application arrives
	while (waiting_queue.size() != 0){
		int arrival = (int)waiting_queue[0].arrival;
		int pe_id = free_pe[free_pe.size()-1].id;
		int x = id2Coord(pe_id).x;
		int y = id2Coord(pe_id).y;
		app_queue[arrival].ini_mapping(pe_id, time);
		t[x][y]->pe->mapTask(time, time + app_queue[arrival].lifetime);
	}
}

void NoximNoC::idle_core_price_sort(){
	int time = (int)sc_time_stamp().to_double() / 1000;
	if (time % INTERVAL == 0){
		for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++) {
			for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++) {
				if ((!t[i][j]->pe->occupied) && (t[i][j]->pe->price > 0)){
					PEPrice temp;
					temp.id = t[i][j]->pe->local_id;
					temp.price = t[i][j]->pe->price;
					free_pe.push_back(temp);
					//idle_core.push_back(t[i][j]->pe->local_id);
		    	}
			}
		}
		std::sort(free_pe.begin(), free_pe.end(), MyCompare);
		cout << "Sorting... " << endl;
		for (int i = 0; i < free_pe.size(); i++){
			cout << free_pe[i].id << " " << free_pe[i].price << endl;
		}
	}
}

void NoximNoC::update_running_app(){
	for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++) {
		for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++) {
			if (t[i][j]->pe->occupied){
				//int app_t = t[i][j]->pe->app.arrival;
				int app_t = t[i][j]->pe->arrival;
				if (find(running_app.begin(), running_app.end(), app_t) == running_app.end()){
					running_app.push_back(app_t);
				}
			}
		}
	}
}

void NoximNoC::update_money(){
	for (int i = 0; i < running_app.size(); i++){
		int arrival = running_app[i];
		app_queue[arrival].money_allowed = 1/running_app.size();
		//running_app[i].money_allowed = 1/running_app.size();
	}
}

void NoximNoC::moc_mapping(){
	idle_core_price_sort();
	update_running_app();
	update_money();
	int time = (int)sc_time_stamp().to_double() / 1000;
	//if (time % 1000 == 0){
		for (int r = 0; r < running_app.size(); r++){
			int arrival = running_app[r];
			if (app_queue[arrival].money_used < app_queue[arrival].money_allowed){
				app_queue[arrival].expand(t);
			}
			else {
				app_queue[arrival].shrink(t);
			}
		}
	//}
}
