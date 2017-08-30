# Makefile for RTVideoFilter
# tk
CC=g++
CFLAGS=-c `allegro-config --libs` -O4 -D__STDC_CONSTANT_MACROS
LIB_CFLAGS=-O4 -D__STDC_CONSTANT_MACROS
RTVF_SRC_DIR=rtvideofilter
RTVF_OBJ_DIR=_obj
RTVF_LIBS=-ldl -lavcodec -lavformat -lswscale -liec61883 `allegro-config --libs` -D__STDC_CONSTANT_MACROS
RTVF_SRC=	rtvideofilter/RTVideoFilter.cpp \
		rtvideofilter/VideoEncoder.cpp

RTVF_OBJ=	_obj/RTVideoFilter.o \
		_obj/VideoEncoder.o

RTVF_BIN=../bin/rtvf

DECODERS_SRC_DIR=decoders
DECODERS_OBJ_DIR=_obj/decoders
DECODERS_BIN_DIR=../bin/decoders
DECODERS_SRC=	decoders/MediaFileDecoder.cpp \
		decoders/PipeDecoder.cpp \
		decoders/VideoCapturer.cpp

DECODERS_OBJ=	_obj/decoders/MediaFileDecoder.o \
		_obj/decoders/PipeDecoder.o \
		_obj/decoders/VideoCapturer.o

DECODERS_BIN=	../bin/decoders/MediaFileDecoder.so \
		../bin/decoders/PipeDecoder.so \
		../bin/decoders/VideoCapturer.so

MODULES_SRC=	modules/ColorFilter.cpp \
		modules/BlurFilter.cpp \
		modules/DelayFilter.cpp \
		modules/FadeawayFilter.cpp \
		modules/GrayscaleFilter.cpp \
		modules/SquareFilter.cpp \
		modules/TresholdFilter.cpp \
		modules/WaveFilter.cpp \
		modules/WeightFilter.cpp \
		modules/NoiseFilter.cpp \
		modules/EdgeFilter.cpp

MODULES_OBJ=	_obj/modules/ColorFilter.o \
		_obj/modules/BlurFilter.o \
		_obj/modules/DelayFilter.o \
		_obj/modules/FadeawayFilter.o \
		_obj/modules/GrayscaleFilter.o \
		_obj/modules/SquareFilter.o \
		_obj/modules/TresholdFilter.o \
		_obj/modules/WaveFilter.o \
		_obj/modules/WeightFilter.o \
		_obj/modules/NoiseFilter.o \
		_obj/modules/EdgeFilter.o
	
MODULES_BIN=	../bin/modules/ColorFilter.so \
		../bin/modules/BlurFilter.so \
		../bin/modules/DelayFilter.so \
		../bin/modules/FadeawayFilter.so \
		../bin/modules/GrayscaleFilter.so \
		../bin/modules/SquareFilter.so \
		../bin/modules/TresholdFilter.so \
		../bin/modules/WaveFilter.so \
		../bin/modules/WeightFilter.so \
		../bin/modules/NoiseFilter.so \
		../bin/modules/EdgeFilter.so

MODULES_SRC_DIR=modules
MODULES_OBJ_DIR=_obj/modules
MODULES_BIN_DIR=../bin/modules

RES_SRC_DIR=res
RES_BIN_DIR=../bin/res
RES_FILES= 	$(RES_SRC_DIR)/topframe.pcx \
		$(RES_SRC_DIR)/bottomframe.pcx
RES_BIN=	$(RES_BIN_DIR)/topframe.pcx \
		$(RES_BIN_DIR)/bottomframe.pcx

all: $(RTVF_BIN) $(MODULES_BIN) $(RES_BIN) $(DECODERS_BIN)

res: $(RES_BIN)

bin: $(RTVF_BIN)

modules: $(MODULES_BIN)

decoders: $(DECODERS_BIN)

$(RES_BIN): $(RES_FILES)
	rm -rf $(RES_BIN_DIR)	
	mkdir $(RES_BIN_DIR)
	cp $(RES_FILES) $(RES_BIN_DIR)

$(RTVF_BIN): $(RTVF_OBJ)
	$(CC) $(RTVF_LIBS) $? -o $@

$(RTVF_OBJ_DIR)/%.o: $(RTVF_SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $< -o $@

$(MODULES_BIN_DIR)/%.so: $(MODULES_OBJ_DIR)/%.o
	gcc -shared -Wl,-soname,$@ -o $@ $<
	
$(MODULES_OBJ_DIR)/%.o: $(MODULES_SRC_DIR)/%.cpp
	gcc $(LIB_CFLAGS) -fPIC -c $< -o $@

$(DECODERS_BIN_DIR)/%.so: $(DECODERS_OBJ_DIR)/%.o
	gcc $(LIB_CFLAGS) -shared -Wl,-soname,$@ -o $@ $<

$(DECODERS_OBJ_DIR)/%.o: $(DECODERS_SRC_DIR)/%.cpp
	gcc $(LIB_CFLAGS) -Wall -fPIC -c $< -o $@

clean:
	rm -f $(RTVF_OBJ)
	rm -f $(MODULES_OBJ)
	rm -f $(RTVF_BIN)
	rm -f $(MODULES_BIN)
	rm -f $(DECODERS_OBJ)
	rm -f $(DECODERS_BIN)
	rm -rf $(RES_BIN_DIR)

doc: $(RTVF_SRC)
	doxygen rtvideofilter/Doxyfile


