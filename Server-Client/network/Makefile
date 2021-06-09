#
#  Makefile for network client/server examples
#
#   make server     -- build net server for workstation
#   make server SERVER=REMOTE -- build server for target
#   make client     -- build net client for workstation
#   make netthermo  -- build thermostat server for workstation
#   make netthermo SERVER=REMOTE  -- build thermostat server for target
#   make web		-- build thermostat web server for workstation
#   make web SERVER=REMOTE	-- build thermostat web server for target

CFLAGS = -g -O0 -Wall -DPORT=4201
INC := ../includes

ifeq ($(SERVER), REMOTE)
CFLAGS += -DSERVER=\"192.168.15.50\"
netthermo: thermostat_t
#web: webthermo_t

# compile everything for the ARM
netserve: netserve.c
	$(CC) $(CFLAGS) -o $@ $^
thermostat.o: thermostat.c $(INC)/driver.h $(INC)/thermostat.h		
	$(CC) $(CFLAGS) -c -I$(INC) thermostat.c
monitor.o : monitor.c $(INC)/thermostat.h
	$(CC) $(CFLAGS) -c  -I$(INC) monitor.c
#webserve.o: webserve.c webvars.h
#	$(CC_ARM) $(CFLAGS) -c  -I$(INC_ARM) webserve.c
#webvars.o: webvars.c webvars.h
#	$(CC_ARM) $(CFLAGS) -c  -I$(INC_ARM) webvars.c
    
else
CFLAGS += -DSERVER=\"127.0.0.1\"
netthermo: thermostat_s
#web: webthermo_s

# Use default compiler
thermostat.o: $(INC)/driver.h $(INC)/thermostat.h
	gcc $(CFLAGS) -c -I$(INC) thermostat.c
monitor.o: monitor.c $(INC)/thermostat.h
	gcc $(CFLAGS) -c -I$(INC) monitor.c
#webserve.o: webvars.h
#webvars.o: webvars.h thermostat.h
netserve: netserve.c
	gcc $(CFLAGS) -o $@ $^
endif

client: netclient
netclient: netclient.c
	gcc $(CFLAGS) -o $@ $^

server: netserve

# Simulation and target versions of the networked and web thermostats.
# Use the driver code in the measure directory and assume it's compiled.

thermostat_s: thermostat.o monitor.o
	gcc -o $@ $^ -lpthread -lsim -L../sim-lib

thermostat_t: thermostat.o monitor.o
	$(CC) -o $@ $^ -lpthread -lmchw -L../pi-lib

#webthermo_s: thermostat.o webserve.o webvars.o ../measure/simdrive.o
#	$(CC) -o $@ $^ -lpthread

#webthermo_t: thermostat.o webserve.o webvars.o ../posix/trgdrive.o
#	$(CC_ARM) -o $@ $^ -lpthread

clean:
	rm -f *.o *_t *_s *~ core
