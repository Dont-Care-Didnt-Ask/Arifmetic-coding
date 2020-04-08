TRGT = Comp_adaptive
EN_DE_CODE = yet_another_ari
FILE = file_funcs
AUX = aux_funcs
MAIN = yet_another_main

#TRGT = Comp_wrapped
#EN_DE_CODE = f_ari_2
#MAIN = f_main	

CC = gcc
FLAGS = -g -Wall -Werror -fsanitize=address -fsanitize=undefined -fstack-protector-strong

all: $(TRGT)

$(TRGT): $(MAIN).o $(FILE).o $(AUX).o $(EN_DE_CODE).o
	$(CC) $^ $(FLAGS) -o $@

$(MAIN).o: $(MAIN).c
	$(CC) $(FLAGS) -c $<

$(EN_DE_CODE).o: $(EN_DE_CODE).c
	$(CC) $(FLAGS) -c $<

$(FILE).o: $(FILE).c
	$(CC) $(FLAGS) -c $<

$(AUX).o: $(AUX).c
	$(CC) $(FLAGS) -c $<

clean:
	rm *.o $(TRGT)