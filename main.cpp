#include <iostream>
#include <fstream>
#include <stdio.h>
//#include <filesystem>

using namespace std;
//namespace fs = std::filesystem;
/*
    Task List:
    DONE: 1) Read data from file "customs.in"
                read file by variables
    DONE: 2) Output results to "customs.out"
    DONE: 3) Muitnieki: count [1...99]; 
                Control time [1...100000];
                Citizen and non-citizen lines
    DONE: 4) Travelers: Total count [0...4000000];
                 Type{P, N};
                 ID/ Arrival time [1...4000000], unique;
    DONE: 5) Incoming data format from file:
                Line 1: P_Muitnieki_count N_Muitnieki_count P_Default_Control_time N_Default_Control_time;
                Line [2... P_Muitnieki_count + N_Muitnieki_count](can be omitted):
                    T Type Muitnieks_ID Control_time;
                Lines after that: Traveler_Type Traveler_ID
                Line last: X
    DONE: 6) Output format:
                Traveler_ID Leaving_time[1...4000000000]
                If there are no travelers, output: "nothing"
                Sorted by Leaving_time, 
                    if travelers have matching Leaving_time,
                    prioritize Citizens over Non-Citizens,
                    same type travelers are sorted by relevant Muitnieks_ID in ascending order
*/
// *********** PROTOTYPES ***********
class Buffer;
class DepartureBST;
struct Traveler;
void assignTraveler( Traveler& );
void assignOrBufferTraveler(Traveler&);

// *********** GLOBALS ***********
const unsigned int MAX_LEAVING_TIME = 4000000000;
// variables for Muitnieki
int P_Muitnieki_count, N_Muitnieki_count;
int P_Default_Control_time, N_Default_Control_time;
int* P_Muitnieki_control_time;
bool* P_Muitnieki_availability;
int* N_Muitnieki_control_time;
bool* N_Muitnieki_availability;

// initialize arays
void initializeGlobals()
{
    P_Muitnieki_control_time = new int[P_Muitnieki_count + 1];
    P_Muitnieki_availability = new bool[P_Muitnieki_count + 1];
    N_Muitnieki_control_time = new int[N_Muitnieki_count + 1];
    N_Muitnieki_availability = new bool[N_Muitnieki_count + 1];

    for (int i = 0; i <= P_Muitnieki_count; i++)
    {
        P_Muitnieki_control_time[i] = P_Default_Control_time;
        P_Muitnieki_availability[i] = true;

        // test initialization
        cout << i << " " << P_Muitnieki_control_time[i] << " " << P_Muitnieki_availability[i] << endl;
    }
    for (int i = 0; i <= N_Muitnieki_count; i++)
    {
        N_Muitnieki_control_time[i] = N_Default_Control_time;
        N_Muitnieki_availability[i] = true;

        // test initialization
        cout << i << " " << N_Muitnieki_control_time[i] << " " << N_Muitnieki_availability[i] << endl;
    }
}

// cleanup arrays
void cleanupGlobals()
{
    delete[] P_Muitnieki_control_time;
    delete[] P_Muitnieki_availability;
    delete[] N_Muitnieki_control_time;
    delete[] N_Muitnieki_availability;
}




