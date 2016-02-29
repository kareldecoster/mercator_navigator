IDIR = ./include
CFLAGS = -Wall -Werror -g -I$(IDIR)
CC = gcc

ODIR = ./src/obj
LDIR =

_LIBS=
LIBS = $(patsubst %,$(LDIR)/%,$(_LIBS))

_DEPS =  mercator_data_acquisition_unit.h config.h mercator_triangulator.h#.h files
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ_1 = pru_adc_reader_test.o mercator_data_acquisition_unit.o#.o files
OBJ_1 = $(patsubst %,$(ODIR)/%,$(_OBJ_1))

_OBJ_2 = mercator_triangulator.o mercator_triangulator_test.o #.o files
OBJ_2 = $(patsubst %,$(ODIR)/%,$(_OBJ_2))

_OBJ_3 = mercator_triangulator.o pru_adc_reader.o mercator_navigator.o #.o files
OBJ_3 = $(patsubst %,$(ODIR)/%,$(_OBJ_3))



$(ODIR)/%.o: ./src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	

all: $(OBJ_1) $(OBJ_2)
	make my_adc_tester
	make my_fft_tester
	make mercator_navigator
	
		
my_adc_tester: $(OBJ_1)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) -lpthread -lprussdrv
	
my_fft_tester: $(OBJ_2)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) -lfftw3 -lm
	
mercator_navigator: $(OBJ_3)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) -lfftw -lm -lpthread -lprussdrv
	
clean: 
	$ rm -f $(ODIR)/*.o core $(INCDIR)/*~
	$ rm -f ./my_adc_tester
	$ rm -f ./my_fft_tester
	$ rm -f ./mercator_navigator
	$ clear
	
	
.PHONY: clean
