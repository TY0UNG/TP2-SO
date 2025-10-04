include Makefile.inc

SAMPLE_DATA=0001-sampleDataModule.bin

all: sampleCodeModule sampleDataModule shell

sampleCodeModule:
	cd SampleCodeModule; make

shell:
	cd Shell; make

sampleDataModule:
	printf "This is sample data." >> $(SAMPLE_DATA) && dd if=/dev/zero bs=1 count=1 >> $(SAMPLE_DATA)

clean:
	cd SampleCodeModule; make clean
	cd Shell; make clean
	rm -rf *.bin


.PHONY: sampleCodeModule all clean shell
