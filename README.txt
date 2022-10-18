Simulator Manual/ README file
Author: Ricardo Santos


1. Contents of this Folder:
	Folder 'Instances': contains the header files (.h files) that instantiate all generic types used throughout the program;

	Folder 'Headers': contains all header files (.h files);

	Folder 'Sources': contains all source files (.cpp files), except for the main.cpp file;

	Folder 'Networks': contains the network files, in CSV format. The first line of each file contains the number of nodes in the network. Subsequent lines contain information about the directed link u->v, with bandwidth w and delay d, in the format "u,v,w,d". Networks provided:
		- Abilene - |V| = 11, |E| = 14;
		- AS1221 - |V| = 50, |E| = 194;
		- AS1239 - |V| = 284, |E| = 1882;
		- AS1755 - |V| = 73, |E| = 292;
		- AS3259 - |V| = 113, |E| = 558;
		- AS3967 - |V| = 72, |E| = 280;
		- As6461 - |V| = 129, |E| = 726;

	Folder 'Objects': contains all object files (.o files), resulting from compiling all the source files;

	Folder 'Outputs': folder to where the output files of the program are redirected;
	
	Folder 'Plots': folder to where the plots obtained from the output files are redirected.

	makefile: allows for the compiling of the simulator program by issuing 'make main' in the terminal, creating the main executable file. 'make clean' removes all object files in the 'Objects/' folder, as well as the main executable file.

	main.cpp : simulator driver program. Takes input arguments from the console, performs the simulation experiments accordingly, and prints information to the console/output files.

	main : main executable file

	
	1.1 Program Files (inside folders 'Sources/' and 'Headers/'):
		- enums.h : defines some global variables and types used throughout the simulator;
		
################# Attributes ##################
		- shortestPathMetric.cpp / shortestPathMetric.h : class that implements an attribute of the shortest paths metric;

		- shortestWidestPathMetric.cpp / shortestWidestPathMetric.h : class that implements an attribute of the shortest-widest paths metric;

		- eigrpMetric.cpp / eigrpMetric.h : class that implements an attribute of the EIGRP composite metric;

		- BGPAttribute.cpp / BGPAttribute.h : class that implements a BGP attribute. A BGP attribute has two fields, the attribute weight, which can be one of the 3 attributes above, and the AS_PATH, that records the nodes that constitute the path;
################# end of Attributes ##################

	        - algebra.cpp / algebra.h : wrapper class for a Routing Algebra. Implements a generic extension operation, and an order relationship between attributes;

		- nodeOneDest.cpp / nodeOneDest.h : class that represents a node and records all the state information needed for both the EIGRP and BGP protocols. Since computations pertaining to different destinations are treated separately, the state variables of this class consider a single destination only;

		- event.cpp / event.h : class that represents an event. Stores the type of the event (message received, announcement, withdrawal, link change, ...), the destination node that the event pertains to, the sender and receiver nodes (in case of messages or changes in links), and the attribute that is announced (in the case of update or diffusing messages in EIGRP, and update messages in BGP);

		- stats.cpp / stats.h : class that stores several statistic counters, such as the number of messages exchanged, termination times, number of state transitions, etc.

		- utilities.cpp / utilities.h : declaration and definition of three utility functions: (i) a function that computes the attributes in stable state via Dijkstra algorithm; (ii) a function that reads a file is csv format containing the network links and creates a vector of objects of class 'NodeOneDest', one for each node in the network, and the corresponding links; and (iii) a function that determines whether the network is connected, using a BFS algorithm;

		- eventQueueNode.cpp / eventQueueNode.h : class that implements a node of the queue of events. Each such node has a key, that represents the time at which the event is to take place; the event itself, from the 'Event' class, defined in 'event.h'; and a sequence number. Event nodes are ordered in the priority queue of nodes by increasing order of key values. In case of equal key values, the sequence number (which is unique per event and increases everytime a new event is created) is used as a tie-breaker: events with the same key value are ordered by increasing sequence numbers;

