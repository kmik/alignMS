#ifndef METADATA_READER_H
#define METADATA_READER_H

#include <thread>
#include <mutex>
#include <ExifTool.h>
#include <ExifToolPipe.h>
#include <TagInfo.h>
#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <queue>

class metadata_reader
{
    public:


        metadata_reader();
        virtual ~metadata_reader();

        void startReadingMetadata();
        void endReadingMetadata();

        void startWritingImage();
        void endWritingImage();

        std::string identifier = "CaptureId";
        std::string channelName = "BandName";
        std::string paneeli = "PanelSerial";

        std::string red = "Red";
        std::string green = "Green";
        std::string blue = "Blue";
        std::string nir = "NIR";
        std::string red_edge = "Red edge";
        std::string lwir = "LWIR";

        void readImageIdentifier(TagInfo *info, std::string imageName, int &channel, int &isPanel, std::string &imgId){

            int counter = 0;
            std::string output;

            isPanel = 0;
            int found = 0;

            if (info) {
                for (TagInfo *i=info; i; i=i->next) {


                        if(found == 3)
                            return;

                        int result1 = strcmp(identifier.c_str(), i->name);
                        int result2 = strcmp(channelName.c_str(), i->name);
                        int result3 = strcmp(paneeli.c_str(), i->name);


                        if(!result1){


                            found++;
                            imgId = i->value;

                        }


                        if(!result2){

                            if(!strcmp(red.c_str(), i->value))
                                channel = 2;
                            else if(!strcmp(blue.c_str(), i->value))
                                channel = 0;
                            else if(!strcmp(green.c_str(), i->value))
                                channel = 1;
                            else if(!strcmp(red_edge.c_str(), i->value))
                                channel = 4;
                            else if(!strcmp(nir.c_str(), i->value))
                                channel = 3;
                            else
                                channel = 5;

                            found++;

                        }

                        if(!result3){
                            found++;
                            isPanel = 1;
                        }
                    //}
                    counter++;
                }
            }
        }

    protected:

    private:
};

#endif // METADATA_READER_H
