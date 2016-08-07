#include "NoximApp.h"

int seed;
double rho;
double lambda;
double logtmin, logtmax;
double maxpar;

bool MyCompareAscend(const PEPrice& d1, const PEPrice& d2)
{
  return d1.price < d2.price;
}
bool MyCompareDscend(const PEPrice& d1, const PEPrice& d2)
{
  return d1.price > d2.price;
}

void initial_parameters(){
    double mu;
    seed = 36;
    rho = 0.5;
    logtmin = 3.0;
    logtmax = 13.0;
    //maxpar = NUM_PES;
    maxpar = NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y;
    mu = avg_lifetime ();
    lambda = rho * maxpar / mu;
    srandom (seed);
}

double drandom ()
{
  return (double) (random() & 0x7fffffff) / (double) 0x7fffffff;
}

/* CHOOSE FROM EXPONENTIAL : choose a value from an exp distribution
   with the given parameter lambda */

double choose_from_exponential ()
{
  double x;

  do x = drandom (); while (x == 0.0);

  int a = (int)(-log (drandom ()) / lambda);
  return (float)a;
}

/* CHOOSE FROM LOG UNIFORM : low and high are the exponents of the
   range; i.e. low = 0 and high = 3 would have a range from 1 second
   to exp(3) seconds */

double choose_from_log_uniform (double low, double high)
{
  double x = drandom () * (high-low) + low;
  //cout <<high <<" :" << x <<": " << low << endl;
  return exp(x);
}

/* CHOOSE LIFETIME : log uniform distribution between tmin and tmax */

double choose_lifetime ()
{
  int a = (int)choose_from_log_uniform (logtmin, logtmax);
  return (float)a;
}

/* AVG_LIFETIME: calculates the average lifetime in a log uniform
   distribution with parameters tmin and tmax */

double avg_lifetime ()
{
  return (exp (logtmax) - exp(logtmin)) / (logtmax - logtmin);
}

/* CHOOSE PARALLELISM : log uniform distribution between 1 and maxpar */

double choose_parallelism ()
{
  return choose_from_log_uniform (0.0, log(maxpar));
}

double choose_sigma ()
{
  return (drandom() * 2.0);
}


// Used for application generation and mapping

//float Application::Sa (int n, float A, float cv2)
float Application::Sa (int n)
{
  /*if (cv2 <= 1.0) {
 
    // low variance model
    if (n <= A) {
      return A*n / (A + cv2/2 * (n-1));
    } else if (n < 2*A - 1) {
      return A*n / (cv2 * (A - 0.5) + n * (1 - cv2/2));
    } else {
      return A;
    }
  } else {
    // high variance model 
    if (n < A*cv2 + A - cv2) {
      return n*A * (cv2+1) / (cv2 * (n+A-1) + A);
    } else {
      return A;
    }
  }*/
  if (sig <= 1.0) { 
    // low variance model
    if (n <= A) {
      return A*n / (A + sig/2 * (n-1));
    } else if (n < 2*A - 1) {
      return A*n / (sig * (A - 0.5) + n * (1 - sig/2));
    } else {
      return A;
    }
  } else {
    // high variance model 
    if (n < A*sig + A - sig) {
      return n*A * (sig+1) / (sig * (n+A-1) + A);
    } else {
      return A;
    }
  }
}


void Application::mapToCore(deque<int> core_list){
  cores = core_list;
}

void Application::ini_mapping(int n, int time){
  cores.push_back(n);
  mapping_time = (double)time;
}

void Application::get_neighbors(){
  neighbors.clear();
  for (deque<int>::iterator it = cores.begin(); it != cores.end(); it++){
    int x = id2Coord(*it).x;
    int y = id2Coord(*it).y;
    if (x+1 < NoximGlobalParams::mesh_dim_x){
      if (find(neighbors.begin(), neighbors.end(), (*it+1)) == neighbors.end()){
        neighbors.push_back(*it+1);
      }
    }
    if (x-1 >= 0){
      if (find(neighbors.begin(), neighbors.end(), (*it-1)) == neighbors.end()){
        neighbors.push_back(*it-1);
      }
    }
    if (y+1 < NoximGlobalParams::mesh_dim_y){
      if (find(neighbors.begin(), neighbors.end(), (*it+NoximGlobalParams::mesh_dim_y)) == neighbors.end()){
        neighbors.push_back(*it+NoximGlobalParams::mesh_dim_y);
      }
    }
    if (y-1 >= 0){
      if (find(neighbors.begin(), neighbors.end(), (*it-NoximGlobalParams::mesh_dim_y)) == neighbors.end()){
        neighbors.push_back(*it-NoximGlobalParams::mesh_dim_y);
      }
    }
  }
}