// *********** CLASSES ***********

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

    // Done: if Muitnieks is with travler, set is_busy to true
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
// global buffers
Buffer P_buffer;
Buffer N_buffer;

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
                    TravelerNode* newTraveler = new TravelerNode(t);
                    TravelerNode** travelerList = (t.type == 'P') ? &node->P_travelers : &node->N_travelers;

                    // Insert the new traveler in the sorted position based on muitnieks_id
                    TravelerNode* current = *travelerList;
                    TravelerNode* previous = nullptr;

                    while (current != nullptr && current->traveler.muitnieks_id < t.muitnieks_id)
                    {
                        previous = current;
                        current = current->next;
                    }

                    if (previous == nullptr)
                    {
                        // Insert at the beginning of the list
                        newTraveler->next = *travelerList;
                        *travelerList = newTraveler;
                    }
                    else
                    {
                        // Insert in the middle or end of the list
                        previous->next = newTraveler;
                        newTraveler->next = current;
                    }
                    return;
                }
            }
            // if the departure time is not found, create a new node and add traveler to it
            insert(time);
            addTraveler(time, t);
        }
        // last processed departure node
        DepartureNode* last_processed_node = nullptr;

        // find departure times in range between previous(excluding) and current(including) arrival
        void processDepartuesInRange(unsigned int start_time, unsigned int end_time)
        {
            /*bool new_departures_added = true;
            while (new_departures_added)
            {*/
                //new_departures_added = false;
			cout << "Processing departures in range: " << start_time << " - " << end_time << endl;
                DepartureNode* node = (last_processed_node != nullptr) ? last_processed_node : root;

                // Traverse BST starting from the last processed node
                while (node != nullptr)
                {
                    // test output
					cout << "Checking Node with leaving time: " << node->leaving_time << endl;

                    if (node->leaving_time > start_time && node->leaving_time <= end_time)
                    {
                        // process all travelers attached to this node
						int moment_in_time = node->leaving_time;
                        // test mit
						cout << "Departures in range, MIT: " << moment_in_time << endl;
                        TravelerNode* p = node->P_travelers;
                        while (p != nullptr)
                        {
                            // set Muitnieks availability to true
                            P_Muitnieki_availability[p->traveler.muitnieks_id] = true;
                            //test output
                            std::cout << "P_Muitnieks " << p->traveler.muitnieks_id << " is free" << endl;
                            // if there are travelers in the buffer, dequeue one and assign to Muitnieks
                            if (!P_buffer.isEmpty())
                            {
                                Traveler traveler = P_buffer.dequeue(node->leaving_time);
                                assignTraveler(traveler);

                                //new_departures_added = true; // new departures added
                            }
                            // go to next traveler, if there is one
                            p = p->next;
                        }
                        TravelerNode* n = node->N_travelers;
                        while (n != nullptr)
                        {
                            // set Muitnieks availability to true
                            N_Muitnieki_availability[n->traveler.muitnieks_id] = true;
                            //test output
                            std::cout << "N_Muitnieks " << n->traveler.muitnieks_id << " is free" << endl;
                            // if there are travelers in the buffer, dequeue one and assign to Muitnieks
                            if (!N_buffer.isEmpty())
                            {
                                Traveler traveler = N_buffer.dequeue(node->leaving_time);
                                assignTraveler(traveler);

                                //new_departures_added = true; // new departures added
                                // TODO: could possibly assign traveler directly to the Muitnieks that was just freed up
                            }
                            // go to next traveler, if there is one
                            n = n->next;
                        }

                        // update last processed node
                        last_processed_node = node;
                        // test last_processed_node
                        cout << "in Departure processing, last_processed_node: " << last_processed_node->leaving_time << endl;
                    }
                    // move to the next node in the BST
                    if (node->leaving_time <= end_time)
                    {
                        node = findNextNode(node);
						if (node != nullptr && node->leaving_time > end_time)
						{
							return; // stop the function if node->leaving_time is greater than end_time
						}
                    }
                    else
                    {
                        if (node->left == last_processed_node)
                        {
                            return; // stop the function if node->left is equal to last_processed_node
                        }
                        node = node->left; // go left if the leaving time is greater than end_time
                    }
                }
            //}
        }
        // helper function to find the next node in the BST
        DepartureNode* findNextNode(DepartureNode* node) 
        {
            if (node->right != nullptr) {
                // Find the leftmost node in the right subtree
                node = node->right;
                while (node->left != nullptr) {
                    node = node->left;
                }
                return node;
            }
        
            // Traverse up the tree to find the next node
            DepartureNode* parent = root; // Start from the root
            DepartureNode* successor = nullptr;
            while (parent != nullptr) {
                if (node->leaving_time < parent->leaving_time) {
                    successor = parent;
                    parent = parent->left;
                } else if (node->leaving_time > parent->leaving_time) {
                    parent = parent->right;
                } else {
                    break;
                }
            }
            return successor;
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
//  global departure BST
DepartureBST Departures;

// *********** FUNCTIONS ***********

// helper function to find a free muitnieks for a traveler
void assignTraveler( Traveler& traveler)
{
    if( traveler.type == 'P')
    // check  if any P_Muitniki are free
    {
        for (int i = 1; i <= P_Muitnieki_count; i++)
        {
            if (P_Muitnieki_availability[i] == true)
            {
                // if free, set availability to false and set traveler Muitnieks ID
                P_Muitnieki_availability[i] = false;
                traveler.muitnieks_id = i;
                // set leaving time for traveler
                traveler.leaving_time += traveler.id + P_Muitnieki_control_time[i];
                // test output
                std::cout << "Traveler " << traveler.id << " is leaving P_Muitnieks " << traveler.muitnieks_id << " at " << traveler.leaving_time << endl;
                Departures.addTraveler(traveler.leaving_time, traveler); // add traveler to departure BST
                return;
            }
        }
    }
    else if (traveler.type == 'N')
    {
        // check if any N_Muitnieki are free
        for (int i = 1; i <= N_Muitnieki_count; i++)
        {
            if (N_Muitnieki_availability[i] == true)
            {
                // if free, set availability to false and set traveler Muitnieks ID
                N_Muitnieki_availability[i] = false;
                traveler.muitnieks_id = i;
                // set leaving time for traveler
                traveler.leaving_time += traveler.id + N_Muitnieki_control_time[i];
                // test output
                cout << "Traveler " << traveler.id << " is leaving N_Muitnieks " << traveler.muitnieks_id << " at " << traveler.leaving_time << endl;
                Departures.addTraveler(traveler.leaving_time, traveler); // add traveler to departure BST
                return;
            }
        }
    }
}

// helper function to buffer a traveler
void bufferTraveler( Traveler& traveler)
{
    if (traveler.type == 'P')
    {
        P_buffer.enqueue(traveler);
        // test output
        cout << "Traveler " << traveler.id << " is waiting in P_buffer" << endl;
    }
    else if (traveler.type == 'N')
    {
        N_buffer.enqueue(traveler);
        // test output
        cout << "Traveler " << traveler.id << " is waiting in N_buffer" << endl;
    }
}

// function to assign traveler to Muitnieks or buffer it
void assignOrBufferTraveler( Traveler& traveler)
{
    assignTraveler(traveler);
    if (traveler.muitnieks_id == -1)
    {
        // if no Muitnieks is free, send traveler to buffer
        bufferTraveler(traveler);
    }
}



// time ticker might be needed, 
    // probably should advance time by some combination of traveler arrival time and leaving time
    // or just advance time by 1, and check if any travelers are waiting for Muitnieks

int main()
{
    // cout << "Current working directory: " << fs::current_path() << endl;
    // open file to read
	ifstream fin("customs.in");
    //ifstream fin("C:\\Users\\User\\Desktop\\Studijas LU\\DSunAlgo\\Muita\\input\\customs.i7");
    // ifstream fin("C:\\Users\\marti\\Projects\\Algoritmi\\Muita\\input\\customs.i5");
    if (!fin)
    {
        cout << "File not found!" << endl;
        return 1;
    }
    
    // open file to write
    //ofstream fout("C:\\Users\\User\\Desktop\\Studijas LU\\DSunAlgo\\Muita\\customs.out");
	ofstream fout("customs.out");
    // read first line from file to get Muitnieki_counts and their default control times
    fin >> P_Muitnieki_count >> N_Muitnieki_count >> P_Default_Control_time >> N_Default_Control_time;

    // test reading first line to int variables
    cout << P_Muitnieki_count << " " << N_Muitnieki_count << " " << P_Default_Control_time << " " << N_Default_Control_time << endl;

    // initialize globals
    initializeGlobals();

    // time control variables 
    unsigned int previous_time = 0, current_time = 0;

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
    if (t == 'X') 
    {
        cout << "nothing" << endl;
        fout << "nothing" << endl;
        // cleanup globals
        cleanupGlobals();
        return 0;
    }
    // read and sort travelers
    while (t == 'P' || t == 'N')
    {
        t_type = t;
        fin >> t_id;
        cout << t_type << " " << t_id << endl;

        // create traveler object
        Traveler traveler(t_type, t_id);

        // set current time to traveler arrival time
        current_time = t_id;

        // test time beforre depatrure processing during arrivals
		cout << "While PN time check: Current_time: " << current_time << " Previous_time: " << previous_time << endl;
        // check if any depatures are in range between previous and current time
		// test last processed node
		(Departures.last_processed_node != nullptr) ? cout << "Last processed node: " << Departures.last_processed_node->leaving_time << endl : cout << "Last processed node is null" << endl;
        Departures.processDepartuesInRange(previous_time, current_time);

        // assign traveler to Muitnieks or buffer
        assignOrBufferTraveler(traveler);

        // update previous time
        previous_time = current_time;

        // start reading next line
        fin >> t;
        if (t == 'X') break; // end of file, won't change current_time
    }
    
    // ********** PROCESS TRAVELERS REMAINING IN BUFFERS **********
    // previous_time == current_time should be true
    // could set currrent_time to max possible leaving time
    // problem might be calculating waiting time for travelers in buffer
    // possible solution: use previous_time as current time, while current_time sets the max range for departures 
    // check if there are any travelers left in the buffers
    // if there are, process them
	
    
    if (!P_buffer.isEmpty() || !N_buffer.isEmpty())
    {
		// test entry to if statement
        cout << "Processing travelers in buffers, after arrivals have ended." << endl;
        if (Departures.last_processed_node != nullptr)
            previous_time = Departures.last_processed_node->leaving_time;
		// test previous_time
		cout << "Previous time: " << previous_time << endl;
        // check if any depatures are in range between previous and current time
        Departures.processDepartuesInRange(previous_time, MAX_LEAVING_TIME);
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
    fprintf(stderr, "Viss %i\n", 0);
    // buffer for travelers waiting for muitnieki to free up


    // cleanup globals
    cleanupGlobals();

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
