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
                Citizen and non-citizen lines
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
struct Muitnieks // currently not used
{
    int id;
    int control_time;
    bool is_busy = false;
    Muitnieks(int i, int c): id(i), control_time(c) {};
    Muitnieks() {};

    // TODO: if Muitnieks is with travler, set is_busy to true
        // when traveler leaves, set is_busy to false
        // could also make an array for Muitnieki to show availability
};

// class for Travelers
struct Traveler
{
    char type;
    int id;
    int muitnieks_id = -1;
    unsigned int leaving_time = 0;
    Traveler(char t, int i): type(t), id(i) {};

    // TODO: function to set leaving time
        // leaving_time = id + Muitnieki_P/N[i].control_time + time_in_buffer

};

// buffer for each type of Muitnieks
    // buffers could be implemented as queues, since traveler count is unknown while reading the file
    // need to keep track of time spent in buffer for each traveler
    // could split travelers into two queues, and if the queue is empty and a Muitnieks is free, then waiting time is 0
    // if queue is not empty, then waiting time could be calculated as the time of leaving( also the time when muitnieks gets freed up) minus time of arrival
struct TravelerNode
{
    Traveler traveler;
    int waiting_time = 0;
    TravelerNode* next = nullptr;
    TravelerNode(Traveler t): traveler(t) {};
};

class Buffer 
{
    private:
        TravelerNode* front;
        TravelerNode* rear;

    public:
        // constructor for buffer
        Buffer() : front(nullptr), rear(nullptr) {};

        // check if buffer is empty
        bool isEmpty()
        {
            return front == nullptr;
        }

        // add traveler to buffer
        void enqueue( Traveler t)
        {
            TravelerNode* newNode = new TravelerNode(t);
            if (isEmpty())
            {
                // if buffer is empty, set front and rear to new node
                front = rear = newNode;
            }
            else
            {
                // Add new node to the end of the buffer and update rear
                rear->next = newNode;
                rear = newNode;
            }
        }

        // remove traveler from buffer
        Traveler dequeue( int moment_in_time)
        {
            if (isEmpty())
            {
                cout << "Buffer is empty!" << endl;
                throw runtime_error("Buffer is empty!");
            }
            front->waiting_time = moment_in_time - front->traveler.id; // Calculate waiting time for the traveler
            TravelerNode* temp = front;
            Traveler t = front->traveler; // Get the traveler from the front node
            t.leaving_time += front->waiting_time; // Update leaving time with waiting time

            // Move front to the next node
            front = front->next;
            
            delete temp; // Free the memory of the old front node

            if (front == nullptr)
            {
                rear = nullptr; // Buffer is now empty
            }
            return t; // Return the traveler
        }

        // peek at the front traveler in the buffer
        Traveler peek()
        {
            if (isEmpty())
            {
                cout << "Buffer is empty!" << endl;
                throw runtime_error("Buffer is empty!");
            }
            return front->traveler; // Return the traveler at the front of the buffer
        }

        // Destructor to free memory
        ~Buffer()
        {
            while (!isEmpty())
            {
                TravelerNode* temp = front;
                front = front->next;
                delete temp; // Free the memory of the node
            }
        }
};

// departure management( BST)
struct DepartureNode
{
    unsigned int leaving_time;
    TravelerNode* P_travelers;
    TravelerNode* N_travelers;
    DepartureNode* left;
    DepartureNode* right;

    DepartureNode(unsigned int t): leaving_time(t), P_travelers(nullptr), N_travelers(nullptr), left(nullptr), right(nullptr) {};
};

class DepartureBST
{
    private:
        DepartureNode* root;

        // helper function to insert a new deparure time
        DepartureNode* insert( DepartureNode* node, unsigned int time)
        {
            if (node == nullptr)
            {
                return new DepartureNode(time);
            }
            if (time < node->leaving_time)
            {
                node->left = insert(node->left, time);
            }
            else if (time > node->leaving_time)
            {
                node->right = insert(node->right, time);
            }
            return node; // return the unchanged node pointer
        }

        // helper function for in-order traversal
        void inOrderTraversal( DepartureNode* node, ofstream& fout)
        {
            if (node == nullptr) return;
            inOrderTraversal(node->left, fout);

            // output travelers in the order of leaving time
            TravelerNode* p = node->P_travelers;
            while (p != nullptr)
            {
                fout << p->traveler.id << " " << node->leaving_time << endl;
                p = p->next;
            }
            TravelerNode* n = node->N_travelers;
            while (n != nullptr)
            {
                fout << n->traveler.id << " " << node->leaving_time << endl;
                n = n->next;
            }

            inOrderTraversal(node->right, fout);
        }

