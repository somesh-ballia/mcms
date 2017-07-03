#!/bin/sh

Shadow_file="/etc/volatile/shadow"
sed -i "s/^$1:!/$1:$2/" $Shadow_file
