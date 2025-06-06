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
#define TINYTIFF_WRITE_COMMENTS

#include <math.h>
#include <float.h>
#include "tinytiffwriter.h"

#ifndef __WINDOWS__
# if defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32)
#  define __WINDOWS__
# endif
#endif

#ifndef __LINUX__
# if defined(linux)
#  define __LINUX__
# endif
#endif

#define __USE_LIBC_FOR_TIFF__
#ifdef __WINDOWS__
#  ifndef __USE_LIBC_FOR_TIFF__
#    define TINYTIFF_USE_WINAPI_FOR_FILEIO
#  endif
#endif // __WINDOWS__

#ifdef TINYTIFF_USE_WINAPI_FOR_FILEIO
#  include <windows.h>
#  warning COMPILING TinyTIFFWriter with WinAPI
#endif // TINYTIFF_USE_WINAPI_FOR_FILEIO

#define TIFF_ORDER_UNKNOWN 0
#define TIFF_ORDER_BIGENDIAN 1
#define TIFF_ORDER_LITTLEENDIAN 2
#define TRUE (0==0)
#define FALSE (0==1)

#define TINYTIFFWRITER_DESCRIPTION_SIZE 1024
#define TIFF_LAST_ERROR_SIZE 1024

int TinyTIFFWriter_getMaxDescriptionTextSize() {
    return TINYTIFFWRITER_DESCRIPTION_SIZE;
}

/** \defgroup tinytiffwriter_internal TinyTIFFWriter: Internal functions
 *  \ingroup tinytiffwriter */

/*! \brief determines the byte order of the system
    \ingroup tinytiffwriter_internal
    \internal

    \return TIFF_ORDER_BIGENDIAN or TIFF_ORDER_LITTLEENDIAN, or TIFF_ORDER_UNKNOWN if the byte order cannot be determined
 */
int TIFF_get_byteorder()
{
        union {
                long l;
                char c[4];
        } test;
        test.l = 1;
        if( test.c[3] && !test.c[2] && !test.c[1] && !test.c[0] )
                return TIFF_ORDER_BIGENDIAN;

        if( !test.c[3] && !test.c[2] && !test.c[1] && test.c[0] )
                return TIFF_ORDER_LITTLEENDIAN;

        return TIFF_ORDER_UNKNOWN;
}


/*! \brief this struct represents a TIFF file
    \ingroup tinytiffwriter_internal
    \internal
 */
struct TinyTIFFWriterFile {
#ifdef TINYTIFF_USE_WINAPI_FOR_FILEIO
    /* \brief the windows API file handle */
    HANDLE hFile;
#else
    /* \brief the libc file handle */
    FILE* file;
#endif // TINYTIFF_USE_WINAPI_FOR_FILEIO
    /* \brief position of the field in the previously written IFD/header, which points to the next frame. This is set to 0, when closing the file to indicate, the last frame! */
    uint32_t lastIFDOffsetField;
    /* \brief file position (from ftell) of the first byte of the previous IFD/frame header */
    long int lastStartPos;
    //uint32_t lastIFDEndAdress;
    uint32_t lastIFDDATAAdress;
    /* \brief counts the entries in the current IFD/frame header */
    uint16_t lastIFDCount;
    /* \brief temporary data array for the current header */
    uint8_t* lastHeader;
    int lastHeaderSize;
    /* \brief current write position in lastHeader */
    uint32_t pos;
    /* \brief width of the frames */
    uint32_t width;
    /* \brief height of the frames */
    uint32_t height;
    /* \brief bits per sample of the frames */
    uint16_t bitspersample;
    /* \brief number of samples of the frames */
    uint16_t samples;
    uint32_t descriptionOffset;
    uint32_t descriptionSizeOffset;
    /* \brief counter for the frames, written into the file */
    uint64_t frames;
    /* \brief specifies the byte order of the system (and the written file!) */
    uint8_t byteorder;
    char lastError[TIFF_LAST_ERROR_SIZE];
    int wasError;
};

/*! \brief wrapper around fopen
    \ingroup tinytiffwriter_internal
    \internal
 */
static void TinyTIFFWriter_fopen(TinyTIFFWriterFile* tiff, const char* filename) {
#ifdef TINYTIFF_USE_WINAPI_FOR_FILEIO
    tiff->hFile = CreateFile(filename,               // name of the write
                       GENERIC_WRITE,          // open for writing
                       0,                      // do not share
                       NULL,                   // default security
                       CREATE_NEW,             // create new file only
                       FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,  // normal file
                       NULL);                  // no attr. template
#else
#  ifdef HAVE_FOPEN_S
    fopen_s(&(tiff->file), filename, "wb");
#  else
    tiff->file=fopen(filename, "wb");
#  endif
#endif
}

/*! \brief checks whether a file was opened successfully
    \ingroup tinytiffwriter_internal
    \internal
 */
