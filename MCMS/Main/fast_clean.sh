#!/bin/sh

find .  -name *.o -exec rm -f {} \;
find . -name *.a | grep -v /Utils/Image2IVRSlides/libvslide/lib/ | xargs rm -f \;
rm -f Bin/*
rm -f Stripped/*
rm -f LogFiles/*
rm -f CdrFiles/*
rm -f *core* Cores/*
rm -f Cfg/Hlog/*
rm -Rf TestResults/*
rm -f httpd.access.log
rm -Rf MediaRecording/share/*
rm -f Bin.*/*
rm -f Processes/*/Makefile.auto
rm -f bin rpm
find .  -name *.depend -exec rm -f {} \;
rm -Rf $MCU_HOME_DIR/tmp/ccache_dir/*
Scripts/CleanAutoXmlGeneratedFiles.sh
find .  -name generate.flag -exec rm -f {} \;

rm -Rf McmsRpmbuild*
rm -Rf Cores/*


