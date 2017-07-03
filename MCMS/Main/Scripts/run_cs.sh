export LD_LIBRARY_PATH=$MCU_HOME_DIR/cs/lib
cd $MCU_HOME_DIR/cs
./bin/acloader -c -C$MCU_HOME_DIR/cs/cfg/cfg_soft/cs_private_cfg_dev.xml -P0 -S1 -N400
