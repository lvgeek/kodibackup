#! /usr/bin/env python

import serial

# on which port should the tests be performed:
PORT = /dev/ttyUSB0

# Open the file for reading.
with open('jamboree-config.txt', 'r') as infile:

    data = infile.read()  # Read the contents of the file into memory.

# Return a list of the lines, breaking at line boundaries.
my_list = data.splitlines()
for line in my_list:
        print "SET" , line
#print my_list  # Print the list.
print "WRITE"

