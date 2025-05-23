#------------------------------------------------------------------------------#
# This makefile was generated by 'cbp2make' tool rev.147                       #
#------------------------------------------------------------------------------#


WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = 
CFLAGS = -O3 -m64 `opencv-config --cxxflags` -fopenmp
RESINC = 
LIBDIR = -L/usr/local/include/opencv4
LIB = 
LDFLAGS = -m64 `opencv-config --ldstaticflags` -Wl,-Bstatic -lboost_system -Wl,-Bdynamic -Wl,-Bstatic -lboost_program_options -Wl,-Bdynamic -lboost_filesystem -lboost_system -fopenmp

INC_DEBUG = $(INC) -Iinclude -I/usr/local/include/opencv4
CFLAGS_DEBUG = $(CFLAGS) -O3 -std=c++11 -m64 -march=corei7 -fopenmp
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = $(LDFLAGS) -m64 -Wl,-Bstatic -lboost_system -Wl,-Bdynamic -Wl,-Bstatic -lboost_program_options -Wl,-Bdynamic -lboost_filesystem -lboost_system -fopenmp -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_xfeatures2d -lopencv_imgproc -lopencv_imgcodecs
OBJDIR_DEBUG = obj/Debug
DEP_DEBUG = 
OUT_DEBUG = bin/Debug/altumAlign

INC_RELEASE = $(INC) -Iinclude -I/usr/local/include/opencv4
CFLAGS_RELEASE = $(CFLAGS) -O2 -march=corei7 -fopenmp
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) -Wl,-Bstatic -lboost_system -Wl,-Bdynamic -Wl,-Bstatic -lboost_program_options -Wl,-Bdynamic -lboost_filesystem -lboost_system -fopenmp -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_xfeatures2d -lopencv_imgproc -lopencv_imgcodecs
OBJDIR_RELEASE = obj/Release
DEP_RELEASE = 
OUT_RELEASE = bin/Release/alltumAlign

OBJ_DEBUG = $(OBJDIR_DEBUG)/src/progress.o $(OBJDIR_DEBUG)/src/metadata_reader.o $(OBJDIR_DEBUG)/src/exif.o $(OBJDIR_DEBUG)/src/calibrationPanels.o $(OBJDIR_DEBUG)/src/altumImage.o $(OBJDIR_DEBUG)/src/TagInfo.o $(OBJDIR_DEBUG)/src/ExifToolPipe.o $(OBJDIR_DEBUG)/src/ExifTool.o $(OBJDIR_DEBUG)/main.o

OBJ_RELEASE = $(OBJDIR_RELEASE)/src/progress.o $(OBJDIR_RELEASE)/src/metadata_reader.o $(OBJDIR_RELEASE)/src/exif.o $(OBJDIR_RELEASE)/src/calibrationPanels.o $(OBJDIR_RELEASE)/src/altumImage.o $(OBJDIR_RELEASE)/src/TagInfo.o $(OBJDIR_RELEASE)/src/ExifToolPipe.o $(OBJDIR_RELEASE)/src/ExifTool.o $(OBJDIR_RELEASE)/main.o

all: debug release

clean: clean_debug clean_release

before_debug: 
	test -d bin/Debug || mkdir -p bin/Debug
	test -d $(OBJDIR_DEBUG)/src || mkdir -p $(OBJDIR_DEBUG)/src
	test -d $(OBJDIR_DEBUG) || mkdir -p $(OBJDIR_DEBUG)

after_debug: 

debug: before_debug out_debug after_debug

out_debug: before_debug $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) $(LIBDIR_DEBUG) -o $(OUT_DEBUG) $(OBJ_DEBUG)  $(LDFLAGS_DEBUG) $(LIB_DEBUG)

$(OBJDIR_DEBUG)/src/progress.o: src/progress.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c src/progress.cpp -o $(OBJDIR_DEBUG)/src/progress.o

$(OBJDIR_DEBUG)/src/metadata_reader.o: src/metadata_reader.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c src/metadata_reader.cpp -o $(OBJDIR_DEBUG)/src/metadata_reader.o

