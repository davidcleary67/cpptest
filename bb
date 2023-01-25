#!/bin/bash

echo Compile and Link
set -x

g++ -ggdb3 -o backup backup.cpp