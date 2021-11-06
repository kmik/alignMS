# alignMS

This tool can be used to co-align multispectral camera bands and convert the DN values to reflectance (given irradiance sensor data or reflecance panel 
images). The original purpose of this tool was to  process multispectral images for various research endeavours related to remote sensing assisted forest 
inventories. The code is still  developed and maintained with this intention. At this time, only Micasense Altum camera is supported.  

OpenCV, boost, openmp, cmake (and cmake-gui) and exiftool are required to run the program.

OpenCV needs to be build from source and installed with extra_modules and ENABLE_NONFREE (this in a noncommerial software). Below a brief instruction:

	git clone https://github.com/opencv/opencv.git
	git clone https://github.com/opencv/opencv_contrib.git
	
	cd opencv
	mkdir build
	cmake-gui
	
	 # First "browse source" to the opencv directory
	 # and "browse build" to the build directory we just
	 # created.
	 
	 # Next tich the box "OPENCV_ENABLE_NONFREE" (easiest
	 # to search for it with a keyword "nonfree".
	 
	 # Next define "OPENCV_EXTRA_MODULES_PATH" to the
	 # opencv_contrib directories "modules" directory.
	 
	 # Next press configure and after configuring is done
	 # press generate.
	 
	cd build
	cmake .
	make -j4
	sudo make install
		.....	
Exiftool can be downloaded from https://exiftool.org/ . The tool needs the path to the executable via --exifToolPath.

	
The tool is run by calling the script file alignMS.sh with appropriate arguments. E.g.

	./alignMS.sh --exifToolPath /path/to/exifTool/exiftool -i test_full/ -f 12345 -r
	
	
All images that are to be processed need to be located in the same directory (-i). This also includes the optional reflectance calibration panel images 
(detected automatically). The code outputs images in "aligned" directory that is created in the input directory. If such directory exists, "_1", "_2" ... 
will be appended to "aligned".