static int TinyTIFFWriter_fOK(TinyTIFFWriterFile* tiff) {
#ifdef TINYTIFF_USE_WINAPI_FOR_FILEIO
   if (tiff->hFile == INVALID_HANDLE_VALUE) return FALSE;
   else return TRUE;
#else
   if (tiff->file) return TRUE;
   else return FALSE;
#endif
}

/*! \brief wrapper around fclose
    \ingroup tinytiffwriter_internal
    \internal
 */
static int TinyTIFFWriter_fclose(TinyTIFFWriterFile* tiff) {
#ifdef TINYTIFF_USE_WINAPI_FOR_FILEIO
    CloseHandle(tiff->hFile);
    return 0;
#else
    int r=fclose(tiff->file);
    tiff->file=NULL;
    return r;
#endif
}

/*! \brief wrapper around fwrite
    \ingroup tinytiffwriter_internal
    \internal
 */
static size_t TinyTIFFWriter_fwrite(const void * ptr, size_t size, size_t count, TinyTIFFWriterFile* tiff) {
#ifdef TINYTIFF_USE_WINAPI_FOR_FILEIO
   DWORD dwBytesWritten = 0;
    WriteFile(
                    tiff->hFile,           // open file handle
                    ptr,      // start of data to write
                    size*count,  // number of bytes to write
                    &dwBytesWritten, // number of bytes that were written
                    NULL);
    return dwBytesWritten;
#else
    return fwrite(ptr, size, count, tiff->file);
#endif
}

/*! \brief wrapper around ftell
    \ingroup tinytiffwriter_internal
    \internal
 */
static long int TinyTIFFWriter_ftell ( TinyTIFFWriterFile * tiff ) {
#ifdef TINYTIFF_USE_WINAPI_FOR_FILEIO
DWORD dwPtr = SetFilePointer( tiff->hFile,
                                0,
                                NULL,
                                FILE_CURRENT );
    return dwPtr;
#else
    return ftell(tiff->file);
#endif
}


/*! \brief wrapper around fseek
    \ingroup tinytiffwriter_internal
    \internal
 */
static int TinyTIFFWriter_fseek_set(TinyTIFFWriterFile* tiff, size_t offset) {
#ifdef TINYTIFF_USE_WINAPI_FOR_FILEIO
   DWORD res = SetFilePointer (tiff->hFile,
                                offset,
                                NULL,
                                FILE_BEGIN);


   return res;
#else
    return fseek(tiff->file, (long)offset, SEEK_SET);
#endif // TINYTIFF_USE_WINAPI_FOR_FILEIO
}

/*! \brief wrapper around fseek(..., FILE_CURRENT)
    \ingroup tinytiffwriter_internal
    \internal
 */
static int TinyTIFFWriter_fseek_cur(TinyTIFFWriterFile* tiff, size_t offset) {
#ifdef TINYTIFF_USE_WINAPI_FOR_FILEIO
   DWORD res = SetFilePointer (tiff->hFile,
                                offset,
                                NULL,
                                FILE_CURRENT);


   return res;
#else
    return fseek(tiff->file, (long)offset, SEEK_CUR);
#endif // TINYTIFF_USE_WINAPI_FOR_FILEIO
}

#define TIFF_FIELD_IMAGEWIDTH 256
#define TIFF_FIELD_IMAGELENGTH 257
#define TIFF_FIELD_BITSPERSAMPLE 258
#define TIFF_FIELD_COMPRESSION 259
#define TIFF_FIELD_PHOTOMETRICINTERPRETATION 262
#define TIFF_FIELD_IMAGEDESCRIPTION 270
#define TIFF_FIELD_STRIPOFFSETS 273
#define TIFF_FIELD_SAMPLESPERPIXEL 277
#define TIFF_FIELD_ROWSPERSTRIP 278
#define TIFF_FIELD_STRIPBYTECOUNTS 279
#define TIFF_FIELD_XRESOLUTION 282
#define TIFF_FIELD_YRESOLUTION 283
#define TIFF_FIELD_PLANARCONFIG 284
#define TIFF_FIELD_RESOLUTIONUNIT 296
#define TIFF_FIELD_SAMPLEFORMAT 339

#define TIFF_TYPE_BYTE 1
#define TIFF_TYPE_ASCII 2
#define TIFF_TYPE_SHORT 3
#define TIFF_TYPE_LONG 4
#define TIFF_TYPE_RATIONAL 5

#define TIFF_PHOTOMETRIC_WHITEISZERO 0
#define TIFF_PHOTOMETRIC_BLACKISZERO 1
#define TIFF_PHOTOMETRIC_RGB 2
#define TIFF_PHOTOMETRIC_PALETTE 3
#define TIFF_PHOTOMETRIC_TRANSPARENCY 4


/*! \brief fixed size of the TIFF frame header in bytes
    \ingroup tinytiffwriter_internal
    \internal
 */
