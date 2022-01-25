# PAA-BPP-Branch&Bound

Use the command 'make' to compile the program

Use the command 'make otimize' to compile the program with otimization flags

To run the program specify these arguments:

- path to the instance file
- seed to shuffle the data
- '1' for L1 as lower bound or '2' for the L2 as lower bound
- time in minutes that the program is allowed to run
- level of detail of the output, from 0 to 6

To save the results in a file use the following command at the end of arguments:

\> path_of_file_to_save

Example: ./bin/driver ./data/Falkenauer_T_60/Falkenauer_t60_00.txt 0 1 120 6 > fal_t_60_00.txt

A script named 'run.sh' was created for convenience, enabling to easily execute all the algorithms in group of instances with a choosen seed, lower bound and max time
The output of 'run.sh' is like a .csv file, using the '>' is possible to generate a .csv file with the compiled results

The files 'C50L1.csv', 'C50L2.csv', 'FalkenauerTL1.csv' and 'FalkenauerTL2.csv' are the results of the usage of the 'run.sh' on the selected instances, the .ods files were manualy created using the .csv, were the results were grouped for better visualization.
