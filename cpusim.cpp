#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <deque>
#include <unordered_set>
#include <string>
#include <algorithm>

using namespace std;

struct Task { 
    string id;  //task id
    int burst;  //total cpu time cyc req by tasks
    int r;    //remaining burst time req to complete  
    vector<string> mem; //list of memory addresses
    int mp=0; //mem pointer 
    int at; //arrival time of task

    Task(string i, int b, vector<string> m, int a): id(i), burst(b), r(b), mem(m), at(a) {}
}; //constructor


class cache {  //fifo caache block
    size_t cap;  //capacity= no. of block cache can hold
    deque<string> q; //double ended queue to handle evictions
    unordered_set<string> s;  //to check if a system is already inside a cache

public:
    cache(size_t c) : cap(c) {} 

    bool has(const string& x) const {  //returns true if mem element x exists in the hash set(cache hit), else false(cache miss)
        return s.count(x);
    }

    string add(const string& x) {
        if (s.count(x)) 
        return ""; //if x exists; returns empty string

        string ev = "";
        if (q.size() >= cap) { //if cache is full
            ev = q.front();  //store the oldest element as ev
            q.pop_front();  //evict the oldest element
            s.erase(ev);  //removes s and saves as ev
        }
        q.push_back(x);  //pushes the new element at the back of queue
        s.insert(x);  //register it in hash set
        return ev;  //returns evicted item



    }
void remove(const string& x) { //removes memory block x from cache
    if (!s.count(x))
        return; //return if block not present
    s.erase(x); //remove block from hash set
    for (auto it = q.begin(); it != q.end(); ++it) {
        if (*it == x) {
            q.erase(it); //remove block from fifo queue
            break;
        }
    }
}
    void print(const string& name) const {  //prints the current contents of cache  
     cout <<"  "<< name<<": [";
    for (int i = 0; i < (int)q.size(); i++) {
        cout << q[i];
        if (i + 1 < (int)q.size()) cout << ", ";
        }
        cout << "]";
    }
};



class mems {    //allocates capacity of l1, l2,l3
public:
    cache l1,l2,l3;  //3 level cache sys
    int ram=0;  //ram misses
    int lat=0; //total latency
mems():l1(32),l2(128),l3(512){}  //constructor initializing cache levels with their capacity
void seed(vector<string> a, vector<string> b, vector<string> c) {  //inserting initial data of l1,l2,l3; this data is available in repective levels before execution
        for(auto&x:a)l1.add(x);
        for(auto &x:b)l2.add(x);
        for(auto&x:c)l3.add(x);
    }
void acc(const string& x) { //simulating a mem access request for x
    if (l1.has(x)) {  //if data is found in l1=fastest access
        cout << " -> HIT L1";
        lat += 4; //lowest latency for l1 hit
        return;
    }
    cout << " >> MISS L1";

    if (l2.has(x)) {  //if not in l1 but found in l2
        cout << " >> Hit L2";
        lat += 12; //latency for l2 hit
        return;
    }
    cout << " >> MISS L2";
    if (l3.has(x)) {
    cout << " >> Hit L3";
    lat += 40; //latency for l3 hit
    l3.remove(x); //remove data from l3
    l2.add(x); //move data from l3 to l2
    string ev = l1.add(x); //move data from l2 to l1
    l2.remove(x); //remove data from l2 after promoting to l1
    if (!ev.empty())
        cout << " (evicted " << ev << ")"; //if l1 full; oldest data is evicted
    return;
}

    cout << " >> Miss L3 >> RAM"; //if not found in cache go to ram
ram++; //increase ram access count
lat += 200; //highest latency for accessing ram
l3.add(x); //copy data from ram to l3
l2.add(x); //copy data from l3 to l2

string ev = l1.add(x); //copy data from l2 to l1
l3.remove(x); //remove data from l3 after promotion
l2.remove(x); //remove data from l2 after promotion

if (!ev.empty())
    cout << " (evicted " << ev << ")"; //check if l1 eviction possible
}
    void show() const { //displaying current contents of all cache levels
        l1.print("L1"); 
        cout<<endl;
        l2.print("L2"); 
        cout << endl;
        l3.print("L3"); 
        cout <<endl;
    }
};

