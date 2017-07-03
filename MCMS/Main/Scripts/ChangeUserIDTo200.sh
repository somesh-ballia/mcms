#!/bin/sh

Passwd_File="/etc/volatile/passwd"
sed -i "/^$1/s/500/200/" $Passwd_File