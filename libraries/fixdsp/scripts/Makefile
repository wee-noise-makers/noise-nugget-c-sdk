SAMPLE_RATES = 8000 11025 16000 22050 32000 44100 48000 96000

all:
	@$(foreach SR, $(SAMPLE_RATES), \
           SAMPLE_RATE=$(SR) python resources_compiler.py resources.py;)
