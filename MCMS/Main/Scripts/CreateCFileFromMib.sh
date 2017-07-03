#!/bin/sh

#Run this script from within the library where you want to create the files
export MIBS=$1
mib2c -c mib2c.iterate_access.conf $2