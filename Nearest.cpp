//g++ Nearest.cpp -o Nearest -std=c++11 -g -O4
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip> // std::setprecision
#include <cmath>       /* sqrt, cos, M_PI */
#include <time.h>
#include <vector>
#include <algorithm>

#define SIZE 2190113
#define R 6371 //radius of earth
#define MAX 9999999
using namespace std;
#include <chrono> 
using namespace std::chrono; 

struct Point {
  string State; 
  string Name;  
  double Latitude; 
  double Longitude;
  //
  double radLat;
  double radLon;
  double arr[2];
};

double equirect_dist(struct Point* a, struct Point* b);
double equirect_dist_rad(double phi1, double phi2,double lam1,double lam2);
string consensus(string* states, int k);
void vote5(string* States,string* Counties,double* best_dist, int k);

//array-style
struct Point* findclosest(struct Point *points, struct Point* inputPoint);
int indexmax(double *dists,int k);
double maxval(double *dists,int k);
struct Point* findkclosest(struct Point *points, struct Point* inputPoint, int k, double* distances);

//KD tree
struct Node 
{ 
    struct Point pt; // To store k dimensional point 
    Node *left, *right; 
};
struct Node* newNode(struct Point* p) 
{ 
    struct Node* temp = new Node; 
    temp->pt = *p;
    temp->left = temp->right = NULL; 
    return temp; 
}
const int kdim = 2;
Node *insertRec(Node *root, struct Point* point, unsigned depth);
Node* insert(Node *root, struct Point* point);
void kdnn(struct Node *root, struct Point* inputPoint, unsigned depth, double* best_dist, struct Point* closest,int knearest);
double equirect_dist(struct Point* a, struct Point* b);
///


int main()
{
    cout<<"\nWelcome to \"Nearest State County Finder\" \n";
    Point *points=new Point[SIZE];
    int Count = 0;
    cin>>setprecision(13);
    cout<<setprecision(13);
    cout<<"Loading input file\n";
    ifstream in("Inputfile.txt");
       
    in>>setprecision(10);
    while (!in.eof() && Count < SIZE)
    {
        in >> points[Count].State >> points[Count].Name 
        >>points[Count].Latitude >>points[Count].Longitude;
        Count++;
    }
 
   for(int i=0;i<2190113;i++) //Precompute latitude and longitude in radiants, and put in arr[2] format for kd tree
   {
     points[i].radLat=(M_PI)/180 * points[i].Latitude;
     points[i].radLon=(M_PI)/180 * points[i].Longitude;
     points[i].arr[0]=points[i].Latitude;
     points[i].arr[1]=points[i].Longitude;  
   }
   
   struct Node *root = NULL;
    for (int i=0; i<SIZE; i++) //fill the tree
       root = insert(root, &points[i]);
 
 
 
   
   cout<<"How many nearest neighbors do you want? Enter 1<K<10: ";
   int k;
   double inputLati;
   double inputLongi;
   cin>>k;
   while(k<1 || k>10)
   {
     cout<<"K needs to be between 1 and 10: ";
     cin>>k;
   
   }
   
   
   cout<<"Please Enter the Latitude of your point: ";
   
   cin>>inputLati;
   while(inputLati<-90 || inputLati>90 )
   {     
     cout<<"Latitude must be between -90 and 90: ";
     cin>>inputLati;
     cout<<endl;
   }   
   cout<<"Please Enter the Longitude of your point: ";
   cin>>inputLongi;
   while(inputLongi<-180 ||inputLongi>80)
   {
     cout<<"Longitude must be between -180 and 80: ";
     cin>>inputLongi;
     cout<<endl;
   }   
   cout<<"\nYou have chosen to find "<<k<<" nearest neighbors to the point "<<inputLati<<", "<<inputLongi<<"."<<endl<<endl;
   
   /////////////////Make the input point
   Point *inputPoint = new Point;
   inputPoint->Latitude=inputLati;
   inputPoint->arr[0]=inputLati;
   inputPoint->Longitude=inputLongi;
   inputPoint->arr[1]=inputLongi;
   inputPoint->radLat=(M_PI)/180 * inputPoint->Latitude;
   inputPoint->radLon=(M_PI)/180 * inputPoint->Longitude;
   inputPoint->Name="INPUT";
   inputPoint->State="XX";
   

   /////////////////array
   Point* closests = new Point[10];
   double distances[10];
   auto start1 = high_resolution_clock::now();
   closests=findkclosest(points,inputPoint,k,distances); //Call the function
   auto stop1 = high_resolution_clock::now(); 
   auto duration1 = duration_cast<milliseconds>(stop1 - start1);
   
   cout<<"Search by array took "<<duration1.count()<<" milliseconds"<<endl;      
   for(int i=0;i<k;i++)
   {
     cout<<"Closest point #"<<i+1<<" is "<<distances[i]<<" km away, at "<<closests[i].Latitude<<", "<<closests[i].Longitude<<" at "<<closests[i].Name<<", "<<closests[i].State<<". "          <<endl;
   }
   
   /////////////////Kd-Tree
   double best_dist[k];
    for(int i=0; i<k;i++)
    {
        best_dist[i]=MAX;
    }
   unsigned depth=0;
   Point* closests2 = new Point[k];

   auto start = high_resolution_clock::now();
   kdnn(root,inputPoint,depth,best_dist,closests2,k);
   auto stop = high_resolution_clock::now(); 
   auto duration = duration_cast<milliseconds>(stop - start);
   
   cout<<"\nSearch by Kd-tree took "<<duration.count()<<" milliseconds"<<endl; 
   string States[k];
   string Counties[k];
   for(int i=0;i<k;i++)
   {
     cout<<"Closest point #"<<i+1<<" is "<<best_dist[i]<<" km away, at "<<closests2[i].Latitude<<", "<<closests2[i].Longitude<<" at "<<closests2[i].Name<<", "<<closests2[i].State<<". "          <<endl;
     States[i]=closests[i].State;
     Counties[i]=closests[i].Name;
   }
   
   
   /////////////////final output
   cout<<endl<<endl;
   if(k==1)
   {
     cout<<"By consensus, State is "<<States[0]<<" and county is "<<Counties[0]<<endl<<endl<<endl;
   }
   else if(k==2)
   {
     if(States[0]==States[1])cout<<"By consensus, the State is "<<States[0];
     else cout<<"By consensus, the State is {No consensus} ";
     if(Counties[0]==Counties[1])cout<<"and the county is "<<Counties[0]<<endl;
     else cout<<"and the county is {No consensus} \n";
   }
   else if(k>=5)
   {
     vote5(States,Counties,best_dist,k);
   }   
   else //handles 3,4
   {
     cout<<"By consensus, State is "<<consensus(States,k)<<" and county is "<<consensus(Counties,k)<<endl<<endl<<endl;
   }
   
   return 0;

}