$(OBJDIR_DEBUG)/src/exif.o: src/exif.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c src/exif.cpp -o $(OBJDIR_DEBUG)/src/exif.o

$(OBJDIR_DEBUG)/src/calibrationPanels.o: src/calibrationPanels.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c src/calibrationPanels.cpp -o $(OBJDIR_DEBUG)/src/calibrationPanels.o

$(OBJDIR_DEBUG)/src/altumImage.o: src/altumImage.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c src/altumImage.cpp -o $(OBJDIR_DEBUG)/src/altumImage.o

$(OBJDIR_DEBUG)/src/TagInfo.o: src/TagInfo.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c src/TagInfo.cpp -o $(OBJDIR_DEBUG)/src/TagInfo.o

$(OBJDIR_DEBUG)/src/ExifToolPipe.o: src/ExifToolPipe.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c src/ExifToolPipe.cpp -o $(OBJDIR_DEBUG)/src/ExifToolPipe.o

$(OBJDIR_DEBUG)/src/ExifTool.o: src/ExifTool.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c src/ExifTool.cpp -o $(OBJDIR_DEBUG)/src/ExifTool.o

$(OBJDIR_DEBUG)/main.o: main.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c main.cpp -o $(OBJDIR_DEBUG)/main.o

clean_debug: 
	rm -f $(OBJ_DEBUG) $(OUT_DEBUG)
	rm -rf bin/Debug
	rm -rf $(OBJDIR_DEBUG)/src
	rm -rf $(OBJDIR_DEBUG)

before_release: 
	test -d bin/Release || mkdir -p bin/Release
	test -d $(OBJDIR_RELEASE)/src || mkdir -p $(OBJDIR_RELEASE)/src
	test -d $(OBJDIR_RELEASE) || mkdir -p $(OBJDIR_RELEASE)

after_release: 

release: before_release out_release after_release

out_release: before_release $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) $(LIBDIR_RELEASE) -o $(OUT_RELEASE) $(OBJ_RELEASE)  $(LDFLAGS_RELEASE) $(LIB_RELEASE)

$(OBJDIR_RELEASE)/src/progress.o: src/progress.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c src/progress.cpp -o $(OBJDIR_RELEASE)/src/progress.o

$(OBJDIR_RELEASE)/src/metadata_reader.o: src/metadata_reader.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c src/metadata_reader.cpp -o $(OBJDIR_RELEASE)/src/metadata_reader.o

$(OBJDIR_RELEASE)/src/exif.o: src/exif.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c src/exif.cpp -o $(OBJDIR_RELEASE)/src/exif.o

$(OBJDIR_RELEASE)/src/calibrationPanels.o: src/calibrationPanels.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c src/calibrationPanels.cpp -o $(OBJDIR_RELEASE)/src/calibrationPanels.o

$(OBJDIR_RELEASE)/src/altumImage.o: src/altumImage.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c src/altumImage.cpp -o $(OBJDIR_RELEASE)/src/altumImage.o

$(OBJDIR_RELEASE)/src/TagInfo.o: src/TagInfo.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c src/TagInfo.cpp -o $(OBJDIR_RELEASE)/src/TagInfo.o

$(OBJDIR_RELEASE)/src/ExifToolPipe.o: src/ExifToolPipe.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c src/ExifToolPipe.cpp -o $(OBJDIR_RELEASE)/src/ExifToolPipe.o

$(OBJDIR_RELEASE)/src/ExifTool.o: src/ExifTool.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c src/ExifTool.cpp -o $(OBJDIR_RELEASE)/src/ExifTool.o

$(OBJDIR_RELEASE)/main.o: main.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c main.cpp -o $(OBJDIR_RELEASE)/main.o

clean_release: 
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -rf bin/Release
	rm -rf $(OBJDIR_RELEASE)/src
	rm -rf $(OBJDIR_RELEASE)

.PHONY: before_debug after_debug clean_debug before_release after_release clean_release