#define TIFF_HEADER_SIZE 700
/*! \brief maximum number of field entries in a TIFF header
    \ingroup tinytiffwriter_internal
    \internal
 */
#define TIFF_HEADER_MAX_ENTRIES 20





/*! \brief write a 4-byte word \a data directly into a file \a fileno
    \ingroup tinytiffwriter_internal
    \internal
 */
#define WRITE32DIRECT(filen, data)  { \
    TinyTIFFWriter_fwrite((void*)(&(data)), 4, 1, filen); \
}

/*! \brief write a data word \a data , which is first cast into a 4-byte word directly into a file \a fileno
    \ingroup tinytiffwriter_internal
    \internal
 */
#define WRITE32DIRECT_CAST(filen, data)  { \
    uint32_t d=data; \
    WRITE32DIRECT((filen), d); \
}






/*! \brief write a 2-byte word \a data directly into a file \a fileno
    \ingroup tinytiffwriter_internal
    \internal
 */
#define WRITE16DIRECT(filen, data)    { \
    TinyTIFFWriter_fwrite((void*)(&(data)), 2, 1, filen); \
}

/*! \brief write a data word \a data , which is first cast into a 2-byte word directly into a file \a fileno
    \ingroup tinytiffwriter_internal
    \internal
 */
#define WRITE16DIRECT_CAST(filen, data)    { \
    uint16_t d=data; \
    WRITE16DIRECT((filen), d); \
}




/*! \brief write a data word \a data , which is first cast into a 1-byte word directly into a file \a fileno
    \ingroup tinytiffwriter_internal
    \internal
 */
#define WRITE8DIRECT(filen, data) {\
    uint8_t ch=data; \
    TinyTIFFWriter_fwrite(&ch, 1, 1, filen);\
}














/*! \brief writes a 32-bit word at the current position into the current file header and advances the position by 4 bytes
    \ingroup tinytiffwriter_internal
    \internal
 */
#define WRITEH32DIRECT_LE(filen, data)  { \
    *((uint32_t*)(&filen->lastHeader[filen->pos]))=data; \
    filen->pos+=4;\
}
/*! \brief writes a value, which is cast to a 32-bit word at the current position into the current file header and advances the position by 4 bytes
    \ingroup tinytiffwriter_internal
    \internal
 */
#define WRITEH32_LE(filen, data)  { \
    uint32_t d=data; \
    WRITEH32DIRECT_LE(filen, d); \
}

// write 2-bytes in big endian
/*! \brief writes a 16-bit word at the current position into the current file header and advances the position by 4 bytes
    \ingroup tinytiffwriter_internal
    \internal
 */
#define WRITEH16DIRECT_LE(filen, data)    { \
    *((uint16_t*)(&filen->lastHeader[filen->pos]))=data; \
    filen->pos+=2; \
}

/*! \brief writes a value, which is cast to a 16-bit word at the current position into the current file header and advances the position by 4 bytes
    \ingroup tinytiffwriter_internal
    \internal
 */
#define WRITEH16_LE(filen, data)    { \
    uint16_t d=data; \
    WRITEH16DIRECT_LE(filen, d); \
}


// write byte
/*! \brief writes an 8-bit word at the current position into the current file header and advances the position by 4 bytes
    \ingroup tinytiffwriter_internal
    \internal
 */
#define WRITEH8(filen, data) { filen->lastHeader[filen->pos]=data; filen->pos+=1; }
/*! \brief writes an 8-bit word at the current position into the current file header and advances the position by 4 bytes
    \ingroup tinytiffwriter_internal
    \internal
 */
#define WRITEH8DIRECT(filen, data) { filen->lastHeader[filen->pos]=data; filen->pos+=1; }

// write 2 bytes
#define WRITEH16(filen, data)  WRITEH16_LE(filen, data)
#define WRITEH32(filen, data)  WRITEH32_LE(filen, data)

#define WRITEH16DIRECT(filen, data)  WRITEH16DIRECT_LE(filen, data)
#define WRITEH32DIRECT(filen, data)  WRITEH32DIRECT_LE(filen, data)

/*! \brief starts a new IFD (TIFF frame header)
    \ingroup tinytiffwriter_internal
    \internal
 */
static void TinyTIFFWriter_startIFD(TinyTIFFWriterFile* tiff, int hsize) {
    if (!tiff) return;
    tiff->lastStartPos=TinyTIFFWriter_ftell(tiff);//ftell(tiff->file);
    //tiff->lastIFDEndAdress=startPos+2+TIFF_HEADER_SIZE;
    tiff->lastIFDDATAAdress=2+TIFF_HEADER_MAX_ENTRIES*12;
    tiff->lastIFDCount=0;
    if (tiff->lastHeader!=NULL && hsize!=tiff->lastHeaderSize) {
        free(tiff->lastHeader);
        tiff->lastHeader=NULL;
        tiff->lastHeaderSize=0;
    }
    if (tiff->lastHeader==NULL) {
        tiff->lastHeader=(uint8_t*)calloc(hsize+2, 1);
        tiff->lastHeaderSize=hsize;
    } else {
        memset(tiff->lastHeader, 0, hsize+2);
    }
    tiff->pos=2;
}

