#!/usr/bin/perl
#file Debug_Core.pl
use strict;
use File::Copy;
use Tie::File;






my $NumArgs = $#ARGV +1;
if ($NumArgs < 2)
{
    print "usage : ./Debug_core.pl <Target IP> <Process Name> [Absolute_path_to_mcms_version_root] (. by default)\n";
    exit(0);
}

my $user = $ENV{USER};
my $path_to_core_dir = "/tmp/" .$user;
my $TargetIP = $ARGV[0];
my $Process=$ARGV[1];
my $Process_Lib = $Process."Lib";
my $Path_To_Local_Homedir = "/nethome/".$user;
my $Default_gdbinit_Location_string = $Path_To_Local_Homedir."/.gdbinit";
my $Backup_gdbinit_Location_string = $Path_To_Local_Homedir."/.gdbinit_old";

#overload the die signal to do the following actions before actually doing the die
local $SIG{__DIE__} = sub
{
    unlink ($Default_gdbinit_Location_string)
    };


#Check if the deafult path to core dir exists (/tmp/username)
if (! -e $path_to_core_dir)
{
    mkdir ($path_to_core_dir,0755) || die "can't open directory ". $path_to_core_dir." : $!";
}



#Backing up the .gdbinit file
#if ( -e $Default_gdbinit_Location_string)
#{
#    rename $Default_gdbinit_Location_string, $Backup_gdbinit_Location_string 
#	|| die "Cannot rename .gdbinit file :$!\n";
#}


copy ( "/opt/polycom/etc/.gdbinit" , $Path_To_Local_Homedir) 
    || die "Cannot copy .gdbinit file :$!\n";

#making adaptations in the .dgbinit file
my @gdbinit;
tie @gdbinit, 'Tie::File', $Default_gdbinit_Location_string || die "cannot open gdbinit file";

for (@gdbinit) {
	  s/^y.*/directory \/mcms\/Processes\/$Process\ndirectory \/mcms\/Processes\/$Process\/$Process_Lib/g;
        }

untie @gdbinit;

#Checking Input parameters
my $mcms_Version_Location;
if ($NumArgs==2)
{
    $mcms_Version_Location=".";
}
else
{
    $mcms_Version_Location=$ARGV[2];
}

#Creating /mcms Link

unlink ("/tmp/mcms")
    || die "cannot delete link mcms :$!\n";

symlink ($mcms_Version_Location, "/tmp/mcms")
    || die "cannot create link :$!\n";


#chop / sign in case it is the last character in mcms_Version_Location
if ($mcms_Version_Location =~ /\/$/)
{
    chop $mcms_Version_Location;
}
	
#Activate DDD	   
my $ddd_activate_command = "ddd --debugger /opt/polycom/carmel/tool_chain/v5/bin/i686-polycom-linux-gnu-gdb " . $mcms_Version_Location ."/Bin/". $Process ." " .$path_to_core_dir."/".$Process;

system($ddd_activate_command) == 0
    or die "ddd activation failed. Status returned was $? and error was $!";

#UNBackup .gdbinit file
#rename $Backup_gdbinit_Location_string, $Default_gdbinit_Location_string
#	|| die "Cannot rename .gdbinit_old file :$!\n";
    
#unlink ($Default_gdbinit_Location_string)
#    || die "cannot delete .gdbinit :$!\n";



