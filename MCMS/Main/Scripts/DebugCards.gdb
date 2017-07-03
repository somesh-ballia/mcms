shell `/opt/polycom/apache/bin/apachectl stop`
shell Scripts/batchkill.sh
shell rm -f /tmp/queue/*
shell rm -f /tmp/shared_memory/*
shell rm -f loglog.txt
shell `sleep 1`
shell Scripts/CleanSharedResources.sh
shell `sleep 5`
shell Bin/Faults &
shell `sleep 1`
shell `Bin/Logger >loglog.txt &`
shell `sleep 2`
shell Bin/Authentication &
shell `sleep 1`
shell Bin/McuMngr &
shell `sleep 1`
shell Bin/ConfParty &
shell `sleep 1`
shell Bin/Resource &
shell `sleep 1`
shell Bin/MplApi &
shell `sleep 1`
shell Bin/GideonSim &
shell `sleep 1`
shell Bin/EndpointsSim &
shell `sleep 1`
shell `/opt/polycom/apache/bin/apachectl start`
shell `sleep 1`
