#!/bin/sh
targetdir=Processes/$1

cp -R Processes/Demo $targetdir
mv $targetdir/DemoLib $targetdir/$1Lib
rm -f $targetdir/*/*.o
rm -f $targetdir/*/*.depend
rm -f $targetdir/*/*.*keep*
rm -f $targetdir/*/*.*contrib*
rm -f $targetdir/*/*.a

find $targetdir -name '*Demo*' -exec \Scripts/RenameDemo.sh {} $1 \;
find $targetdir -name 'Makefile' -exec \Scripts/RenameDemo.sh {} $1 \;


echo 'Process '$1' sources created in '$targetdir', Please add it to Include/McmsProcesses.h'