void kdnn(struct Node *root, struct Point* inputPoint, unsigned depth, double* best_dist, struct Point* closest, int knearest)
{
    if(root==NULL)
        return;
        
    unsigned cd = depth%kdim;
    double dist = equirect_dist(&(root->pt),inputPoint);
    double arraymax;
    int imax;
    arraymax=maxval(best_dist,knearest);
    imax=indexmax(best_dist,knearest);
    if(dist<arraymax)
    {
        best_dist[imax]=dist;
        closest[imax]=root->pt;
    }
    arraymax=maxval(best_dist,knearest);
    imax=indexmax(best_dist,knearest);
    //which side is best:
    if(inputPoint->arr[cd]<root->pt.arr[cd])//left is best
    {
        kdnn(root->left,inputPoint,depth+1,best_dist,closest,knearest);
        if(root->right!=NULL)
        {
            Point *posbest = new Point;//possible best point of that side 
            if(cd)
            {
                posbest->Latitude=inputPoint->arr[1-cd];
                posbest->Longitude=root->right->pt.arr[cd];
            }
            else
            {
                posbest->Latitude=root->right->pt.arr[cd];
                posbest->Longitude=inputPoint->arr[1-cd];
            }
            if( equirect_dist(posbest,inputPoint) <arraymax )//if bestpossible better than bestdist, do other side
            {
                kdnn(root->right,inputPoint,depth+1,best_dist,closest,knearest);    
            }
        }
    }
    else
    {
        kdnn(root->right,inputPoint,depth+1,best_dist,closest,knearest);
        if(root->left !=NULL)
        {
            Point *posbest = new Point;//possible best point of that side 
            if(cd)
            {
                posbest->Latitude=inputPoint->arr[1-cd];
                posbest->Longitude=root->left->pt.arr[cd];
            }
            else
            {
                posbest->Latitude=root->left->pt.arr[cd];
                posbest->Longitude=inputPoint->arr[1-cd];
            }
            if( equirect_dist(posbest,inputPoint) <arraymax )//if bestpossible better than bestdist, do other side
            {
                kdnn(root->left,inputPoint,depth+1,best_dist,closest,knearest);    
            }
        }
        
    }
    
}


