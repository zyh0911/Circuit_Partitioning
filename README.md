# Circuit_Partitioning
MICS5520H final project

My code provide two modes. The first mode is to generate output format txt file. The second mode is to measure runtime.

If you are in the hpceda server.
```
    module load gcc/gcc-10.3.0
```

Please type:

```
    mkdir build 
    cd build
    cmake ..
    make
    cd src
```

The first mode:
```   
    ./src 
```

The second mode:
```
    ./src ../../test/100.txt
```