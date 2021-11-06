# alignMS

This tool can be used to co-align multispectral camera bands and convert the DN values to reflectance (given irradiance sensor data or reflecance panel 
images). The original purpose of this tool was to  process multispectral images for various research endeavours related to remote sensing assisted forest 
inventories. The code is still  developed and maintained with this intention. At this time, only Micasense Altum camera is supported.  

OpenCV, boost, openmp, cmake (and cmake-gui) and exiftool are required to run the program.


OpenCV needs to be build from source and installed with extra_modules and ENABLE_NONFREE (this in a noncommerial software). Below a brief instruction:

	 # Install the required dependencies first
	
	sudo apt install build-essential cmake git pkg-config libgtk-3-dev \
	libavcodec-dev libavformat-dev libswscale-dev libv4l-dev \
	libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev \
	gfortran openexr libatlas-base-dev python3-dev python3-numpy \
	libtbb2 libtbb-dev libdc1394-22-dev
    
	git clone https://github.com/opencv/opencv.git
	git clone https://github.com/opencv/opencv_contrib.git
	
	cd opencv
	mkdir build
	cmake-gui
	
	 # First "browse source" to the opencv directory
	 # and "browse build" to the build directory we just
	 # created.
	 
	 # Next tick the box "OPENCV_ENABLE_NONFREE" (easiest
	 # to search for it with a keyword "nonfree".
	 
	 # Next define "OPENCV_EXTRA_MODULES_PATH" to the
	 # opencv_contrib directories "modules" directory.
	 
	 # Next press configure and after configuring is done
	 # press generate.
	 
	cd build
	cmake .
	make -j4
	sudo make install

		
Exiftool can be downloaded from https://exiftool.org/ . The tool needs the path to the executable via --exifToolPath.

	
The tool is run by calling the script file alignMS.sh with appropriate arguments. E.g.

	./alignMS.sh --exifToolPath /path/to/exifTool/exiftool -i test_full/ -f 12345 -r
	
	
All images that are to be processed need to be located in the same directory (-i). This also includes the optional reflectance calibration panel images 
(detected automatically). The code outputs images in "aligned" directory that is created in the input directory. If such directory exists, "_1", "_2" ... 
will be appended to "aligned". The -f flag dictates the band ordering in the merged image file. If -f is not present, no merged file will be output.

A help can be output with -h flag:

	./alignMS.sh -h
	
	  ##########################################
	  -  This tool is used to align MicaSense  -
	  -   Altum images from separate channels  -
	  -     Aligned images are output in the   -
	  -   directory where the original images  -
	  -   are located (in sub-directory named  -
	  -               "aligned")             -
	  ##########################################

	-- Mandatory arguments --

	 -i [ --images ] <String>	Image directory
	 -p [ --panel ]  <String>	Calibration panel 
	 -e [ --exifToolPath ]  <String>	Path to exifTool 
	 
	-- Optional arguments -- default value in parenthesis ()

	 -t [ --trim ]        <Integer>	Crop black edges (0 - 100) 
	 -d [ --def_panel ]   <String>	Panel image id file 
	 -f [ --fuse ]        <String>	Merge the result 
	 -c [ --cores ]       <Integer>	Number of CPU threads used (6) 
	 -r [ --reflectance ] <None>	Output reflectance 
	 -b [ --both ]        <None>	Output reflectance and RAW 
	 -m [ --mode ]        <Integer>	Reflectance mode (1) 
		                  	   1 = Panel            
		                  	   2 = DLS+panel        
		                  	   3 = DLS              
		                  	   4 = Adaptive*        
	 -s [ --source ]         <Integer>	Reference channel 0 - 4 (4)
		                  	   0 = Blue   1 = Green 
		                  	   2 = Red    3 = NiR   
		                  	   4 = Red Edge
	
	
--source indicates which channel is going to be used as a reference, i.e.
the image from this channel is fixed and all other channels are translated
to this image plane. --mode means how the reflectance values are computed.
Adaptive is not yet implemented!

--reflectance means that the output is only reflectance images, --both
means the output will contain both reflectance images and original images
in their own directories (named "REFL" and "RAW"). --trim will crop the fused
images with the input percentage. Given --trim 5 will remove image pixels 
5% or closer to the edge of the image.  