struct Point* findkclosest(struct Point *points, struct Point* inputPoint, int k, double* distances)
{
  double dists[10];
  double localdist;
  for(int i=0; i<k;i++)
  {
    dists[i]=MAX;
  }
  
  Point* closests = new Point[10];
  double arraymax;
  int imax;

    for(int i=0;i<SIZE;i++)//iterate trough all points, replacing when you see a closer one
    {
  
       arraymax=maxval(dists,k);
       imax=indexmax(dists,k);
       localdist = equirect_dist_rad(points[i].radLat,inputPoint->radLat,points[i].radLon,inputPoint->radLon);
      if(localdist<arraymax)
      {
        closests[imax]=points[i];
        dists[imax]=localdist;
      } 
    }
  
  for(int i=0;i<k;i++)//Fill the final distances into the input spot "distances"
  {
    distances[i]=dists[i];
  }
  return closests;
}


string consensus(string* states, int k)
{
    
    for (int i=0;i<k/2;i++)
    {
        int count=1;
        for(int j=i+1;j<k;j++)
        {
            if(states[j]==states[i])
            {
                count++;
            }
            
        }
        if(count>k/2)
        {
            return states[i];
        }
    }
    
    return "{no consensus}";
}


struct timespec diff(struct timespec start, struct timespec end)
{
  struct timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}


double equirect_dist_rad(double phi1, double phi2,double lam1,double lam2) //Equirectangular distance between 2 points, taking input already as radiants to make it faster.
{
  
  double x = (lam2-lam1) * cos((phi1+phi2)/2);
  double y = (phi2-phi1);
  double d = sqrt(x*x + y*y) * R;
  
  return d;
}

Node* insert(Node *root, struct Point* point) 
{ 
    return insertRec(root, point, 0); 
}

Node *insertRec(Node *root, struct Point* point, unsigned depth) 
{ 
    if (root == NULL) // Tree is empty? 
       return newNode(point); 
      
    unsigned cd = depth % kdim; //current dimension (cd) of comparison 
    if (point->arr[cd] < (root->pt.arr[cd]))  // Compare new point with root on current dimension and decide the left or right
        root->left  = insertRec(root->left, point, depth + 1); 
    else
        root->right = insertRec(root->right, point, depth + 1); 
  
    return root; 
}

double equirect_dist(struct Point* a, struct Point* b) //Equirectangular distance between 2 points
{
  double phi1 = (M_PI)/180 * a->Latitude;
  double phi2 = (M_PI)/180 * b->Latitude;
  double lam1 = (M_PI)/180 * a->Longitude;
  double lam2 = (M_PI)/180 * b->Longitude; 
  
  double x = (lam2-lam1) * cos((phi1+phi2)/2);
  double y = (phi2-phi1);
  double d = sqrt(x*x + y*y) * R;
  
  return d;
}



double maxval(double *dists,int k)
{
  double max =0;
  for (int i=0;i<k;i++)
  {
    if(dists[i]>max)
    {
      max=dists[i];
    }
  }

  return max;
}


int indexmax(double *dists,int k)
{
  
  double max =0;
  for (int i=0;i<k;i++)
  {
    if(dists[i]>max)
    {
      max=dists[i];
    }
  }
  int maxfound;
  for (int i=0;i<k;i++)
  {
    if(dists[i]==max)
    {
      maxfound =i;
      return maxfound;
    }
  }
}

void vote5(string* States,string* Counties,double* best_dist, int k)//called if k = 5 or more 
{
  string st;//final state
  string co;//final county
  string fivest[5];
  string fiveco[5];
  if(k>5)//if k>5, need to get 5 closests
  {
    vector< pair <double,string> > vcounties;
    for (int i=0; i<k; i++)vcounties.push_back( make_pair(best_dist[i],Counties[i]) );
    sort(vcounties.begin(),vcounties.end());
    vcounties.resize(5);//truncates to 5
    
    vector< pair <double,string> > vstates;
    for (int i=0; i<k; i++)vstates.push_back( make_pair(best_dist[i],States[i]) );
    sort(vstates.begin(),vstates.end());
    vstates.resize(5);//truncates to 5
    
    for(int i = 0; i < vcounties.size(); i++)
    {
       fivest[i] = vstates[i].second;
       fiveco[i] = vcounties[i].second;
       st = consensus(fivest,5);
       co = consensus(fiveco,5);
       
    }
    
  }
  
  else//if k=5, no need to get the 5 largest.
  {
    st = consensus(States,5);
    co = consensus(Counties,5);
  }
  
  cout<<"By consensus of 5 closest points, State is "<<st<<" and county is "<<co<<endl<<endl<<endl;
}


