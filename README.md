# alignMS

This tool can be used to co-align multispectral camera bands and convert the DN values to reflectance (given irradiance sensor data or reflecance panel 
images). The original purpose of this tool was to  process multispectral images for various research endeavours related to remote sensing assisted forest 
inventories. The code is still  developed and maintained with this intention. At this time, only Micasense Altum camera is supported.  

The tool is run by calling the script file alignMS.sh with appropriate arguments. E.g.

	./alignMS.sh --exifToolPath /path/to/exifTool/exiftool -i test_full/ -f 12345 -r
	
	

