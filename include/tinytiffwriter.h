/*
    Copyright (c) 2008-220 Jan W. Krieger (<jan@jkrieger.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License (LGPL) as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


*/





#ifndef TINYTIFFWRITER_H
#define TINYTIFFWRITER_H

#include "tinytiff_export.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/*! \defgroup tinytiffwriter Tiny TIFF writer library
    \ingroup tinytiff_maingroup

   The methods in this file allow to write TIFF files with limited capabilites,
   but very fast. Usually writing TIFF files with a library like libTIFF is
   relatively slow, when multiple images are written into a single file. The methods
   in this files overcome this problem by implementing a tiny writer lib that
   allows to write a TIFF file where all images have the same properties (size,
   bit depth, ...). This is a situation thet occurs e.g. in cases where a camera
   acquires a video that should be saved as TIFF file.

   The library works like this (write 50 32x32 pixel 8-bit images:
\code
   TinyTIFFWriterFile* tif=TinyTIFFWriter_open("myfil.tif", 8, 1, 32, 32);
   if (tif) {
       for (uint16_t frame=0; frame<50; frame++) {
           uint8_t* data=readImage();
           TinyTIFFWriter_writeImage(tif, data);
       }
       TinyTIFFWriter_close(tif);
   }
\endcode
   The images are written in big- or little-endian according to your system.
   The TIFF header is set accordingly, so we do not need to shuffle around
   bytes when writing, but the created TIFF file may differ from hardware system
   to hardware system, although the same data is written (once in littl-endian,
   once in big-endian).

   Currently this library saves all images as unsigned int, but with given bit-depth
   (8, 16, 32 or 64). Also this library explicitly writes a resolution of 1 in both
   directions.

   Internally this library works like this:
   TinyTIFFWriter_open() will basically only initialize the internal datastructures
   and write the TIFF header. It also determines the byte order used by the system
   and sets the TIFF header acordingly.

   As the image size is known, the size of
   every image in the file can be predetermined (we assume a maximum number of TIFF
   directory entries). The size will be: \verbatim
      MAX_HEADER_ENTRIES*12 + SOME_FREE_SPACE + WIDTH*HEIGHT*SAMPLES*(BITS_PER_SAMPLES/8)
      ---------------------------------------   ---------------------------------
          directory/image description data                 image data
\endverbatim
   The free space, indicated as \c SOME_FREE_SPACE is used to store contents of
   extended fields, like RATIONAL or ARRAY fields.

   Every image in the file will have this size and unused bytes are set to 0x00.
   TinyTIFFWriter_writeImage() then works like this:
   The image description data is first assembled in memory, then the complete image description
   data and the complete image data is written to the file all together. This reduces the
   number of file access operations and writes the data in two reltively large chunks
   which allows the operating system to properly optimize file access. Finally this
   method will save the position of the \c NEXT_IFD_OFFSET field in the image header.
   The \c NEXT_IFD_OFFSET field is filled with the adress of the next potential image.

   Finally the method TinyTIFFWriter_close() will write \c 0x00000000 into the \c NEXT_IFD_OFFSET
   of the last image (as saved above) which ends the list of images in the file.

   This ansatz for writing TIFF files is only about a factor of 2 slower than directly
   writing binary data into a file. In addition the time needed to write an image stays
   equal also when writing many images, which is NOT the case for libtiff.

   Here are some example benchmark data acquired using MinGW on a rather old CentrinoDuo notebook:
\verbatim
TIFF SPEED TEST, 8-Bit 1000 images 32x32 pixels
  average time to write one image: 17.1907 usecs    range: [7.82222..274.895] usecs
  average image rate: 58.1709 kHz
RAW SPEED TEST, 8-Bit 1000 images 32x32 pixels
  average time to write one image: 21.8469 usecs    range: [2.23492..299.2] usecs
  average image rate: 45.7731 kHz
TIFF SPEED TEST, 16-Bit 1000 images 32x32 pixels
  average time to write one image: 15.2676 usecs    range: [5.5873..262.044] usecs
  average image rate: 65.4983 kHz
RAW SPEED TEST, 16-Bit 1000 images 32x32 pixels
  average time to write one image: 27.5138 usecs    range: [3.63175..296.406] usecs
  average image rate: 36.3454 kHz
LIBTIFF SPEED TEST, 8-Bit 1000 images 32x32 pixels
  average time to write one image: 3024.75 usecs    range: [113.143..7161.52] usecs
  average image rate: 0.330606 kHz
LIBTIFF SPEED TEST, 16-Bit 1000 images 32x32 pixels
  average time to write one image: 3028.42 usecs    range: [120.965..10426.7] usecs
  average image rate: 0.330205 kHz
\endverbatim
   So this library is about a factor of 2.2 slower than direct binary output (raw) and about a factor
   of 500 faster than libTIFF. Note however the wide range of per-image write speeds which stems
   from the time the operating systems takes for file access. But the average rates are very good, so
   if your image creation is synchronous, you will need to use a FIFO to save images intermediately to
   account for the write speed jitter.
 */

/** \brief struct used to describe a TIFF file
  * \ingroup tinytiffwriter
  */
typedef struct TinyTIFFWriterFile TinyTIFFWriterFile; // forward

#ifndef TINYTIFFWRITER_TRUE
/** \brief a logic value of TRUE, e.g. used by TINYTIFFWRITER_wasError()
  * \ingroup tinytiffwriter
  */
#  define TINYTIFFWRITER_TRUE 1
#endif
#ifndef TINYTIFFWRITER_FALSE
/** \brief a logic value of FALSE, e.g. used by TINYTIFFWRITER_wasError()
  * \ingroup tinytiffwriter
  */
#  define TINYTIFFWRITER_FALSE 0
#endif

#ifdef __cplusplus
extern "C" {
#endif
    /** \brief maximum size of the imageDescription field in the first frame (including trailing \c 0, which has to be present!)
      * \ingroup tinytiffwriter
      */
    TINYTIFF_EXPORT int TinyTIFFWriter_getMaxDescriptionTextSize();

    /*! \brief returns a pointer to the last error message
        \ingroup tinytiffwriter

        \param tiff TIFF file

        \note the pointer is accessible as long as the TIFF file has not been closed using TINYTIFFWRITER_close()
    */
    TINYTIFF_EXPORT const char* TinyTIFFWriter_getLastError(TinyTIFFWriterFile* tiff);

    /*! \brief returns TRUE (non-zero) when there was an error in the last function call, or FALSE (zero) if there was no error
        \ingroup tinytiffwriter

        \param tiff TIFF file

    */
    TINYTIFF_EXPORT int TinyTIFFWriter_wasError(TinyTIFFWriterFile* tiff);

    /*! \brief returns TINYTIFFWRITER_TRUE (non-zero) when there was no error in the last function call, or TINYTIFFWRITER_FALSE if there was an error
        \ingroup tinytiffwriter

        \param tiff TIFF file

        */
    TINYTIFF_EXPORT int TinyTIFFWriter_success(TinyTIFFWriterFile* tiff);

    /*! \brief create a new TIFF file
        \ingroup tinytiffwriter

        \param filename name of the new TIFF file
        \param number of samples per pixel (e.g. 3 for RGB images)
        \param bitsPerSample bits used to save each sample of the images
        \param width width of the images in pixels
        \param height height of the images in pixels
        \return a new TinyTIFFWriterFile pointer on success, or NULL on errors

      */
    TINYTIFF_EXPORT TinyTIFFWriterFile* TinyTIFFWriter_open(const char* filename, uint16_t bitsPerSample, uint16_t samples, uint32_t width, uint32_t height);

    /*! \brief write a new image to the give TIFF file
        \ingroup tinytiffwriter

        \param tiff TIFF file to write to
        \param data points to the image in row-major ordering with the right bit-depth,
                    multi-sample data has to be provided in the "chunky" format, e.g. if
                    you have 3 samples ("R", "G" and "B"), the the data in this field has to
                    be \c R1G1B1|R2G2B2|R3G3B3|R4G4B4|...
        \return TINYTIFFWRITER_TRUE on success and TINYTIFFWRITER_FALSE on failure.
                An error description can be obtained by calling TinyTIFFWriter_getLastError().
    */
    TINYTIFF_EXPORT int TinyTIFFWriter_writeImage(TinyTIFFWriterFile* tiff, const void* data);


    /*! \brief close a given TIFF file
        \ingroup tinytiffwriter

        \param tiff TIFF file to close
        \param pixel_width pixel width in nanometers
        \param pixel_height pixel width in nanometers
        \param deltaz in a multi-frame-TIFF distance between image planes in nanometers
        \param frametime in a multi-frame-TIFF frametime in seconds

            This functions writes some additional data into the ImageDescription field of the first frame, if it is proved (!=0!!!).
        It also writes the image count there. The ImageDescription finally has the form:
    \verbatim
        TinyTIFFWriter_version=1.1
        images=1000
        pixel_width=100
        pixel_height=100
        deltaz=100
        frametime=1e-4
    \endverbatim

    This function also releases memory allocated in TinyTIFFWriter_open() in \a tiff.
    */
    TINYTIFF_EXPORT void TinyTIFFWriter_close_withmetadatadescription(TinyTIFFWriterFile* tiff, double pixel_width, double pixel_height, double frametime, double deltaz);

    /*! \brief close a given TIFF file
        \ingroup tinytiffwriter

        \param tiff TIFF file to close


        This function also releases memory allocated in TinyTIFFWriter_open() in \a tiff.
    */
    TINYTIFF_EXPORT void TinyTIFFWriter_close(TinyTIFFWriterFile* tiff);


    /*! \brief close a given TIFF file and write the given string into the IMageDescription tag of the first frame in the file.
        \ingroup tinytiffwriter

        \param tiff TIFF file to close
        \param imageDescription ImageDescription tag contents (max. size: TINYTIFFWRITER_DESCRIPTION_SIZE, including trailing 0!!!)


        This function also releases memory allocated in TinyTIFFWriter_open() in \a tiff.
     */
    TINYTIFF_EXPORT void TinyTIFFWriter_close_withdescription(TinyTIFFWriterFile* tiff, const char* imageDescription);
#ifdef __cplusplus
}
#endif

#endif // TINYTIFFWRITER_H

