Simulator add-on Manual/ README file
Author: Ricardo Santos

1. Contents of this Folder:
	Folder 'Outputs' : folder to where the output files of the program are redirected, updated with the directories and subdirectories for the ASes networks. Each AS has its respective folder, with subfolders for each of the routing metrics used. The subdirectories of this folder are empty, so the main file needs to be ran in order to obtain the output files;
	
	Folder 'Plots' : folder to where the plots obtained from the output files are redirected, updated with the directories and subdirectories for the AS networks. Plots concerning the Abilene network are saved in the subdirectories of the 'Abilene' folder. Plots concerning the ASes are sent to subdirectories of folders 'Advertises', 'LinkFailures', and 'Withdrawals', according to the experiment they correspond to;

	runSimulation.sh : bash file to run all the tests for all protocols, for the Abilene network. To run all the tests for the ASes networks, uncomment line 4. To use this file, issue the command 'bash runSimuation.sh'. The stdout output of the main program is redirected to a file called out.txt, that is created upon running the bash file. The output files are sent to the 'Outputs\' folder, as usual;

	plots.py : pyhton file to generate the plots.
usage: python/python3 .\plots.py [-h] [-p PROTOCOL] [-m METRIC] [-t BEHAVIOUR] [-e EXPERIMENT] [--save] [--noshow] [--saveall] NETWORK

Plots the curves for the simulation results, according to user input
Example usage: .\plots.py -p All -m SP -t ASYNC -e W --save Abilene
Plots the curves for termination times, number of messages exchanged, number of attribute changes, and number of update to diffusing state transitions for both EIGRP 
and BGP protocols, operating asynchronously under the shortest paths metric, in a given network. Also saves the plots to the 'Plots\ folder'.

positional arguments:
NETWORK                           the name of the network to which the plots pertain. Must be the last argument. If not provided, defaults to 'Abilene'

options:
-h, --help                        show this help message and exit

-p, --protocol PROTOCOL           protocol for which the plots are generated. 'All' if both EIGRP and BGP are to be considered (options: 'EIGRP', 'BGP', 'All'; default: 'EIGRP')

-m, --metric METRIC               routing metric for which the plots are generated. 'All' if all metrics are to be considered (options: 'HC', 'SP', 'SWP', 'EM' ; default: 'HC')

-t, --type BEHAVIOUR              protocol behaviour for which the plots are generated. (options: 'SYNC', 'ASYNC', 'QUASI' ; default: 'SYNC')

-e, --experiment EXPERIMENT       experiment for which the plots are generated. (options: 'ADV', 'W', 'LF', 'LF2' ; default: 'W')

--save                            if present, the plots are saved to the Plots\ folder (default: False)

--noshow                          if present, the plots are not shown to the user. may be used in conjunction with '--save' (default: False)

--saveall                         if present, saves all plots for NETWORK network, without showing them (default: False)