/*struct Point* findclosest(struct Point *points, struct Point* inputPoint)
{
   Point *closest = new Point;
   closest->Name="CLOSEST";
   closest->State="CL";
   
   double dist = MAX;
   double localdist;
   
   for (int i=0; i<SIZE; i++)
   {
     localdist = equirect_dist_rad(points[i].radLat,inputPoint->radLat,points[i].radLon,inputPoint->radLon);
     if(localdist < dist)
     {
       closest = &points[i];
       dist = localdist;
     }  
   }
  
  return closest;
}*/


/*void kdnn(struct Node *root, struct Point* inputPoint, unsigned depth, double* best_dist, struct Point* closest) //only for k=1
{
    if(root==NULL)
        return;
        
    unsigned cd = depth%kdim;
    double dist = equirect_dist(&(root->pt),inputPoint);
    if(dist<*best_dist)
    {
        *best_dist=dist;
        *closest=root->pt;
    }
    //which side is best:
    if(inputPoint->arr[cd]<root->pt.arr[cd])//left is best
    {
        kdnn(root->left,inputPoint,depth+1,best_dist,closest);
        if(root->right!=NULL)
        {
            Point *posbest = new Point;//possible best point of that side 
            if(cd)
            {
                posbest->Latitude=inputPoint->arr[1-cd];
                posbest->Longitude=root->right->pt.arr[cd];
            }
            else
            {
                posbest->Latitude=root->right->pt.arr[cd];
                posbest->Longitude=inputPoint->arr[1-cd];
            }
            if( equirect_dist(posbest,inputPoint) <*best_dist )//if bestpossible better than bestdist, do other side
            {
                kdnn(root->right,inputPoint,depth+1,best_dist,closest);    
            }
        }
    }
    else
    {
        kdnn(root->right,inputPoint,depth+1,best_dist,closest);
        if(root->left !=NULL)
        {
            Point *posbest = new Point;//possible best point of that side 
            if(cd)
            {
                posbest->Latitude=inputPoint->arr[1-cd];
                posbest->Longitude=root->left->pt.arr[cd];
            }
            else
            {
                posbest->Latitude=root->left->pt.arr[cd];
                posbest->Longitude=inputPoint->arr[1-cd];
            }
            if( equirect_dist(posbest,inputPoint) <*best_dist )//if bestpossible better than bestdist, do other side
            {
                kdnn(root->left,inputPoint,depth+1,best_dist,closest);    
            }
        }
        
    }
    
}*/


/*void kdnn(struct Node *root, struct Point* inputPoint, unsigned depth, double* best_dist, struct Point* closest) //had problem were looked at too many points by approximaation
{
    if(root==NULL)
        return;
        
    unsigned cd = depth%k;
    //double dist = equirect_dist(&(root->pt),inputPoint);
    double dist = equirect_dist_rad(root->pt.radLat,inputPoint->radLat,root->pt.radLon,inputPoint->radLon);
                //equirect_dist_rad(points[i].radLat,inputPoint->radLat,points[i].radLon,inputPoint->radLon);
    if(dist<*best_dist)
    {
        *best_dist=dist;
        *closest=root->pt;
    }
    //which side is best:
    if(inputPoint->arr[cd]<root->pt.arr[cd])//left is best
    {
        kdnn(root->left,inputPoint,depth+1,best_dist,closest);
        if(root->right!=NULL)
        {
            if(  abs(root->right->pt.arr[cd]-inputPoint->arr[cd])<*best_dist )//if bestpossible better than bestdist, do other side
            {
                kdnn(root->right,inputPoint,depth+1,best_dist,closest);    
            }
        }
    }
    else
    {
        kdnn(root->right,inputPoint,depth+1,best_dist,closest);
        if(root->left !=NULL)
        {
            if(  abs(root->left->pt.arr[cd]-inputPoint->arr[cd])<*best_dist )//if bestpossible better than bestdist, do other side
            {
                kdnn(root->left,inputPoint,depth+1,best_dist,closest);    
            }
        }   
    }   
}*/