################# Protocols ##################
		- protocol.h : Contains the abstract class 'Protocol'. Provides a type to a generic routing protocol;

		- bgpOneDest.cpp / bgpOneDest.h : class that implements the BGP protocol. It implements all state transitions as defined by the BGP protocol following internal events (reception of a message from an out-neighbour) and external events (destination announcement and withdrawal, link attribute change, link addition and link failure). It also encompasses a method to determine if, in stable state, the attribute elected at each node is according to the attribute obtained by the Dijkstra Algorithm on the network. Implements the 'Protocol' class;

		- eigrpOneDest.cpp / eigrpOneDest.h : class that implements the EIGRP protocol. It implements all state transitions as defined by the EIGRP protocol following internal events (reception of a message from an out-neighbour) and external events (destination announcement and withdrawal, link attribute change, link addition and link failure). It also encompasses a method to determine if, in stable state, the attribute elected at each node is according to the attribute obtained by the Dijkstra Algorithm on the network. Implements the 'Protocol' class;

		- bgpSynchronous.cpp / bgpSynchronous.h : class 'BGPSync', which implements a synchronous version of the BGP protocol;

		- eigrpSynchronous.cpp / eigrpSynchronous.h : class 'EIGRPSync' which implements a synchronous version of the EIGRP protocol;

		- bgpQuasiSync.cpp / bgpQuasiSync.h : class 'BGPQuasiSync'. This class implements a protocol similar to BGP, but in which there is a small delay between the reception of a message and the election of a new path/attribute. This delay is smaller then the lowest link delay in the network, and serves mostly for messages that are scheduled for reception at the same time to be processed concurrently. This version of BGP is similar to the protocol in real scenarios, where the MRAI timer is set to a non-zero value;
################# end of Protocols ##################

		- event.cpp / event.h : class that represents an event. Stores the type of the event (message received, announcement, withdrawal, link change, ...), the destination node that the event pertains to, the sender and receiver nodes (in case of messages or changes in links), and the attribute that is announced (in the case of update or diffusing messages in EIGRP, and update messages in BGP);

		- stats.cpp / stats.h : class that stores several statistic counters, such as the number of messages exchanged, termination times, number of state transitions, etc.

		- utilities.cpp / utilities.h : declaration and definition of three utility functions: (i) a function that computes the attributes in stable state via Dijkstra algorithm; (ii) a function that reads a file is csv format containing the network links and creates a vector of objects of class 'NodeOneDest', one for each node in the network, and the corresponding links; and (iii) a function that determines whether the network is connected, using a BFS algorithm;

		- eventQueueNode.cpp / eventQueueNode.h : class that implements a node of the queue of events. Each such node has a key, that represents the time at which the event is to take place; the event itself, from the 'Event' class, defined in 'event.h'; and a sequence number. Event nodes are ordered in the priority queue of nodes by increasing order of key values. In case of equal key values, the sequence number (which is unique per event and increases everytime a new event is created) is used as a tie-breaker: events with the same key value are ordered by increasing sequence numbers;

		- asyncSimulation.cpp / asyncSimulation.h : function to perform the simulation experiments when the protocol behaviour is asynchronous, and corresponding auxiliar functions. The function in the header file is called by main.cpp;

		- syncSimulation.cpp / syncSimulation.h : function to perform the simulation experiments when the protocol behaviour is synchronous, and corresponding auxiliar functions. The function in the header file is called by main.cpp;



2. Usage: 
	2.1 COMPILE:
* To compile the program, run 'make main'. The main executable, 'main', is created. Object files (.o) are sent to the 'Objects/' folder. This folder provides the program fully compiled. All object files and the main executable are provided.

	2.2 RUN:
* To run the program, issue the following command in the command line:
./main Network [-p Protocol] [-t Type] [-m Metric] [-d Destination] [-del Delays] [Test_opt]  [-v]

The arguments are the following:
Network : network where the simulation is to be run. The network file is assumed to be named [Network].csv, and inside the folder 'Networks/'. This argument is mandatory, and an error is thrown if not provided.

-p/-protocol : protocol to be simulated. 'Protocol' can either be 'bgp' or 'eigrp' (not case sensitive). If another value is provided, an error is thrown. Optional argument, defaults to 'eigrp' if the -p/-protocol flag is not provided. If provided, then 'Protocol' must follow right after.

-t/-type : whether the protocol behaviour is to be synchronous, asynchronous, ou with a small delay between the reception of a message and attribute election (quasi sync, only for the BGP protocol). 'Type' can take three values, 'sync', 'async', or 'quasi'. Not case sensitive. If another value is provided, an error is thrown. If 'Protocol'=EIGRP and 'Type'=quasi, 'Type' is assumed to be async (Quasi Synchronous version of EIGRP not implemented). Optional argument, defaults to 'sync' if the -t/-type flag is not provided. If provided, then 'Type' must follow right after.

-m/-metric : routing metric under which the protocol operates. 'Metric' can take up to four values: 
(i) 'HC', for the hop count metric; 
(ii) 'SP', for the shortest paths metric; 
(iii) 'SWP', for the shortest-widest paths metric; and 
(iv) 'EM', for the composite metric of EIGRP. 
Not case sensitive, and an error is thrown if another value is provided. Optional argument, defaults to 'HC' if the -m/-metric flag is not provided. If provided, then 'Metric' must follow right after. 

-d/-destination :  destination that announces attribute \epsilon. 'Destination' can range from 0 to |V| - 1, where |V| is the number of nodes in the network. If a value outside this range is provided, an error is thrown. Optional argument. If the -d/-destination flag is not provided, the experiments are repeated for every node as a destination, that is, announcing attribute \epsilon. If provided, 'Destination' must follow right after and the experiments are performed with only node 'Destination' as a destination.