/*! \brief ends the current IFD (TIFF frame header) and writes the header (as a single block of size TIFF_HEADER_SIZE) into the file
    \ingroup tinytiffwriter_internal
    \internal

    This function also sets the pointer to the next IFD, based on the known header size and frame data size.
 */
static void TinyTIFFWriter_endIFD(TinyTIFFWriterFile* tiff, int hsize) {
    if (!tiff) return;
    //long startPos=ftell(tiff->file);

    tiff->pos=0;
    WRITEH16DIRECT(tiff, tiff->lastIFDCount);

    tiff->pos=2+tiff->lastIFDCount*12; // header start (2byte) + 12 bytes per IFD entry
    WRITEH32(tiff, tiff->lastStartPos+2+hsize+tiff->width*tiff->height*(tiff->bitspersample/8));
    //printf("imagesize = %d\n", tiff->width*tiff->height*(tiff->bitspersample/8));

    //fwrite((void*)tiff->lastHeader, TIFF_HEADER_SIZE+2, 1, tiff->file);
    TinyTIFFWriter_fwrite((void*)tiff->lastHeader, tiff->lastHeaderSize+2, 1, tiff);
    tiff->lastIFDOffsetField=tiff->lastStartPos+2+tiff->lastIFDCount*12;
    //free(tiff->lastHeader);
    //tiff->lastHeader=NULL;
}

/*! \brief write an arbitrary IFD entry
    \ingroup tinytiffwriter_internal
    \internal

    \note This function writes into TinyTIFFFile::lastHeader, starting at the position TinyTIFFFile::pos
 */
static void TinyTIFFWriter_writeIFDEntry(TinyTIFFWriterFile* tiff, uint16_t tag, uint16_t type, uint32_t count, uint32_t data) {
    if (!tiff) return;
    if (tiff->lastIFDCount<TIFF_HEADER_MAX_ENTRIES) {
        tiff->lastIFDCount++;
        WRITEH16DIRECT(tiff, tag);
        WRITEH16DIRECT(tiff, type);
        WRITEH32DIRECT(tiff, count);
        WRITEH32DIRECT(tiff, data);
    }
}

/*! \brief write an 8-bit word IFD entry
    \ingroup tinytiffwriter_internal
    \internal

    \note This function writes into TinyTIFFFile::lastHeader, starting at the position TinyTIFFFile::pos
 */
 static void TinyTIFFWriter_writeIFDEntryBYTE(TinyTIFFWriterFile* tiff, uint16_t tag, uint8_t data) {
    if (!tiff) return;
    if (tiff->lastIFDCount<TIFF_HEADER_MAX_ENTRIES) {
        tiff->lastIFDCount++;
        WRITEH16DIRECT(tiff, tag);
        WRITEH16(tiff, TIFF_TYPE_BYTE);
        WRITEH32(tiff, 1);
        WRITEH8DIRECT(tiff, data);
        WRITEH8(tiff, 0);
        WRITEH16(tiff, 0);
    }
}

/*! \brief write an 16-bit word IFD entry
    \ingroup tinytiffwriter_internal
    \internal

    \note This function writes into TinyTIFFFile::lastHeader, starting at the position TinyTIFFFile::pos
 */
 void TinyTIFFWriter_writeIFDEntrySHORT(TinyTIFFWriterFile* tiff, uint16_t tag, uint16_t data) {
    if (!tiff) return;
    if (tiff->lastIFDCount<TIFF_HEADER_MAX_ENTRIES) {
        tiff->lastIFDCount++;
        WRITEH16DIRECT(tiff, tag);
        WRITEH16(tiff, TIFF_TYPE_SHORT);
        WRITEH32(tiff, 1);
        WRITEH16DIRECT(tiff, data);
        WRITEH16(tiff, 0);
    }
}

/*! \brief write an 32-bit word IFD entry
    \ingroup tinytiffwriter_internal
    \internal

    \note This function writes into TinyTIFFFile::lastHeader, starting at the position TinyTIFFFile::pos
 */
 static void TinyTIFFWriter_writeIFDEntryLONG(TinyTIFFWriterFile* tiff, uint16_t tag, uint32_t data) {
    if (!tiff) return;
    if (tiff->lastIFDCount<TIFF_HEADER_MAX_ENTRIES) {
        tiff->lastIFDCount++;
        WRITEH16DIRECT(tiff, tag);
        WRITEH16(tiff, TIFF_TYPE_LONG);
        WRITEH32(tiff, 1);
        WRITEH32DIRECT(tiff, data);
    }
}

