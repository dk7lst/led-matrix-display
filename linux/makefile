all: main

BaseEffect.o: BaseEffect.cpp BaseEffect.h
	g++ -Wall -c BaseEffect.cpp

FrameBuffer.o: FrameBuffer.cpp FrameBuffer.h
	g++ -Wall -c FrameBuffer.cpp

NetworkProtocol.o: NetworkProtocol.cpp NetworkProtocol.h FrameBuffer.h
	g++ -Wall -c NetworkProtocol.cpp

TransparencyEffect.o: TransparencyEffect.cpp TransparencyEffect.h BaseEffect.h
	g++ -Wall -c TransparencyEffect.cpp

CrossedLinesEffect.o: CrossedLinesEffect.cpp CrossedLinesEffect.h BaseEffect.h
	g++ -Wall -c CrossedLinesEffect.cpp

TextEffect.o: TextEffect.cpp TextEffect.h BaseEffect.h
	g++ -Wall -c TextEffect.cpp

VuMeterEffect.o: VuMeterEffect.cpp VuMeterEffect.h BaseEffect.h
	g++ -Wall -c VuMeterEffect.cpp

main: main.cpp BaseEffect.o TransparencyEffect.o FrameBuffer.o NetworkProtocol.o CrossedLinesEffect.o VuMeterEffect.o CrossedLinesEffect.h TextEffect.o VuMeterEffect.h TransparencyEffect.h NetworkProtocol.h
	g++ -Wall -o main main.cpp BaseEffect.o TransparencyEffect.o FrameBuffer.o NetworkProtocol.o CrossedLinesEffect.o TextEffect.o VuMeterEffect.o