-del/-delays : message transmition delay options for the asynchronous behaviour of protocols. 'Delays' takes an integer value from 0 to 3. Values outside this range are not allowed. Delay options are as follows: 
(i) 'Delays' = 0, message propagation delays are constant and equal for all links, taking a value of 1 ms; 
(ii) 'Delays' = 1, message propagation delays are constant and equal to the delay of each link, provided in the network file; 
(iii) 'Delays' = 2, link propagation delays are variable. Each time a new message from u to v is scheduled, the propagation delay is chosen at random from an uniform distribution in the interval [0.75*link_delay, 1.25*link_delay], where 'link_delay' is the delay associated to link u->v, provided in the network file;
(iv) 'Delays' = 3, link propagation delays are variable, and everytime a new message is scheduled, the propagation delay is chosen at random from an uniform distribution in the interval [0.01, 1] (ms).
This argument is optional, and defaults to 1 if not provided. If provided and the protocol behaviour is SYNCHRONOUS, this argument is ignored.

Test_opt : flag that represents the experiment to be conducted. May take four different values:
(i) -adv, the experiment consists only in the announcement of an attribute by a node (may be repeated for all nodes in the network if -d flag is not provided);
(ii) -w, the experiment consists in the withdrawal of an attribute by a node (may be repeated for all nodes in the network if -d flag is not provided);
(iii) -lf, the experiment consists in the failure of a link, repeated for all links in the network, one at a time. If the -d flag is not provided, then the failure of all links is repeated with every node as a destination.
(iv) -lf2, the experiment consists in the simultaneous failure of a pair of links in the network. This is repeated for every pair of links in the network that, when removed, leave the network disconnected. If the -d flag is not provided, then the failure of all links is repeated with every node as a destination.
Optional argument. If not provided, defaults to 'Test_opt' -w.

-v/-verbose : prints data files to the terminal. Optional argument. If not provided, only summary information is printed.

Arguments in square brackets are optional, and can be provided in any order. Arguments inside the same square brackets, if provided, must conform to the presented order (eg. -p Protocol and not Protocol -p, nor -p only, nor Protocol only).


	2.3 RUNNING EXAMPLES:

./main Abilene : runs the EIGRP protocol with SYNCHRONOUS behaviour, under the HOP COUNT metric, in the ABILENE network, and, for every node in the network, each node announces attribute \epsilon and, when a stable state is reached, withdarws it;

./main AS1221 -p BGP -t async -m sp -d 0 -adv -del 0 : runs the BGP protocol with ASYNCHRONOUS behaviour, where link delays are constant and equal for every link in the network (1 ms), under the SHORTEST PATH metric, in the AS1221 network. The experiment conducted is the ANNOUNCEMENT of node 0.

./main Abilene -p BGP -t quasi -m swp -lf : runs the BGP protocol with QUASI-SYNCHRONOUS behaviour, where link propagation delays are constant and equal to the link delays in the Abilene.csv file, under the SHORTEST PATH metric, in the Abilene network. For every node of the network as a destination, each link in the network is failed and the protocol allowed to converge.


	2.4 PROGRAM OUTPUT:

- The distribution of Termination times per node, per destination (or per node, per link failure, depending on the case), is printed to a file named
<Network>/<Metric>/<Protocol>_<Type><Test_opt>_TerminationTimes.txt;  

- The distribution of messages exchanged per destination (or per link failure, depending on the case), is printed to a file named
<Network>/<Metric>/<Protocol>_<Type><Test_opt>_Messages.txt;


- The distribution of update to diffusing state transitions per node, per destination (or per node, per link failure, depending on the case), is printed to a file named
<Network>/<Metric>/<Protocol>_<Type><Test_opt>_Transitions.txt;
This file is printed only when the protocol being simulated is EIGRP.


- The distribution of the number of attribute changes per node, per destination (or per node, per link failure, depending on the case), is printed to a file named
<Network>/<Metric>/<Protocol>_<Type>_<Test_opt>_AttributeChangesCount.txt;

where
<Network>, <Metric>, <Protocol>, <Type>, and <Test_opt> are as defined in section 2.2.


Example output files for Abilene Network, running EIGRP Synchronous, under the hop count metric, for the withdrawal of a destination (the corresponding command to run this example is either ./main Abilene or ./main Abilene -p eigrp -t sync -m hc -w):