void Application::get_margins(){
  margins.clear();
  for (deque<int>::iterator it = cores.begin(); it != cores.end(); it++){
    int x = id2Coord(*it).x;
    int y = id2Coord(*it).y;
    bool flag = 0;
    if (x+1 < NoximGlobalParams::mesh_dim_x){
      if (find(cores.begin(), cores.end(), (*it+1)) == cores.end()){
        flag = flag | 1;
      }
    }
    if (x-1 >= 0){
      if (find(cores.begin(), cores.end(), (*it-1)) == cores.end()){
        flag = flag | 1;
      }
    }
    if (y+1 < NoximGlobalParams::mesh_dim_y){
      if (find(cores.begin(), cores.end(), (*it+NoximGlobalParams::mesh_dim_y)) == cores.end()){
        flag = flag | 1;
      }
    }
    if (y-1 >= 0){
      if (find(cores.begin(), cores.end(), (*it-NoximGlobalParams::mesh_dim_y)) == cores.end()){
        flag = flag | 1;
      }
    }
    if (flag)
      margins.push_back(*it);
  }
}


int Application::expand(NoximTile* t[32][32]){
  neighbors.clear();
  get_neighbors();
  deque<PEPrice> free_neighbors;
  for (deque<int>::iterator it = neighbors.begin(); it != neighbors.end(); it++){
    int x = id2Coord(*it).x;
    int y = id2Coord(*it).y;
    if (!t[x][y]->pe->occupied){
      PEPrice temp;
      temp.id = *it;
      temp.price = t[x][y]->pe->price;
      free_neighbors.push_back(temp);
    }
  }
  if (free_neighbors.size() == 0)
    return 0;
  else{
    sort(free_neighbors.begin(), free_neighbors.end(), MyCompareAscend);
    float cheapest = free_neighbors[0].price;
    if (money_allowed - money_used < cheapest){
        return 0;
    }
    for (deque<PEPrice>::iterator it = free_neighbors.begin(); it != free_neighbors.end(); it++){
      //float cheapest = t[id2Coord(free_neighbors[0].id).x)][id2Coord(free_neighbors[0].id).y)]->pe->price;
      if (money_allowed >= money_used + it->price) {
        invade(it->id, t);
      }
    }
  }
}

int Application::shrink(NoximTile* t[32][32]){
  get_margins();
  deque<PEPrice> inner_margins;
  for (deque<int>::iterator it = margins.begin(); it != margins.end(); it++){
    PEPrice temp;
    int x = id2Coord(*it).x;
    int y = id2Coord(*it).y;
    temp.id = *it;
    temp.price = t[x][y]->pe->price;
    inner_margins.push_back(temp);
  }
  sort(inner_margins.begin(), inner_margins.end(), MyCompareDscend);
  for (deque<PEPrice>::iterator it = inner_margins.begin(); it != inner_margins.end(); it++){
    if (money_used > money_allowed){
      retreat(it->id, t);
    }
  }

}
void Application::Application::claim(int n, NoximTile* t[32][32]){
}
void Application::invade(int n, NoximTile* t[32][32]){
  int x = id2Coord(n).x;
  int y = id2Coord(n).y;
  //t[x][y]->pe->occupied = 1;
  //t[x][y]->pe->app = *this;
  t[x][y]->pe->mapTask(arrival, arrival + lifetime);

  cores.push_back(n);
  money_used += t[x][y]->pe->price;
}
void Application::retreat(int n, NoximTile* t[32][32]){
  int x = id2Coord(n).x;
  int y = id2Coord(n).y;
  t[x][y]->pe->clearTask();

  money_used -= t[x][y]->pe->price;
  for (deque<int>::iterator it = cores.begin(); it != cores.end(); it++){
    if (*it == n){
      cores.erase(it);
    } 
  }
}