/*! \brief write an array of 32-bit words as IFD entry
    \ingroup tinytiffwriter_internal
    \internal

    \note This function writes into TinyTIFFFile::lastHeader, starting at the position TinyTIFFFile::pos
 */
 static void TinyTIFFWriter_writeIFDEntryLONGARRAY(TinyTIFFWriterFile* tiff, uint16_t tag, uint32_t* data, uint32_t N) {
    if (!tiff) return;
    if (tiff->lastIFDCount<TIFF_HEADER_MAX_ENTRIES) {
        tiff->lastIFDCount++;
        WRITEH16DIRECT(tiff, tag);
        WRITEH16(tiff, TIFF_TYPE_LONG);
        WRITEH32(tiff, N);
        if (N==1) {
            WRITEH32DIRECT(tiff, *data);
        } else {
            WRITEH32DIRECT(tiff, tiff->lastIFDDATAAdress+tiff->lastStartPos);
            int pos=tiff->pos;
            tiff->pos=tiff->lastIFDDATAAdress;
            for (uint32_t i=0; i<N; i++) {
                WRITEH32DIRECT(tiff, data[i]);
            }
            tiff->lastIFDDATAAdress=tiff->pos;
            tiff->pos=pos;
        }
    }
}

/*! \brief write an array of 16-bit words as IFD entry
    \ingroup tinytiffwriter_internal
    \internal

    \note This function writes into TinyTIFFFile::lastHeader, starting at the position TinyTIFFFile::pos
 */
static void TinyTIFFWriter_writeIFDEntrySHORTARRAY(TinyTIFFWriterFile* tiff, uint16_t tag, uint16_t* data, uint32_t N) {
    if (!tiff) return;
    if (tiff->lastIFDCount<TIFF_HEADER_MAX_ENTRIES) {
        tiff->lastIFDCount++;
        WRITEH16DIRECT(tiff, tag);
        WRITEH16(tiff, TIFF_TYPE_SHORT);
        WRITEH32(tiff, N);
        if (N==1) {
            WRITEH32DIRECT(tiff, *data);
        } else {
            WRITEH32DIRECT(tiff, tiff->lastIFDDATAAdress+tiff->lastStartPos);
            int pos=tiff->pos;
            tiff->pos=tiff->lastIFDDATAAdress;
            for (uint32_t i=0; i<N; i++) {
                WRITEH16DIRECT(tiff, data[i]);
            }
            tiff->lastIFDDATAAdress=tiff->pos;
            tiff->pos=pos;
        }


    }
}

/*! \brief write an array of characters (ASCII TEXT) as IFD entry
    \ingroup tinytiffwriter_internal
    \internal

    \note This function writes into TinyTIFFFile::lastHeader, starting at the position TinyTIFFFile::pos
 */
static void TinyTIFFWriter_writeIFDEntryASCIIARRAY(TinyTIFFWriterFile* tiff, uint16_t tag, const char* data, uint32_t N, int* datapos, int* sizepos) {
    if (!tiff) return;
    if (tiff->lastIFDCount<TIFF_HEADER_MAX_ENTRIES) {
        tiff->lastIFDCount++;
        WRITEH16DIRECT(tiff, tag);
        WRITEH16(tiff, TIFF_TYPE_ASCII);
        if (sizepos) *sizepos=tiff->pos;
        WRITEH32(tiff, N);
        if (N<4) {
            if (datapos) *datapos=tiff->pos;
            for (uint32_t i=0; i<4; i++) {
                if (i<N) {
                    WRITEH8DIRECT(tiff, data[i]);
                } else {
                    WRITEH8DIRECT(tiff, 0);
                }
            }
        } else {
            WRITEH32DIRECT(tiff, tiff->lastIFDDATAAdress+tiff->lastStartPos);
            int pos=tiff->pos;
            tiff->pos=tiff->lastIFDDATAAdress;
            if (datapos) *datapos=tiff->pos;
            for (uint32_t i=0; i<N; i++) {
                WRITEH8DIRECT(tiff, data[i]);
            }
            tiff->lastIFDDATAAdress=tiff->pos;
            tiff->pos=pos;
        }


    }
}

/*! \brief write a rational number as IFD entry
    \ingroup tinytiffwriter_internal
    \internal

    \note This function writes into TinyTIFFFile::lastHeader, starting at the position TinyTIFFFile::pos
 */
