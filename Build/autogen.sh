#!/bin/sh

mkdir -p m4 config 
mkdir -p ../Doc

git log -v > ChangeLog

autoreconf --force --install -I config -I m4 

