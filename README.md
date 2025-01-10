# Operating Systems Final Lab

This repo represents my final project for my third year operating systems course. It is written in C and utilizes pthread, mutex, condvar, data structures, tcp connections, forking, ... . The goal was to learn how to design an efficient and resilient tcp server which could handle multiple clients concurrently. Using the sensor data flowing in from the tcp connections, a form of processing was done to check whether the readings were within specification. Finally, a log report was written, along with a csv of all collected data. 

## Tcp connections

The connection to sensor units was done via tcp in `connmgr.c`, with a library which was given by the prof (`lib/tcpsock.c`). I slightly modified it so that it could handle client timeout events (this was an extra challenge). A pthread system was also implemented to multithread the server. This way, we could handle multiple clients at once.

## Buffering the data

The system is designed to buffer the data coming from the sensors in `sbuffer.c`. This buffer handles all the condition variables and mutexes to avoid race conditions when multiple threads are writing to the same data structure. 

## Data analysis

Analysing the data, to see if all sensors were within specification, happens in `datamgr.c`. It does this by keeping a moving average per sensor node. I did go a little bit far in this, and implemented a dplist which was itself contained in a dplist. I don't know why I tortured myself, but I was motivated by the fact that this could be more flexible than just having a normal array. If and when it detects something going wrong, it logs this using the logging process. 

## Saving the data

When a data point has been processed, we write it away in a csv file. Again, we used the logging process here. The code for this isn't spectacular, its just some file manipulation. You can find it in `sensor_db.c`

## Double Linked list (dplist)

We had to write a double linked list data structure to dynamically add or remove elements efficiently. It was implemented from scratch and can be found in `lib/dplist.c`. 

## Logging 

Logging was done by first starting up a child process through a fork. Then, a pipe was made between parent and child, through which log messages could flow. This was done to always be able to write every log message, even if the main program crashes. The child destroys itself when it notices that the pipe to its parent has been closed. The code for this forking and logging can be found in `main.c`.

## Sensor node

A piece of code which was given to us by the prof. It emulates a sensor. `sensor_node.c`

## How to run

Running the following make command will build everything you need:

```
make all
```

Then running any of the tests should simulate the entire system. 

```
./test3.sh
./test5.sh
```
This should end in Valgrind telling you there are no memory leaks or errors, and leave you with `sensor_db.csv` and `gateway.log`. 

Running `testown.sh` is a bit of a torture test, and depending on how it has been configured, the program will crash. 