static void TinyTIFFWriter_writeIFDEntryRATIONAL(TinyTIFFWriterFile* tiff, uint16_t tag, uint32_t numerator, uint32_t denominator) {
    if (!tiff) return;
    if (tiff->lastIFDCount<TIFF_HEADER_MAX_ENTRIES) {
        tiff->lastIFDCount++;
        WRITEH16DIRECT(tiff, tag);
        WRITEH16(tiff, TIFF_TYPE_RATIONAL);
        WRITEH32(tiff, 1);
        WRITEH32DIRECT(tiff, tiff->lastIFDDATAAdress+tiff->lastStartPos);
        //printf("1 - %lx\n", tiff->pos);
        int pos=tiff->pos;
        tiff->pos=tiff->lastIFDDATAAdress;
        //printf("2 - %lx\n", tiff->pos);
        WRITEH32DIRECT(tiff, numerator);
        //printf("3 - %lx\n", tiff->pos);
        WRITEH32DIRECT(tiff, denominator);
        tiff->lastIFDDATAAdress=tiff->pos;
        tiff->pos=pos;
        //printf("4 - %lx\n", tiff->pos);
    }
}



TinyTIFFWriterFile* TinyTIFFWriter_open(const char* filename, uint16_t bitsPerSample, uint16_t samples, uint32_t width, uint32_t height) {
    TinyTIFFWriterFile* tiff=(TinyTIFFWriterFile*)malloc(sizeof(TinyTIFFWriterFile));

    //tiff->file=fopen(filename, "wb");
    TinyTIFFWriter_fopen(tiff, filename);
    tiff->width=width;
    tiff->height=height;
    tiff->samples=samples;
    tiff->bitspersample=bitsPerSample;
    tiff->lastHeader=NULL;
    tiff->lastHeaderSize=0;
    tiff->byteorder=TIFF_get_byteorder();
    tiff->frames=0;
    tiff->descriptionOffset=0;
    tiff->descriptionSizeOffset=0;
    tiff->lastStartPos=0;
    tiff->lastIFDDATAAdress=0;
    tiff->lastIFDCount=0;
    tiff->pos=0;
    memset(tiff->lastError, 0, TIFF_LAST_ERROR_SIZE);
    tiff->wasError=TINYTIFFWRITER_FALSE;

    if (TinyTIFFWriter_fOK(tiff)) {
        if (TIFF_get_byteorder()==TIFF_ORDER_BIGENDIAN) {
            WRITE8DIRECT(tiff, 'M');   // write TIFF header for big-endian
            WRITE8DIRECT(tiff, 'M');
        } else {
            WRITE8DIRECT(tiff, 'I');   // write TIFF header for little-endian
            WRITE8DIRECT(tiff, 'I');
        }
        WRITE16DIRECT_CAST(tiff, 42);
        tiff->lastIFDOffsetField=TinyTIFFWriter_ftell(tiff);//ftell(tiff->file);
        WRITE32DIRECT_CAST(tiff, 8);      // now write offset to first IFD, which is simply 8 here (in little-endian order)
        return tiff;
    } else {
        free(tiff);
        return NULL;
    }
}
void TinyTIFFWriter_close_withdescription(TinyTIFFWriterFile* tiff, const char* imageDescription) {
   if (tiff) {
        TinyTIFFWriter_fseek_set(tiff, tiff->lastIFDOffsetField);
        WRITE32DIRECT_CAST(tiff, 0);
#ifdef TINYTIFF_WRITE_COMMENTS
        if (tiff->descriptionOffset>0) {
          const size_t inlen=strlen(imageDescription);
          char description[TINYTIFFWRITER_DESCRIPTION_SIZE+1];
          memset(description, 0, TINYTIFFWRITER_DESCRIPTION_SIZE+1);

          if (inlen>0) {
              if (inlen<=TINYTIFFWRITER_DESCRIPTION_SIZE) {
#ifdef HAVE_STRCPY_S
                strcpy_s(description, TINYTIFFWRITER_DESCRIPTION_SIZE+1, imageDescription);
#else
                strcpy(description, imageDescription);
#endif
              } else {
#ifdef HAVE_MEMCPY_S
                  memcpy_s(description, TINYTIFFWRITER_DESCRIPTION_SIZE+1, imageDescription, TINYTIFFWRITER_DESCRIPTION_SIZE);
#else
                  memcpy(description, imageDescription, TINYTIFFWRITER_DESCRIPTION_SIZE);
#endif
              }
          } else {
#ifdef HAVE_SPRINTF_S
              sprintf_s(description, TINYTIFFWRITER_DESCRIPTION_SIZE+1, "TinyTIFFWriter_version=1.1\nimages=%ld", (unsigned long)(tiff->frames));
#else
              sprintf(description, "TinyTIFFWriter_version=1.1\nimages=%ld", (unsigned long)(tiff->frames));
#endif
		  }
          description[TINYTIFFWRITER_DESCRIPTION_SIZE-1]='\0';
          //const size_t dlen=strlen(description);

          //printf("WRITING COMMENT\n***");
          //printf(description);
          //printf("***\nlen=%ld\n\n", dlen);
          //printf("***\ninlen=%ld\n\n", inlen);
          TinyTIFFWriter_fseek_set(tiff, tiff->descriptionOffset);
          TinyTIFFWriter_fwrite(description, 1, TINYTIFFWRITER_DESCRIPTION_SIZE+1, tiff);//<<" / "<<dlen<<"\n";
          TinyTIFFWriter_fseek_set(tiff, tiff->descriptionSizeOffset);
          WRITE32DIRECT_CAST(tiff, (TINYTIFFWRITER_DESCRIPTION_SIZE+1));
        }
#endif // TINYTIFF_WRITE_COMMENTS
        TinyTIFFWriter_fclose(tiff);
        free(tiff->lastHeader);
        free(tiff);
    }
}

