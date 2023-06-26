OBJS	= lib/serialib.o serialmanager.o serialcontrol.o controlframe.o dataframe.o app.o main.o
SOURCE	= lib/serialib.cpp serialmanager.cpp serialcontrol.cpp controlframe.cpp dataframe.cpp app.cpp main.cpp
HEADER	= 
OUT	= supervisorio
CC	 = g++
FLAGS	 = -g3 -c -O2 -Wall -Wextra -Werror $(shell pkg-config gtkmm-4.0 --cflags) -mavx2 -march=x86-64 -m64
LFLAGS	 = -lm
LIBS  = $(shell pkg-config gtkmm-4.0 --libs)
# -g option enables debugging mode 
# -c flag generates object code for separate files


all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS) $(LIBS)

# create/compile the individual files >>separately<<
main.o: main.cpp
	$(CC) $(FLAGS) main.cpp -std=c++17

app.o: app.cpp
	$(CC) $(FLAGS) app.cpp -std=c++17

dataframe.o: dataframe.cpp
	$(CC) $(FLAGS) dataframe.cpp -std=c++17

controlframe.o: controlframe.cpp
	$(CC) $(FLAGS) controlframe.cpp -std=c++17

serialmanager.o: serialmanager.cpp
	$(CC) $(FLAGS) serialmanager.cpp -std=c++17

serialcontrol.o: serialcontrol.cpp
	$(CC) $(FLAGS) serialcontrol.cpp -std=c++17

lib/serialib.o: lib/serialib.cpp
	$(CC) $(FLAGS) lib/serialib.cpp -o lib/serialib.o -std=c++17

# clean house
clean:
	rm -f $(OBJS) $(OUT)