        // helper function to destroy the BST
        void destroy( DepartureNode* node)
        {
            if (node == nullptr) return;

            destroy(node->left);
            destroy(node->right);

            // free linked lists
            TravelerNode* p = node->P_travelers;
            while (p != nullptr)
            {
                TravelerNode* temp = p;
                p = p->next;
                delete temp; // Free the memory of the node
            }

            TravelerNode* n = node->N_travelers;
            while (n != nullptr)
            {
                TravelerNode* temp = n;
                n = n->next;
                delete temp; // Free the memory of the node
            }
            
            delete node; // Free the memory of the node
        }
    public:
        DepartureBST() : root(nullptr) {};

        // insert departure time into BST
        void insert(unsigned int time)
        {
            root = insert(root, time);
        }

        // add a traveler to the departure time
        void addTraveler(unsigned int time, Traveler t)
        {
            DepartureNode* node = root;
            while (node != nullptr)
            {
                if (time < node->leaving_time)
                {
                    node = node->left;
                }
                else if (time > node->leaving_time)
                {
                    node = node->right;
                }
                else
                {
                    // found the departure time, add traveler to the list
                    // TODO: travelers must be sorted by muitnieks ID
                    if (t.type == 'P')
                    {
                        TravelerNode* newTraveler = new TravelerNode(t);
                        newTraveler->next = node->P_travelers;
                        node->P_travelers = newTraveler;
                    }
                    else if (t.type == 'N')
                    {
                        TravelerNode* newTraveler = new TravelerNode(t);
                        newTraveler->next = node->N_travelers;
                        node->N_travelers = newTraveler;
                    }
                    return;
                }
            }
            // if the departure time is not found, create a new node and add traveler to it
            insert(time);
            addTraveler(time, t);
        }

        // write all departures to file
        void writeToFile(ofstream& fout)
        {
            inOrderTraversal(root, fout);
        }

        // destructor to free memory      
        ~DepartureBST()
        {
            destroy(root); // Free the memory of the BST
        }
};

// time ticker might be needed, 
    // probably should advance time by some combination of traveler arrival time and leaving time
    // or just advance time by 1, and check if any travelers are waiting for Muitnieks