void TinyTIFFWriter_close_withmetadatadescription(TinyTIFFWriterFile* tiff, double pixel_width, double pixel_height, double frametime, double deltaz) {
    if (tiff) {
      char description[TINYTIFFWRITER_DESCRIPTION_SIZE+1];
      memset(description, 0, TINYTIFFWRITER_DESCRIPTION_SIZE+1);
      const int spwlen=256;
      char spw[256];
#ifdef HAVE_SPRINTF_S
      sprintf_s(description, TINYTIFFWRITER_DESCRIPTION_SIZE+1, "TinyTIFFWriter_version=1.1\nimages=%lu", (unsigned long int)tiff->frames);
#else
      sprintf(description, "TinyTIFFWriter_version=1.1\nimages=%lu", (unsigned long int)tiff->frames);
#endif
      if (fabs(pixel_width)>10.0*DBL_MIN) {
#ifdef HAVE_SPRINTF_S
          sprintf_s(spw, spwlen, "\npixel_width=%lf ", pixel_width);
#else
          sprintf(spw, "\npixel_width=%lf ", pixel_width);
#endif
#ifdef HAVE_STRCAT_S
          strcat_s(description, TINYTIFFWRITER_DESCRIPTION_SIZE+1, spw);
#else
          strcat(description,spw);
#endif
      }
      if (fabs(pixel_height)>10.0*DBL_MIN) {
#ifdef HAVE_SPRINTF_S
          sprintf_s(spw, spwlen, "\npixel_height=%lf ", pixel_height);
#else
          sprintf(spw, "\npixel_height=%lf ", pixel_height);
#endif
#ifdef HAVE_STRCAT_S
          strcat_s(description, TINYTIFFWRITER_DESCRIPTION_SIZE+1, spw);
#else
          strcat(description,spw);
#endif
      }
      if (fabs(deltaz)>10.0*DBL_MIN) {
#ifdef HAVE_SPRINTF_S
          sprintf_s(spw, spwlen, "\ndeltaz=%lf ", deltaz);
#else
          sprintf(spw, "\ndeltaz=%lf ", deltaz);
#endif
#ifdef HAVE_STRCAT_S
          strcat_s(description, TINYTIFFWRITER_DESCRIPTION_SIZE+1, spw);
#else
          strcat(description,spw);
#endif
      }
      if (fabs(frametime)>10.0*DBL_MIN) {
#ifdef HAVE_SPRINTF_S
          sprintf_s(spw, spwlen, "\nframetime=%lg ", frametime);
#else
          sprintf(spw, "\nframetime=%lg ", frametime);
#endif
#ifdef HAVE_STRCAT_S
          strcat_s(description, TINYTIFFWRITER_DESCRIPTION_SIZE+1, spw);
#else
          strcat(description,spw);
#endif

      }
      description[TINYTIFFWRITER_DESCRIPTION_SIZE]='\0';
      TinyTIFFWriter_close_withdescription(tiff, description);
    }
}


#ifdef HAVE_SPRINTF_S
#define TINTIFFWRITER_WRITEImageDescriptionTemplate(tiff) \
    if (tiff->frames<=0) {\
        int datapos=0;\
        int sizepos=0;\
        char description[TINYTIFFWRITER_DESCRIPTION_SIZE+1];\
        memset(description, 0, TINYTIFFWRITER_DESCRIPTION_SIZE+1);\
        sprintf_s(description, TINYTIFFWRITER_DESCRIPTION_SIZE+1, "TinyTIFFWriter_version=1.1\n");\
        description[TINYTIFFWRITER_DESCRIPTION_SIZE]='\0';\
        TinyTIFFWriter_writeIFDEntryASCIIARRAY(tiff, TIFF_FIELD_IMAGEDESCRIPTION, description, TINYTIFFWRITER_DESCRIPTION_SIZE, &datapos, &sizepos);\
        tiff->descriptionOffset=tiff->lastStartPos+datapos;\
        tiff->descriptionSizeOffset=tiff->lastStartPos+sizepos;\
     }