'Abilene/HOPS/EIGRP_SYNC_W_TerminationTimes.txt'
11
0 0 1 1 2 3 2 3 4 4 5 5 
1 1 0 1 1 2 2 3 3 4 5 4 
2 1 1 0 2 2 1 2 3 3 4 4 
3 2 1 2 0 1 2 3 2 4 4 3 
4 3 2 2 1 0 1 2 1 3 3 2 
5 2 2 1 2 1 0 1 2 2 3 3 
6 3 3 2 3 2 1 0 1 1 2 2 
7 4 3 3 2 1 2 1 0 2 2 1 
8 4 4 3 4 3 2 1 2 0 1 2 
9 5 5 4 4 3 3 2 2 1 0 1 
10 5 4 4 3 2 3 2 1 2 1 0 

'Abilene/HOPS/EIGRP_SYNC_W_Messages.txt'
11
0 56 0 28
1 56 0 28
2 56 0 28
3 56 0 28
4 56 0 28
5 56 0 28
6 56 0 28
7 56 0 28
8 56 0 28
9 56 0 28
10 56 0 28


'Abilene/HOPS/EIGRP_SYNC_W_Transitions.txt'
11
0 1 1 1 1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 1 1 1 1 
2 1 1 1 1 1 1 1 1 1 1 1 
3 1 1 1 1 1 1 1 1 1 1 1 
4 1 1 1 1 1 1 1 1 1 1 1 
5 1 1 1 1 1 1 1 1 1 1 1 
6 1 1 1 1 1 1 1 1 1 1 1 
7 1 1 1 1 1 1 1 1 1 1 1 
8 1 1 1 1 1 1 1 1 1 1 1 
9 1 1 1 1 1 1 1 1 1 1 1 
10 1 1 1 1 1 1 1 1 1 1 1 

'Abilene/HOPS/EIGRP_SYNC_W_AttributeChangesCount.txt'
11
0 1 1 1 1 1 1 1 1 1 1 1 
1 1 1 1 1 1 1 1 1 1 1 1 
2 1 1 1 1 1 1 1 1 1 1 1 
3 1 1 1 1 1 1 1 1 1 1 1 
4 1 1 1 1 1 1 1 1 1 1 1 
5 1 1 1 1 1 1 1 1 1 1 1 
6 1 1 1 1 1 1 1 1 1 1 1 
7 1 1 1 1 1 1 1 1 1 1 1 
8 1 1 1 1 1 1 1 1 1 1 1 
9 1 1 1 1 1 1 1 1 1 1 1 
10 1 1 1 1 1 1 1 1 1 1 1 

The first line of all files presents the number of nodes in the network. The remaining lines have the date for each destination. The first collum is the destination ID. For the messages file, each line (except the first) is of the form 
<destination_ID> <Total number of messages exchanged> <Update messages> <Diffusing/Clear messages>.

If the program is ran with the -v flag, then these files are also printed to the console.


* Besides the output files, summary information is also printed to the terminal. 
Example, for the same command as before:

#### begin example ####
Running Abilene with metric HOPS and protocol EIGRP SYNC _W
|V| = 11 |E| = 28
Longest Path discovered (Advertise): 6
Longest Path discovered (Withdrawal): 6

##### Stats of Announcement Phase #####

# Number of Announcement trials: 11

# Average (Minimum / Maximum) Number of Iterations until Stable State: 5.09091 (4 / 6) iterations    

# Average (Minimum / Maximum) Termination Times per Node: 2.19835 (0 / 5) iterations

# Average (Minimum / Maximum) Number of Attribute Changes per Node: 1 (1 / 1) changes

# Average (Minimum / Maximum) Number of Messages Exchanged: 28 (28 / 28) messages
        Average Number of UPDATE Messages Exchanged: 28

#######################################


##### Stats of Withdrawal Phase #####

# Number of Withdrawal trials: 11

# Average (Minimum / Maximum) Number of Iterations until Stable State: 10.1818 (8 / 12) iterations   

# Average (Minimum / Maximum) Termination Times per Node: 2.19835 (0 / 5) iterations

# Average (Minimum / Maximum) Number of Attribute Changes per Node: 1 (1 / 1) changes

# Average (Minimum / Maximum) Number of Messages Exchanged: 56 (56 / 56) messages
        Average Number of UPDATE Messages Exchanged: 0
        Average Number of DIFFUSING (CLEAR) Messages Exchanged: 28
Average (Minimum/Maximum) Number of UPDATE->DIFFUSING transitions per node: 1 (1 / 1)
Average (Minimum/Maximum) Number of isotonicity breaks (UPDATE message causes a diffusion): 0 (0 / 0)

#######################################

#### end example ####


* Throught the program, there are several error checking control points. For example, once a stable state is reached, the elected attribute at each node is compared to the expected attributes, obtained by the Dijkstra algorithm. If any attribute differs from the expected, an error is printed to the terminal. In EIGRP, everytime a node elects a new attribute, we check that the safety property B_u > B_v holds and that no cycle was formed. If any of these fail, an error is also issued to the terminal.



