# SNMP-Jog
SNMP Jog is a trimmed down, utility first implementation of the traditional SnmpWalk command merged with a simple management station. Jog can be run via command line and focuses on providing only the most important information in a readable, comparative manner. 

Jog reports three main categories of information:

1. Agent interfaces
2. Connected Devices
3. Traffic throughput

## Guidelines
* Monitor a single device
    * Discover device interfaces
    * Discover device connections
    * Monitor the traffic on device
* Input
    * Time interval between samples
    * Number of samples to take
    * IP address of agent
    * Community
* Output
    * Device interfaces
    * Device IP neighbors
    * Traffic on each if
    * Easy to read
* Provide documentation for program tests

## Grading Factors
* Choice of MIB-II variables used
* Software Design
* Implementation of polling
* Data Presentation

## Deliverable
* Functional Spec.
* Design Spec.
* Testing
* Put into a tar file named cross.kenneth.itkin.mason.tar