#else
#define TINTIFFWRITER_WRITEImageDescriptionTemplate(tiff) \
    if (tiff->frames<=0) {\
        int datapos=0;\
        int sizepos=0;\
        char description[TINYTIFFWRITER_DESCRIPTION_SIZE+1];\
        memset(description, 0, TINYTIFFWRITER_DESCRIPTION_SIZE+1);\
        sprintf(description, "TinyTIFFWriter_version=1.1\n");\
        description[TINYTIFFWRITER_DESCRIPTION_SIZE]='\0';\
        TinyTIFFWriter_writeIFDEntryASCIIARRAY(tiff, TIFF_FIELD_IMAGEDESCRIPTION, description, TINYTIFFWRITER_DESCRIPTION_SIZE, &datapos, &sizepos);\
        tiff->descriptionOffset=tiff->lastStartPos+datapos;\
        tiff->descriptionSizeOffset=tiff->lastStartPos+sizepos;\
     }
#endif



int TinyTIFFWriter_writeImage(TinyTIFFWriterFile* tiff, const void* data) {
    if (!tiff) {
        return TINYTIFFWRITER_FALSE;
    }
    long pos=TinyTIFFWriter_ftell(tiff);
    int hsize=TIFF_HEADER_SIZE;
#ifdef TINYTIFF_WRITE_COMMENTS
    if (tiff->frames<=0) {
        hsize=TIFF_HEADER_SIZE+TINYTIFFWRITER_DESCRIPTION_SIZE+1+16;
    }
#endif // TINYTIFF_WRITE_COMMENTS
    TinyTIFFWriter_startIFD(tiff,hsize);
    TinyTIFFWriter_writeIFDEntryLONG(tiff, TIFF_FIELD_IMAGEWIDTH, tiff->width);
    TinyTIFFWriter_writeIFDEntryLONG(tiff, TIFF_FIELD_IMAGELENGTH, tiff->height);
    TinyTIFFWriter_writeIFDEntrySHORT(tiff, TIFF_FIELD_BITSPERSAMPLE, tiff->bitspersample);
    TinyTIFFWriter_writeIFDEntrySHORT(tiff, TIFF_FIELD_COMPRESSION, 1);
    if (tiff->samples==1) {
        TinyTIFFWriter_writeIFDEntrySHORT(tiff, TIFF_FIELD_PHOTOMETRICINTERPRETATION, TIFF_PHOTOMETRIC_BLACKISZERO);
    } else if (tiff->samples>1) {
        TinyTIFFWriter_writeIFDEntrySHORT(tiff, TIFF_FIELD_PHOTOMETRICINTERPRETATION, TIFF_PHOTOMETRIC_RGB);
    }
#ifdef TINYTIFF_WRITE_COMMENTS
    TINTIFFWRITER_WRITEImageDescriptionTemplate(tiff);
#endif // TINYTIFF_WRITE_COMMENTS
    TinyTIFFWriter_writeIFDEntryLONG(tiff, TIFF_FIELD_STRIPOFFSETS, pos+2+hsize);
    TinyTIFFWriter_writeIFDEntrySHORT(tiff, TIFF_FIELD_SAMPLESPERPIXEL, tiff->samples);
    TinyTIFFWriter_writeIFDEntryLONG(tiff, TIFF_FIELD_ROWSPERSTRIP, tiff->height);
    TinyTIFFWriter_writeIFDEntryLONG(tiff, TIFF_FIELD_STRIPBYTECOUNTS, tiff->width*tiff->height*tiff->samples*(tiff->bitspersample/8));
    TinyTIFFWriter_writeIFDEntryRATIONAL(tiff, TIFF_FIELD_XRESOLUTION, 1,1);
    TinyTIFFWriter_writeIFDEntryRATIONAL(tiff, TIFF_FIELD_YRESOLUTION, 1,1);
    TinyTIFFWriter_writeIFDEntrySHORT(tiff, TIFF_FIELD_PLANARCONFIG, 1);
    TinyTIFFWriter_writeIFDEntrySHORT(tiff, TIFF_FIELD_RESOLUTIONUNIT, 1);
    TinyTIFFWriter_endIFD(tiff, hsize);
    TinyTIFFWriter_fwrite(data, tiff->width*tiff->height*tiff->samples*(tiff->bitspersample/8), 1, tiff);
    tiff->frames=tiff->frames+1;

    return TINYTIFFWRITER_TRUE;
}




void TinyTIFFWriter_close(TinyTIFFWriterFile *tiff)
{
    TinyTIFFWriter_close_withdescription(tiff, "");
}

const char *TinyTIFFWriter_getLastError(TinyTIFFWriterFile *tiff)
{
    if (tiff) return tiff->lastError;
    return NULL;
}

int TinyTIFFWriter_wasError(TinyTIFFWriterFile *tiff)
{
    if (tiff) return tiff->wasError;
    return TINYTIFFWRITER_TRUE;
}

int TinyTIFFWriter_success(TinyTIFFWriterFile *tiff)
{
    if (tiff) return !tiff->wasError;
    return TINYTIFFWRITER_FALSE;
}
