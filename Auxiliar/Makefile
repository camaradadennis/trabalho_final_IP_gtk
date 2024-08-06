vpath %.h ../include

TARGET = gen_data_file

$(TARGET): gen_data_file.c carga.h
	gcc -o $@ -iquote../include $<

.PHONY: clean
clean:
	rm -f $(TARGET)
