all:
	ca65 -t apple2 raq_rom.a65
	ld65 raq_rom.o -o raq_rom.bin -C raq_rom.cfg -m raq_rom.map

clean:
	rm -f raq_rom.o
	rm -f raq_rom.bin
