#!/usr/bin/perl
use strict;
use File::Copy;
use Tie::File;
#FileName is :AttachRemoteProcess.pl
#It Enables to connect to any target machine for a remote debug.

#The Steps Are:
#1.Backup any .gdbinit in the users home directory , if it exists.
#2.Write a .gdbinit to his home directory
#3.Run the debugger


my $user = $ENV{USER};
my $NumArgs = $#ARGV +1;
my $Process_name = $ARGV[0];
my $Process_Lib = $Process_name."Lib";
my $Target_IP = $ARGV[1];
my $Mcms_Root_Location;
my $Process_Location ;
if ($NumArgs ==2)
{
    $Process_Location = "./Bin/".$Process_name;
    $Mcms_Root_Location = "."
}
else
{
    $Mcms_Root_Location = $ARGV[2];
    $Process_Location = $Mcms_Root_Location ."/Bin/".$Process_name;
}
my $Debug_Process_String = "ddd --debugger /opt/polycom/carmel/tool_chain/v5/bin/i686-polycom-linux-gnu-gdb ".$Process_Location;
my $Path_To_Local_Homedir = "/nethome/".$user;

if (($NumArgs <2) || (! -e $Process_Location))
{   
    print "usage : AttachRemoteProcess.pl <Process_name> <Target IP> [mcms_root_location] (. by default)\n";
    exit(0);
}

my $Default_gdbinit_Location_string = $Path_To_Local_Homedir."/.gdbinit";
my $Backup_gdbinit_Location_string = $Path_To_Local_Homedir."/.gdbinit_old";

local $SIG{__DIE__} = sub
{
    unlink ($Default_gdbinit_Location_string)
    };


#if ( -e $Default_gdbinit_Location_string)
#{
#    rename $Default_gdbinit_Location_string, $Backup_gdbinit_Location_string 
#	|| die "Cannot rename .gdbinit file :$!\n";
#}

copy ( "/opt/polycom/etc/.gdbinit" , $Path_To_Local_Homedir) 
    || die "Cannot copy .gdbinit file :$!\n";

my @gdbinit;
tie @gdbinit, 'Tie::File', $Default_gdbinit_Location_string || die "cannot open gdbinit file";

for (@gdbinit) {
          s/^x.*/target remote $Target_IP:9999/g;         # Replace PERL with Perl everywhere in the file
	  s/^y.*/directory \/mcms\/Processes\/$Process_name\ndirectory \/mcms\/Processes\/$Process_name\/$Process_Lib/g;
        }

untie @gdbinit;

unlink ("/tmp/mcms")
    || die "cannot delete link mcms :$!\n";

symlink ($Mcms_Root_Location, "/tmp/mcms")
    || die "cannot create link :$!\n";

my $Activate_gdbserver ="ssh root@".$Target_IP." /mcms/Scripts/run_gdbserver.sh ".$Process_name." &";
system ($Activate_gdbserver) ==0 
    or die "Cannot Activate GDBSERVER :$!\n";

sleep 10;

system($Debug_Process_String) ==0
    || die "Cannot Start ddd debugger :$!\n";

#rename $Backup_gdbinit_Location_string, $Default_gdbinit_Location_string
#	|| die "Cannot rename .gdbinit_old file :$!\n";


unlink ($Default_gdbinit_Location_string)
    || die "cannot delete .gdbinit :$!\n";

