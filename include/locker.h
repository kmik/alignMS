#ifndef LOCKER_H
#define LOCKER_H


#include <thread>
#include <mutex>
#include <ExifTool.h>
#include <ExifToolPipe.h>
#include <TagInfo.h>
#include <stdio.h>



class locker
{
    public:
        locker();
        virtual ~locker();

        void waitPermission();

    protected:

    private:
};

#endif // LOCKER_H
