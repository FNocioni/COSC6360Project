#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <vector>
#include <string>
#include <fstream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <math.h>
#include <ctime>
#include <map>
#include <iterator>

#define MAX_DATABASES 10

using namespace std;

struct Database{
    map<string, int> ids;
};

struct ReplicaManager{
    Database db;
    vector<string> queryPool;
    bool isFaulty = false;

    int findValue(string key){
        auto it = db.ids.find(key);
        return it->second;
    }

    void setValue(string key, int val){
        auto it = db.ids.find(key);
        if(it != db.ids.end()){
            it->second = val;
        }
    }
};


struct Shepherd{
    int activeReplicas = 0;
    int faults;
    int primaryID = 0;
    bool done;

    ReplicaManager replica[MAX_DATABASES];

    int barrier = 0;

    //Coordinator Functions
    void initDB(map<string, int> starterDB){
        for(int i = 0; i < activeReplicas; i++){
            replica[i].db.ids = starterDB;
        }
    }


    //Receiving queries
    void SearchQuery(string var){        
        int val = replica[primaryID].findValue(var);
        cout << "Primary found " << var << " with value: " << val << endl;
        for(int i = 0; i < activeReplicas; i++){
            if(i == primaryID){
                continue;
            }
            val = replica[i].findValue(var);
            cout << "Replica " << i << " found " << var << " with value: " << val << endl;
        }
        done = true;
        cout << "COMMIT" << endl;
    }

    void SetQuery(string var, int val){
        replica[primaryID].setValue(var, val);
        cout << "Primary updated " << var << " with value: " << val << endl;
        for(int i = 0; i < activeReplicas; i++){
            if(i == primaryID){
                continue;
            }
            val = replica[i].findValue(var);
            cout << "Replica " << i << " updated " << var << " with value: " << val << endl;
        }
        done = true;
        cout << "COMMIT" << endl;
    }

    void printReplicaStates(){
        cout << "===== Replica States =====" << endl;
        cout << "Replica: " << primaryID << " PRIMARY" << endl;
        for(int i = 0; i < activeReplicas; i++){
            if(i == primaryID){
                continue;
            }
            cout << "Replica: " << i << " ACTIVE" << endl;
        }
    }


};

//Shared Memory
static Shepherd *shephard;

void outputDatabase(map<string, int> db){
    cout << "===== DATABASE OUTPUT =====" << endl;
    for(map<string, int>::iterator itr = db.begin(); itr != db.end(); itr++){
        cout << itr->first;
        for(int i = 0; i < 18-itr->first.length(); i++){
            cout << " ";
        }
        cout << itr->second << endl;
    }
}

int main(int argc, char** argv){
    cout << "##### Commit Barrier Scheduler Simulator #####" << endl << endl;
    shephard = (Shepherd *)mmap(NULL, sizeof *shephard, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);       
    
    int totalDatabases = 3;
    shephard->activeReplicas = totalDatabases;
    shephard->faults = floor((totalDatabases-1)/2);   //2f+1 tolerated faults.
    shephard->done = true;

    map<string, int> starterDB;
    starterDB.insert(pair<string, int>("Employees", 310));
    starterDB.insert(pair<string, int>("Facilities", 7));
    starterDB.insert(pair<string, int>("Customers", 54209));
    starterDB.insert(pair<string, int>("Budget", 23946152));
    starterDB.insert(pair<string, int>("Salary", 65000));
    starterDB.insert(pair<string, int>("Expenses", 942810));
    starterDB.insert(pair<string, int>("Quarterly_Profit", 9188134));
    starterDB.insert(pair<string, int>("Annual_Earnings", 23786731));        
    shephard->initDB(starterDB);
    shephard->printReplicaStates();
    outputDatabase(starterDB);

    string s = "";
    while(true){
        cout << "\nBARRIER VALUE: " << shephard->barrier << endl;
        cout << "Input: ";
        getline(cin, s);
        if(s == "end" || s == "End" || s == "END"){
            cout << "Exiting Program" << endl;
            break;
        }
        stringstream ss(s);
        string s1, s2;
        int s3;
        ss >> s1 >> s2;
        if(s1 == "search"){
            shephard->done = false;
            shephard->SearchQuery(s2);            
        }

        if(s1 == "set"){
            shephard->done = false;
            ss >> s3;            
            shephard->SetQuery(s2, s3);            
            
        }
        while(!shephard->done) {
            cout << shephard->done << endl;
        } 
        cout << endl;
        outputDatabase(shephard->replica[shephard->primaryID].db.ids);
        cout << endl;
        shephard->barrier++;
        cout << s << endl;
    }



    return 0;

}