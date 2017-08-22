CC = iccavr
LIB = ilibw
CFLAGS =  -e -D__ICC_VERSION=722 -D_EE_EXTIO -DATMega1280  -l -MLongJump -MHasMul -MEnhanced -Wf-use_elpm 
ASFLAGS = $(CFLAGS) 
LFLAGS =  -e:0x20000 -ucrtatmega.o -bfunc_lit:0xe4.0x20000 -dram_end:0x21ff -bdata:0x200.0x21ff -dhwstk_size:30 -beeprom:0.4096 -fintelhex -S2
FILES = BusTransmissionLayer.o cardStoreProcess.o CommandString.o control.o diluteProcess.o FunctionalInteface.o IIC.o Libcommon.o main.o maintain.o physicalLayer.o ringQueueProcess.o string.o testQueueProcess.o uart_Printf.o unLoadProcess.o 

MAINCONTROL:	$(FILES)
	$(CC) -o MAINCONTROL $(LFLAGS) @MAINCONTROL.lk   -lcatm128
BusTransmissionLayer.o: C:\iccv7avr\include\iom1280v.h C:\iccv7avr\include\_iom640to2561v.h C:\iccv7avr\include\macros.h C:\iccv7avr\include\AVRdef.h .\..\2017-0~1.7��\B1404_LIB.h .\..\2017-0~1.7��\LibCommon.h
BusTransmissionLayer.o:	..\2017-0~1.7��\BusTransmissionLayer.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\BusTransmissionLayer.c
cardStoreProcess.o: C:\iccv7avr\include\iom1280v.h C:\iccv7avr\include\_iom640to2561v.h .\..\2017-0~1.7��\B1404_LIB.h .\..\2017-0~1.7��\Common.h .\..\2017-0~1.7��\Configuration.h C:\iccv7avr\include\eeprom.h
cardStoreProcess.o:	..\2017-0~1.7��\cardStoreProcess.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\cardStoreProcess.c
CommandString.o: .\..\2017-0~1.7��\Common.h .\..\2017-0~1.7��\Configuration.h
CommandString.o:	..\2017-0~1.7��\CommandString.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\CommandString.c
control.o: C:\iccv7avr\include\iom1280v.h C:\iccv7avr\include\_iom640to2561v.h .\..\2017-0~1.7��\B1404_LIB.h .\..\2017-0~1.7��\Common.h .\..\2017-0~1.7��\Configuration.h
control.o:	..\2017-0~1.7��\control.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\control.c
diluteProcess.o: C:\iccv7avr\include\iom1280v.h C:\iccv7avr\include\_iom640to2561v.h .\..\2017-0~1.7��\B1404_LIB.h .\..\2017-0~1.7��\Common.h .\..\2017-0~1.7��\Configuration.h C:\iccv7avr\include\eeprom.h
diluteProcess.o:	..\2017-0~1.7��\diluteProcess.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\diluteProcess.c
FunctionalInteface.o: C:\iccv7avr\include\iom1280v.h C:\iccv7avr\include\_iom640to2561v.h C:\iccv7avr\include\macros.h C:\iccv7avr\include\AVRdef.h .\..\2017-0~1.7��\B1404_LIB.h .\..\2017-0~1.7��\LibCommon.h
FunctionalInteface.o:	..\2017-0~1.7��\FunctionalInteface.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\FunctionalInteface.c
IIC.o: C:\iccv7avr\include\iom1280v.h C:\iccv7avr\include\_iom640to2561v.h .\..\2017-0~1.7��\B1404_LIB.h .\..\2017-0~1.7��\LibCommon.h
IIC.o:	..\2017-0~1.7��\IIC.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\IIC.c
Libcommon.o: C:\iccv7avr\include\iom1280v.h C:\iccv7avr\include\_iom640to2561v.h .\..\2017-0~1.7��\B1404_LIB.h .\..\2017-0~1.7��\LibCommon.h C:\iccv7avr\include\eeprom.h
Libcommon.o:	..\2017-0~1.7��\Libcommon.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\Libcommon.c
main.o: .\..\2017-0~1.7��\B1404_LIB.h C:\iccv7avr\include\iom1280v.h C:\iccv7avr\include\_iom640to2561v.h .\..\2017-0~1.7��\Common.h .\..\2017-0~1.7��\Configuration.h C:\iccv7avr\include\macros.h C:\iccv7avr\include\AVRdef.h C:\iccv7avr\include\eeprom.h
main.o:	..\2017-0~1.7��\main.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\main.c
maintain.o: C:\iccv7avr\include\iom1280v.h C:\iccv7avr\include\_iom640to2561v.h .\..\2017-0~1.7��\B1404_LIB.h .\..\2017-0~1.7��\Common.h .\..\2017-0~1.7��\Configuration.h C:\iccv7avr\include\eeprom.h
maintain.o:	..\2017-0~1.7��\maintain.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\maintain.c
physicalLayer.o: C:\iccv7avr\include\iom1280v.h C:\iccv7avr\include\_iom640to2561v.h C:\iccv7avr\include\macros.h C:\iccv7avr\include\AVRdef.h .\..\2017-0~1.7��\B1404_LIB.h .\..\2017-0~1.7��\LibCommon.h .\..\2017-0~1.7��\Common.h .\..\2017-0~1.7��\Configuration.h
physicalLayer.o:	..\2017-0~1.7��\physicalLayer.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\physicalLayer.c
ringQueueProcess.o: C:\iccv7avr\include\iom1280v.h C:\iccv7avr\include\_iom640to2561v.h .\..\2017-0~1.7��\B1404_LIB.h .\..\2017-0~1.7��\Common.h .\..\2017-0~1.7��\Configuration.h
ringQueueProcess.o:	..\2017-0~1.7��\ringQueueProcess.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\ringQueueProcess.c
string.o: .\..\2017-0~1.7��\B1404_LIB.h .\..\2017-0~1.7��\Common.h .\..\2017-0~1.7��\Configuration.h
string.o:	..\2017-0~1.7��\string.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\string.c
testQueueProcess.o: C:\iccv7avr\include\iom1280v.h C:\iccv7avr\include\_iom640to2561v.h .\..\2017-0~1.7��\B1404_LIB.h .\..\2017-0~1.7��\Common.h .\..\2017-0~1.7��\Configuration.h C:\iccv7avr\include\eeprom.h
testQueueProcess.o:	..\2017-0~1.7��\testQueueProcess.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\testQueueProcess.c
uart_Printf.o: .\..\2017-0~1.7��\B1404_LIB.h C:\iccv7avr\include\string.h C:\iccv7avr\include\_const.h
uart_Printf.o:	..\2017-0~1.7��\uart_Printf.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\uart_Printf.c
unLoadProcess.o: C:\iccv7avr\include\iom1280v.h C:\iccv7avr\include\_iom640to2561v.h .\..\2017-0~1.7��\B1404_LIB.h .\..\2017-0~1.7��\Common.h .\..\2017-0~1.7��\Configuration.h C:\iccv7avr\include\eeprom.h
unLoadProcess.o:	..\2017-0~1.7��\unLoadProcess.c
	$(CC) -c $(CFLAGS) ..\2017-0~1.7��\unLoadProcess.c
