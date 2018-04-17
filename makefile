objects = shell.o parse.o builtin.o util.o job_control.o

osshell: $(objects)
	cc -o osshell $(objects)
	rm $(objects)

$(objects): structures.h builtin.h shell.h parse.h util.h job_control.h

clean:
	rm osshell $(objects)
