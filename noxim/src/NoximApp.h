#ifndef __NOXIMAPP_H__
#define __NOXIMAPP_H__

#include <cmath>
#include <deque>
#include <algorithm>
#include "NoximMain.h"
#include "NoximTile.h"
using namespace std;

typedef struct Application{
	int app_id;
	double arrival;
	double mapping_time;
	double lifetime;
	double A;
	double sig;
	deque <int> cores;

	void mapToCore(deque<int>);
	float speedup;
	
	deque <int> neighbors;
	deque <int> margins;
	float money_used;
	float money_allowed;
	//float Sa (int n, float A, float cv2);
	float Sa (int n);
	void ini_mapping(int n, int time);
	int expand(NoximTile* t[32][32]);
	int shrink(NoximTile* t[32][32]);
	void claim(int n, NoximTile* t[32][32]);
	void invade(int n, NoximTile* t[32][32]);
	void retreat(int n, NoximTile* t[32][32]);

	void get_neighbors();
	void get_margins();
}APPLICATION;

void initial_parameters();
double drandom ();
double choose_from_exponential ();
double choose_from_log_uniform (double low, double high);
double choose_lifetime ();
double avg_lifetime ();
double choose_parallelism ();
double choose_sigma ();

#endif