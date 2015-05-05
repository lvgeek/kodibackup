APP = carpc-controller

SRCS = $(wildcard *.cpp)
#OBJS = $(SRCS:.c=.o) main.o xbmcclient_if.o commands.o
OBJS = $(SRCS:.cpp=.o)
LIBS = -lbcm2835 -lpthread -lrt

all:
	g++ main.cpp -g3 RPI_gpio.c cmd.c si4703.c -o $(APP) $(LIBS)

app: $(OBJS)
	g++ -g3 $(OBJS) -o $(APP) $(LIBS)

install:
	cp $(APP) /usr/bin/

clean:
	rm -f $(OBJS) $(APP)

tester : $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS)

#%.o : %.cpp
#	$(CXX) -c $<

#%.o : %.c
#	$(CC) -std=c99 -c $<
