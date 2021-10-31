

mkdir -p "$1/tiled/"

for f in "$1/"*.tif; do

     gdal_translate "$f" -co TILED=NO "$1/tiled/{f%.*}.tif" -co TILED=YES -co COMPRESS=DEFLATE
done
