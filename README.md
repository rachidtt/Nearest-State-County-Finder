# Nearest-State-County-Finder

Given the Latitude/Longitude coordinates of an input point, find the K nearest neighbors and decide the state and county of the point.


## (For the full report see Report.pdf)


## Overview

Given a large data set of coordinates in the U.S, take user input coordinates of a point by Latitude and Longitude, as well as K a number between 1 and 10 of nearest neighbors desired. Return the k nearest neighbors and perform a majority vote to get the state and county of the point. Distance is computed using equirectangular approximation. Both a basic array search implementation as well as a faster k-d tree implementation are used 


### Prerequisites

Clone the repository

```
git clone https://github.com/rachidtt/Nearest-State-County-Finder.git
```

### Instructions to run


Run the makefile

```
Make
```

run the executable

```
./Nearest
```

Input k the number of nearest neighbors you want

Enter Latitude

Enter Longitude



## Built With

* [C++](http://www.cplusplus.com/) 


## Authors

 * **Rachid Tak Tak** - [rachidtt](https://github.com/rachidtt)


