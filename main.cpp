#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;
/*
    Task List:
    TODO: 1) Read data from file "customs.in"
                read file by variables
    TODO: 2) Output results to "customs.out"
    TODO: 3) Muitnieki: count [1...99]; 
                Control time [1...100000];
                (Non)Citizen lines
    TODO: 4) Travelers: Total count [0...4000000];
                 Type{P, N};
                 ID/ Arrival time [1...4000000], unique;
    TODO: 5) Incoming data format from file:
                Line 1: P_Muitnieki_count N_Muitnieki_count P_Default_Control_time N_Default_Control_time;
                Line [2... P_Muitnieki_count + N_Muitnieki_count](can be omitted):
                    T Type Muitnieks_ID Control_time;
                Lines after that: Traveler_Type Traveler_ID
                Line last: X
    TODO: 6) Output format:
                Traveler_ID Leaving_time[1...4000000000]
                If there are no travelers, output: "nothing"
                Sorted by Leaving_time, 
                    if travelers have matching Leaving_time,
                    prioritize Citizens over Non-Citizens,
                    same type travelers are sorted by relevant Muitnieks_ID in ascending order
*/

// class for Muitnieki
    // is it needed if the only value is control time?, is it?
    // need to know if Muitnieks is with traveler or not
struct Muitnieks
{
    int id;
    int control_time;
    bool is_busy = false;
    Muitnieks(int i, int c): id(i), control_time(c) {};
    Muitnieks() {};

    // TODO: if Muitnieks is with travler, set is_busy to true
        // when traveler leaves, set is_busy to false
};

// class for Travelers
struct Traveler
{
    char type;
    int id;
    int muitnieks_id;
    int leaving_time = 0;
    Traveler(char t, int i): type(t), id(i) {};

    // TODO: function to set leaving time
        // leaving_time = id + Muitnieki_P/N[i].control_time + time_in_buffer

};

// buffer

// time ticker might be needed, 
    // probably should advance time by traveler_id

// function to sort travelers

int main()
{
    // cout << "Current working directory: " << fs::current_path() << endl;
    // open file to read
    // ifstream fin("customs.in");
    ifstream fin("C:\\Users\\marti\\Projects\\Algoritmi\\Muita\\input\\customs.i5");
    if (!fin)
    {
        cout << "File not found!" << endl;
        return 1;
    }
    
    // read first line from file to get Muitnieki_counts and their default control times
    int P_Muitnieki_count, N_Muitnieki_count, P_Default_Control_time, N_Default_Control_time;
    fin >> P_Muitnieki_count >> N_Muitnieki_count >> P_Default_Control_time >> N_Default_Control_time;

    // test reading first line to int variables
    cout << P_Muitnieki_count << " " << N_Muitnieki_count << " " << P_Default_Control_time << " " << N_Default_Control_time << endl;

    // initialize arrays for Muitnieki; Muitnieki_count + 1 because start count from 1 not 0
    Muitnieks P_Muitnieki[P_Muitnieki_count + 1] = {};
    Muitnieks N_Muitnieki[N_Muitnieki_count + 1] = {};
    // fill initial values for Muitnieki
    cout << "P array" << endl;
    for ( int i = 1 ; i <= P_Muitnieki_count; i++)
    {
        P_Muitnieki[i] = Muitnieks(i, P_Default_Control_time);
        // test initialization
        
        cout << P_Muitnieki[i].id << " " << P_Muitnieki[i].control_time << endl;
    }
    cout << "N array" << endl;
    for ( int i =1 ; i <= N_Muitnieki_count; i++)
    {
        N_Muitnieki[i] = Muitnieks(i, N_Default_Control_time);
        // test initialization
        
        cout << N_Muitnieki[i].id << " " << N_Muitnieki[i].control_time << endl;
    }
    // read custom control times for muitnieki, if any
    char t, m_type, t_type;
    int m_id, custom_time, t_id;
    // DONE: doesn't transition from T to t_type read, need to fix
    fin >> t;
        
    while (t == 'T')
    {
        fin >> m_type >> m_id >> custom_time;
        // make neccessary adjustments to control times for muitnieki
        m_type == 'P' ? P_Muitnieki[m_id].control_time = custom_time : N_Muitnieki[m_id].control_time = custom_time;
        cout << t << " " << m_type << " " << m_id << " " << custom_time << endl;
        fin >> t;
    }
    if (t == 'X') cout << "nothing" << endl;
    while (t == 'P' || t == 'N')
    {
        t_type = t;
        fin >> t_id;
        cout << t_type << " " << t_id << endl;
        fin >> t;
        if (t == 'X') break;
    }
    

    // test muitnieki array contents
    cout << "P array" << endl;
    for ( int i =1 ; i <= P_Muitnieki_count; i++)
    {
        cout << P_Muitnieki[i].id << " " << P_Muitnieki[i].control_time << endl;
    }
    cout << "N array" << endl;
    for ( int i =1 ; i <= N_Muitnieki_count; i++)
    {
        cout << N_Muitnieki[i].id << " " << N_Muitnieki[i].control_time << endl;
    }

    // read and sort travelers

    // buffer for travelers waiting for muitnieki to free up


    // write to file


    // close files
    fin.close();
    return 0;
}