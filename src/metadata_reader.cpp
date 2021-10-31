#include "metadata_reader.h"

std::mutex m;
std::mutex m2;

metadata_reader::metadata_reader()
{
    //ctor
}

metadata_reader::~metadata_reader()
{
    //dtor
}

void metadata_reader::startReadingMetadata(){

    m.lock();


}

void metadata_reader::endReadingMetadata(){

    m.unlock();


}

void metadata_reader::startWritingImage(){

    m2.lock();


}

void metadata_reader::endWritingImage(){

    m2.unlock();


}
