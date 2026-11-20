# Source-only check. Requires stock audiomix/play and CONFIG_CHAR_DEV_DSP=y.
# Generate original 8000 Hz mono U8 plus a host-listenable WAV:
#   make OUT=generated
# Copy sbcheck and SBCHECK.U8 to ELKS; no player/kernel is installed here.
#   sh sbcheck --exclusive 0x220 SBCHECK.U8
OUT ?= generated
all:
	python3 generate.py $(OUT)
.PHONY: all