int main()
{
    // cout << "Current working directory: " << fs::current_path() << endl;
    // open file to read
    ifstream fin("C:\\Users\\User\\Desktop\\Studijas LU\\DSunAlgo\\Muita\\input\\customs.i6");
    // ifstream fin("C:\\Users\\marti\\Projects\\Algoritmi\\Muita\\input\\customs.i5");
    if (!fin)
    {
        cout << "File not found!" << endl;
        return 1;
    }
    
    // open file to write
    ofstream fout("C:\\Users\\User\\Desktop\\Studijas LU\\DSunAlgo\\Muita\\customs.out");

    // read first line from file to get Muitnieki_counts and their default control times
    int P_Muitnieki_count, N_Muitnieki_count, P_Default_Control_time, N_Default_Control_time;
    fin >> P_Muitnieki_count >> N_Muitnieki_count >> P_Default_Control_time >> N_Default_Control_time;

    // test reading first line to int variables
    cout << P_Muitnieki_count << " " << N_Muitnieki_count << " " << P_Default_Control_time << " " << N_Default_Control_time << endl;

    // initialize arrays for Muitnieki control times and availability
        //  Muitnieki_count + 1 because start count from 1 not 0
    int P_Muitnieki_control_time[P_Muitnieki_count + 1] = {};
    bool P_Muitnieki_availability[P_Muitnieki_count + 1] = {};

    int N_Muitnieki_control_time[N_Muitnieki_count + 1] = {};
    bool N_Muitnieki_availability[N_Muitnieki_count + 1] = {};
    
    // create buffers for travelers
    Buffer P_buffer;
    Buffer N_buffer;

    // create departure BST
    DepartureBST Departures;

    // fill initial values for Muitnieki
    cout << "P array" << endl;
    for ( int i = 1 ; i <= P_Muitnieki_count; i++)
    {
        P_Muitnieki_control_time[i] = P_Default_Control_time;
        P_Muitnieki_availability[i] = true;
        
        // test initialization
        cout << i << " " << P_Muitnieki_control_time[i] << " " << P_Muitnieki_availability[i] << endl;
    }
    cout << "N array" << endl;
    for ( int i =1 ; i <= N_Muitnieki_count; i++)
    {
        N_Muitnieki_control_time[i] = N_Default_Control_time;
        N_Muitnieki_availability[i] = true;

        // test initialization        
        cout << i << " " << N_Muitnieki_control_time[i] << " " << N_Muitnieki_availability[i] << endl;
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
        m_type == 'P' ? P_Muitnieki_control_time[m_id] = custom_time : N_Muitnieki_control_time[m_id] = custom_time;
        cout << t << " " << m_type << " " << m_id << " " << custom_time << endl;
        fin >> t;
    }
    if (t == 'X') cout << "nothing" << endl;

    // read and sort travelers
    while (t == 'P' || t == 'N')
    {
        t_type = t;
        fin >> t_id;
        cout << t_type << " " << t_id << endl;
        // create traveler object
        Traveler traveler(t_type, t_id);

        // check if traveler is citizen or non-citizen
        if (traveler.type == 'P')
        {
            // check if any of P_Muitnieki are free
            for (int i = 1; i <= P_Muitnieki_count; i++)
            {
                if (P_Muitnieki_availability[i] == true)
                {
                    // if free, set availability to false and set traveler Muitnieks ID
                    P_Muitnieki_availability[i] = false;
                    traveler.muitnieks_id = i;
                    // set leaving time for traveler
                    traveler.leaving_time += t_id + P_Muitnieki_control_time[i];
                    cout << "Traveler " << traveler.id << " is leaving P_Muitnieks " << traveler.muitnieks_id << " at " << traveler.leaving_time << endl;
                    Departures.addTraveler(traveler.leaving_time, traveler); // add traveler to departure BST
                    break;
                }
            }
            // if no Muitnieks is free, send traveler to P_buffer
            if (traveler.muitnieks_id == -1)
            {
                cout << "Traveler " << traveler.id << " is waiting in P_buffer" << endl;
                // Done: add traveler to P_buffer
                P_buffer.enqueue(traveler);
                
            }
        }
        else if (traveler.type == 'N')
        {
            for (int i = 1; i <= N_Muitnieki_count; i++)
            {
                if (N_Muitnieki_availability[i] == true)
                {
                    // if free, set availability to false and set traveler Muitnieks ID
                    N_Muitnieki_availability[i] = false;
                    traveler.muitnieks_id = i;
                    // set leaving time for traveler
                    traveler.leaving_time += t_id + N_Muitnieki_control_time[i];
                    cout << "Traveler " << traveler.id << " is leaving N_Muitnieks " << traveler.muitnieks_id << " at " << traveler.leaving_time << endl;
                    Departures.addTraveler(traveler.leaving_time, traveler); // add traveler to departure BST
                    break;
                }
            }
            // if no Muitnieks is free, send traveler to N_buffer
            if (traveler.muitnieks_id == -1)
            {
                cout << "Traveler " << traveler.id << " is waiting in N_buffer" << endl;
                // DONE: add traveler to N_buffer
                N_buffer.enqueue(traveler);
            }
        }

        fin >> t;
        if (t == 'X') break;
    }
    

    // test muitnieki array contents
    cout << "P array" << endl;
    for ( int i =1 ; i <= P_Muitnieki_count; i++)
    {
        cout << i << " " << P_Muitnieki_control_time[i] << " " << P_Muitnieki_availability[i] << endl;
    }
    cout << "N array" << endl;
    for ( int i =1 ; i <= N_Muitnieki_count; i++)
    {
        cout << i << " " << N_Muitnieki_control_time[i] << " " << N_Muitnieki_availability[i] << endl;
    }

    // test traveler buffer contents
    cout << "P buffer" << endl;
    if (P_buffer.isEmpty())
    {
        cout << "Buffer is empty!" << endl;
    }
    else
    {
        while (!P_buffer.isEmpty())
        {
            Traveler t = P_buffer.dequeue(0);
            cout << "Traveler " << t.id << " is in P_buffer" << endl;
        }
    }

    cout << "N buffer" << endl;
    if (N_buffer.isEmpty())
    {
        cout << "Buffer is empty!" << endl;
    }
    else
    {
        while (!N_buffer.isEmpty())
        {
            Traveler t = N_buffer.dequeue(0);
            cout << "Traveler " << t.id << " is in N_buffer" << endl;
        }
    }
    
    // write departures to file
    Departures.writeToFile(fout);

    // buffer for travelers waiting for muitnieki to free up


    

    // destroy buffer objects
    // P_buffer.~Buffer();
    // N_buffer.~Buffer();
    // // destroy departure BST
    // Departures.~DepartureBST();
    // close files
    fin.close();
    fout.close();
    return 0;
}