int main() {
     ifstream fin("input.txt");  //opening the input file
    if (!fin) {   
        cout << "missing input.txt\n"; 
        return 1;
    }

    vector<Task> t; 
    string line; //hold 1 line from input.txt
    int at = 0;  //arrival time assigned to tasks

    while (getline(fin, line)) {  //reading input file line by line
        if (line.empty()) //skip line if it is empty
        continue;

        stringstream ss(line); //create stream to parse current line
        string tok, id;  // tok = current word, id = task ID
        int b=0;  //burst time
        vector<string> m;  //memory refferences for tasks

        while (ss>>tok) { //parse tokens(tok) from line
            if (tok=="TASK"||tok=="task") { 
                ss>>id; //read task id
            }
            else if (tok=="BURST"||tok=="burst") {
                ss>>b;  //read burst time
            }
            else if (tok=="MEM"||tok=="mem") {
                string x;
                while (ss>>x)  //read all mem blocks after mem
                 m.push_back(x);
            }
        }

        if (!id.empty() && b>0)  //creating task object if valid data found
            t.push_back(Task(id,b,m,at++));
    }

    if (t.empty()) {  //if no tasks found
        cout<<"No tasks found"<<endl;
        return 1;
    }

mems mem;

mem.seed(  //preloading data in caches
        {"M2", "M5", "M1"}, {"M3", "M7"}, {"M6"});

    queue<int> q;  //queue for implementing Round Robin
    vector<bool> in(t.size(), false); //track tasks added to the queue
    int clk=1;
    int qt=3;  // Round Robin time quantum
    int done=0; //no. of tasks completed

    while (done<(int)t.size()) { 
    for (int i = 0; i < (int)t.size(); i++) { //adding new tasks to the queue
            if (!in[i] && t[i].at <= clk - 1) {
                q.push(i);
                in[i] = true;
            }
        }

        if (q.empty()) {  //if no task is ready cpu is kept waiting
            clk++;
            continue;
        }
int i = q.front(); //getting next task index
q.pop();

Task &cur = t[i];  //reference to current task
int slice = min(qt, cur.r);  //task runs for quantum or remaining burst time 

        for (int s = 0; s < slice; s++) {
            cout << "Cycle: "<<clk<<" -"<<"Running:"<<cur.id<<endl;

            if (!cur.mem.empty()) {
                string b = cur.mem[cur.mp % cur.mem.size()]; //accessing mem blocks in cyclic order
                cout <<"Requesting"<< b;
                mem.acc(b);  
                cur.mp++;  //move to next mem reference
            } else {
                cout <<"requesting none"<<endl;
            }

            cout <<endl;
            mem.show();
            cout <<"\n";  //displaying cache contents
            cur.r--;
            clk++;
            if (cur.r==0)  //tasks if finished execution
            {
                done++;
                break;
            }

            for(int j=0;j<(int)t.size();j++) {  //tasks tht arrive during execution
                if (!in[j]&& t[j].at<=clk-1) { //adding new arrived tasks to the queueu
                    q.push(j);
                    in[j]=true;
                }
            }
        }
        if (cur.r>0) //adding the unfinised tasks back into queue
            q.push(i);
    }
    cout<<"=== FINAL RESULTS ==="<<endl;
    cout<<"Total Cycles: "<<clk-1<<endl;
    cout<<"Latency: "<<mem.lat<<endl;
    cout<<"Tasks: "<<done<<endl;
    cout<<"Scheduler: Round Robin"<<endl;
    cout<<"Quantum: "<<qt<<endl;
    cout<<"RAM accesses: "<<mem.ram<<endl;
}
