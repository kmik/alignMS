/*
    Copyright (c) 2008-2020 Jan W. Krieger (<jan@jkrieger.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

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


#ifndef TINYTIFFREADER_H
#define TINYTIFFREADER_H

#include "tinytiff_export.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>


/*! \defgroup tinytiffreader Tiny TIFF reader library
   \ingroup tinytiff_maingroup

   The methods in this file allow to read TIFF files with limited capabilites,
   but very fast (comapred to libtiff) and also more frames from a multi-frame
   TIFF than libtiff (which is currently limited to 65535 frames due to internal
   data sizes!).

   This library currently support TIFF files, which meet the following criteria:
     * TIFF-only (no BigTIFF), i.e. max. 4GB
	 * uncompressed frames
	 * one, or more samples per frame
	 * data types: UINT, INT, FLOAT
   .

   This example reads all frames from a TIFF file:
   \code
   TinyTIFFReaderFile* tiffr=NULL;
   tiffr=TinyTIFFReader_open(filename);
    if (!tiffr) {
        std::cout<<"    ERROR reading (not existent, not accessible or no TIFF file)\n";
    } else {
        if (TinyTIFFReader_wasError(tiffr)) std::cout<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr)<<"\n";
        std::cout<<"    ImageDescription:\n"<< TinyTIFFReader_getImageDescription(tiffr) <<"\n";
        uint32_t frames=TinyTIFFReader_countFrames(tiffr);
        std::cout<<"    frames: "<<frames<<"\n";
        if (TinyTIFFReader_wasError(tiffr)) std::cout<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr)<<"\n";
        uint32_t frame=0;
        do {
            uint32_t width=TinyTIFFReader_getWidth(tiffr);
            uint32_t height=TinyTIFFReader_getHeight(tiffr);
			bool ok=true;
            if (width>0 && height>0) std::cout<<"    size of frame "<<frame<<": "<<width<<"x"<<height<<"\n";
            else { std::cout<<"    ERROR IN FRAME "<<frame<<": size too small "<<width<<"x"<<height<<"\n"; ok=false; }
            if (ok) {
                frame++;
                uint16_t* image=(uint16_t*)calloc(width*height, sizeof(uint16_t));
                TinyTIFFReader_getSampleData(tiffr, image, 0);
                if (TinyTIFFReader_wasError(tiffr)) { ok=false; std::cout<<"   ERROR:"<<TinyTIFFReader_getLastError(tiffr)<<"\n"; }

                ///////////////////////////////////////////////////////////////////
				// HERE WE CAN DO SOMETHING WITH THE IMAGE IN image (ROW-MAJOR!)
                ///////////////////////////////////////////////////////////////////

                free(image);
            }
        } while (TinyTIFFReader_readNext(tiffr)); // iterate over all frames
        std::cout<<"    read "<<frame<<" frames\n";
    }
    TinyTIFFReader_close(tiffr);
   \endcode

   This example reads the first frame in a TIFF file:
   \code
   TinyTIFFReaderFile* tiffr=NULL;
   tiffr=TinyTIFFReader_open(filename);
   if (!tiffr) {
        std::cout<<"    ERROR reading (not existent, not accessible or no TIFF file)\n";
   } else {
		uint32_t width=TinyTIFFReader_getWidth(tiffr);
        uint32_t height=TinyTIFFReader_getHeight(tiffr);
		uint16_t* image=(uint16_t*)calloc(width*height, sizeof(uint16_t));
        TinyTIFFReader_getSampleData(tiffr, image, 0);

                ///////////////////////////////////////////////////////////////////
				// HERE WE CAN DO SOMETHING WITH THE IMAGE IN image (ROW-MAJOR!)
                ///////////////////////////////////////////////////////////////////

		free(image);
	}
    TinyTIFFReader_close(tiffr);
   \endcode


 */

/*! \defgroup tinytiffreader_C TinyTIFFReader: C-Interface
   \ingroup tinytiffreader

*/

#ifndef TINYTIFFREADER_TRUE
/** \brief a logic value of TRUE, e.g. used by TinyTIFFReader_wasError()
  * \ingroup tinytiffreader
  */
#  define TINYTIFFREADER_TRUE 1
#endif
#ifndef TINYTIFFREADER_FALSE
/** \brief a logic value of FALSE, e.g. used by TinyTIFFReader_wasError()
  * \ingroup tinytiffreader
  */
#  define TINYTIFFREADER_FALSE 0
#endif

/** \brief struct used to describe a TIFF file
  * \ingroup tinytiffreader
  */
typedef struct TinyTIFFReaderFile TinyTIFFReaderFile; // forward

