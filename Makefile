MFLAGS := 

.DEFAULT: all
.PHONY: all debug addflag

all:
	@ echo "[+] Compile Sleepmask"
	@ $(MAKE) $(MFLAGS) --no-print-directory -C mask
	@ echo "[+] Compile UDRL"
	@ $(MAKE) $(MFLAGS) --no-print-directory -C loader

debug: addflag all

addflag:
	@echo Sleepmask: Compiling DEBUG Build
	$(eval MFLAGS += debug)
    
clean:
	@ $(MAKE) clean --no-print-directory -C mask
	@ $(MAKE) clean --no-print-directory -C loader
