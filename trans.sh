

mkdir -p "$1/tiled"

for f in "$1/"*.tif; do
	$(basename $a)
     	gdal_translate "$f" -co TILED=NO "$1tiled/$(basename $f)" -co TILED=YES -co COMPRESS=DEFLATE -co NUM_THREADS=ALL_CPUS
	bin/Debug/exifTool/exiftool -tagsfromfile "$f" -all:all -xmp "$1tiled/$(basename $f)" -overwrite_original > /dev/null 2>&1
done
