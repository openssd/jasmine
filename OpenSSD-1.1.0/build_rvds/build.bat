@echo off
if "%1" == "" goto error
@echo on
del *.o
del firmware.axf
del firmware.bin
armasm --apcs /interwork -g --keep --cpu=ARM7TDMI-S --diag_style=ide --fpu=None --diag_suppress=1786 -o init.o ..\target_spw\init_rvds.s
armcc --via=armcc_opt.via -I..\ftl_%1 ..\ftl_%1\ftl.c
armcc --via=armcc_opt.via -I..\ftl_%1 ..\sata\sata_identify.c
armcc --via=armcc_opt.via -I..\ftl_%1 ..\sata\sata_cmd.c
armcc --via=armcc_opt.via -I..\ftl_%1 ..\sata\sata_isr.c
armcc --via=armcc_opt.via -I..\ftl_%1 ..\sata\sata_main.c
armcc --via=armcc_opt.via -I..\ftl_%1 ..\sata\sata_table.c
armcc --via=armcc_opt.via -I..\ftl_%1 ..\target_spw\mem_util.c
armcc --via=armcc_opt.via -I..\ftl_%1 ..\target_spw\flash.c
armcc --via=armcc_opt.via -I..\ftl_%1 ..\target_spw\flash_wrapper.c
armcc --via=armcc_opt.via -I..\ftl_%1 ..\target_spw\initialize.c
armcc --via=armcc_opt.via -I..\ftl_%1 ..\target_spw\misc.c
armcc --via=armcc_opt.via -I..\ftl_%1 ..\target_spw\uart.c
armcc --via=armcc_opt.via -I..\ftl_%1 ..\tc\tc_synth.c
if "%1" == "faster" (
	armcc --via=armcc_opt.via -I..\ftl_%1 ..\target_spw\shashtbl.c 
	armlink --via=armlink_opt.via init.o ftl.o sata_identify.o sata_cmd.o sata_isr.o sata_main.o sata_table.o mem_util.o flash.o flash_wrapper.o initialize.o misc.o uart.o tc_synth.o shashtbl.o
) else (
	armlink --via=armlink_opt.via init.o ftl.o sata_identify.o sata_cmd.o sata_isr.o sata_main.o sata_table.o mem_util.o flash.o flash_wrapper.o initialize.o misc.o uart.o tc_synth.o
)
fromelf --bin -o firmware.bin firmware.axf
@goto end
:error
echo You should specify target FTL. (e.g. build tutorial)
:end
@pause