#ifdef __cplusplus
extern "C" {
#endif
    /*! \brief open TIFF file for reading
        \ingroup tinytiffreader_C

        \param filename name of the new TIFF file
        \return a new TinyTIFFReaderFile pointer on success, or NULL on errors, Note that you can not use TinyTIFFReader_getLastError() if
                \c NULL is returned! In this case the C method open() retrned an error, so the fiel didn't exist, or you do not have
                permission to read it. Also if no TIFF file was detected (first twi bytes were neither 'II' nor 'MM') \c NULL is returned.

      */
    TINYTIFF_EXPORT TinyTIFFReaderFile* TinyTIFFReader_open(const char* filename);


    /*! \brief close a given TIFF file
        \ingroup tinytiffreader_C

        \param tiff TIFF file to close

        This function also releases memory allocated in TinyTIFFReader_open() in \a tiff.
     */
    TINYTIFF_EXPORT void TinyTIFFReader_close(TinyTIFFReaderFile* tiff);

    /*! \brief returns a pointer to the last error message
        \ingroup tinytiffreader_C

        \param tiff TIFF file

        \note the pointer is accessible as long as the TIFF file has not been closed using TinyTIFFReader_close()
     */
    TINYTIFF_EXPORT const char* TinyTIFFReader_getLastError(TinyTIFFReaderFile* tiff);

    /*! \brief returns TINYTIFFREADER_TRUE (non-zero) when there was an error in the last function call, or TINYTIFFREADER_FALSE if there was no error
        \ingroup tinytiffreader_C

        \param tiff TIFF file

     */
    TINYTIFF_EXPORT int TinyTIFFReader_wasError(TinyTIFFReaderFile* tiff);

    /*! \brief returns TINYTIFFREADER_TRUE (non-zero) when there was no error in the last function call, or TINYTIFFREADER_FALSE if there was an error
        \ingroup tinytiffreader_C

        \param tiff TIFF file

     */
    TINYTIFF_EXPORT int TinyTIFFReader_success(TinyTIFFReaderFile* tiff);

    /*! \brief returns TINYTIFFREADER_TRUE (non-zero) if another frame exists in the TIFF file
        \ingroup tinytiffreader_C

        \param tiff TIFF file

     */
    TINYTIFF_EXPORT int TinyTIFFReader_hasNext(TinyTIFFReaderFile* tiff);

    /*! \brief reads the next frame from a multi-frame TIFF
        \ingroup tinytiffreader_C

        \param tiff TIFF file
        \return TINYTIFFREADER_TRUE (non-zero) if another frame exists in the TIFF file

     */
    TINYTIFF_EXPORT int TinyTIFFReader_readNext(TinyTIFFReaderFile* tiff);


    /*! \brief return the width of the current frame
        \ingroup tinytiffreader_C

        \param tiff TIFF file

     */
    TINYTIFF_EXPORT uint32_t TinyTIFFReader_getWidth(TinyTIFFReaderFile* tiff);

    /*! \brief return the height of the current frame
        \ingroup tinytiffreader_C

        \param tiff TIFF file

     */
    TINYTIFF_EXPORT uint32_t TinyTIFFReader_getHeight(TinyTIFFReaderFile* tiff);

    /*! \brief return the image description of the current frame
    \ingroup tinytiffreader_C

    \param tiff TIFF file

    */
    TINYTIFF_EXPORT const char* TinyTIFFReader_getImageDescription(TinyTIFFReaderFile* tiff);


    #define TINYTIFFREADER_SAMPLEFORMAT_UINT 1
    #define TINYTIFFREADER_SAMPLEFORMAT_INT 2
    #define TINYTIFFREADER_SAMPLEFORMAT_FLOAT 3
    #define TINYTIFFREADER_SAMPLEFORMAT_UNDEFINED 4

    /*! \brief return the sample format of the current frame
        \ingroup tinytiffreader_C

        \param tiff TIFF file

     */
    TINYTIFF_EXPORT uint16_t TinyTIFFReader_getSampleFormat(TinyTIFFReaderFile* tiff);

    /*! \brief return the bits per sample of the current frame
        \ingroup tinytiffreader_C

        \param tiff TIFF file
        \param sample return bits for the given sample number [default: 0]

     */
    TINYTIFF_EXPORT uint16_t TinyTIFFReader_getBitsPerSample(TinyTIFFReaderFile* tiff, int sample);

    /*! \brief return the samples per pixel of the current frame
        \ingroup tinytiffreader_C

        \param tiff TIFF file

     */
    TINYTIFF_EXPORT uint16_t TinyTIFFReader_getSamplesPerPixel(TinyTIFFReaderFile* tiff);

    /*! \brief read the given sample from the current frame into the given buffer,
               the byteorder is transformed to the byteorder of the system!
        \ingroup tinytiffreader_C

        \param tiff TIFF file
        \param buffer the buffer this function writes into
        \param sample the sample to read [default: 0]
        \return \c TINYTIFFREADER_TRUE (non-zero) on success

        \note The user is responsible for providing the correct buffer size
              (taking width, height and bitsPerSample into account).

     */
    TINYTIFF_EXPORT int TinyTIFFReader_getSampleData(TinyTIFFReaderFile* tiff, void* buffer, uint16_t sample);



    /*! \brief return the width of the current frame
        \ingroup tinytiffreader_C

        \param tiff TIFF file

     */
    TINYTIFF_EXPORT uint32_t TinyTIFFReader_countFrames(TinyTIFFReaderFile* tiff);

#ifdef __cplusplus
}
#endif

#endif // TINYTIFFREADER_H
