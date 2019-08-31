This program takes as input the size of a board and solves the full knights ('Sunny') tour problem (in which the knight starts at (0,0) and can move over two units and perpendicular one unit). It outputs all the boards that resulted in a dead end, the most coverage possible, as well as information about the different steps taken when advancing through the solution.

This program takes a brute force approach. The emphasis is on using threads and mutexes to not corrupt or mess up shared resources. 
