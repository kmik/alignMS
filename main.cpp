#include <opencv4/opencv2/core.hpp>

#include <opencv2/highgui.hpp>

#include <opencv2/imgcodecs.hpp>
#include <string>
#include <vector>
#include <dirent.h>
#include <iostream>
#include <altumImage.h>
#include <fstream>
#include <stdio.h>
#include <map>
#include <unordered_set>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <progress.h>
//#define DEBUG
#include <libgen.h>
#include <unistd.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <calibrationPanels.h>
#include <ExifTool.h>
#include <ExifToolPipe.h>
#include <TagInfo.h>
#include <omp.h>
#include <metadata_reader.h>

#include <locker.h>
#include <pthread.h>
#include <queue.h>
#include <thread>

const int META_N_img = 20;



std::ofstream log_file;
std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

inline bool output_composite = false;
bool job_done = false;
//#define DEBUG

using namespace cv;



struct plotItem{

    long time;
    bool panelOrNot;
    float ir1, ir2, ir3, ir4, ir5;

};

bool metadataWriterThread(blocking_queue<metaItem> *q, ExifTool *et2, std::string exfPath){

    bool terminate = false;
    metaItem m;
    std::string commandi = "";

    std::string outputFile = "";
    TagInfo *info;
    int cmdNum, cmdNum2;
    std::string commandi2 = "-stay_open";

    std::vector<std::string> froms;
    std::vector<std::string> tos;

    int metadatacount = 0;

    while(true){

        if(et2->IsRunning() == 0){

            std::cout << "SHOULD NOT BE HERE WHAT THE HELL!!" << std::endl;
            delete et2;

            et2 = new ExifTool(exfPath.c_str());
        }

        commandi = "";
        outputFile = "";
/*
        if(job_done && q->empty())
            break;
*/
        if(q->empty() ){

            if(job_done)
                break;
            else
                continue;

        }

        q->pop(m);

        std::cout << " POPPED " << m.src << std::endl;


                froms.push_back(m.src);
                tos.push_back(m.dst);




        commandi += "-tagsfromfile\n";
        commandi += m.src;
        commandi += "\n-all:all";
        commandi += "\n-xmp";
        commandi += "\n-overwrite_original";
        commandi += "\n-stay_open";

        et2->ExtractInfo(m.src.c_str(), commandi2.c_str());
        info = et2->ImageInfo(m.src.c_str(), commandi2.c_str());

        cmdNum = et2->WriteInfo(m.dst.c_str(), commandi.c_str());
        cmdNum2 = et2->Complete();

        //std::cout << commandi << std::endl;

        while(cmdNum2 <= 0){

            cmdNum = et2->WriteInfo(m.dst.c_str(), commandi.c_str());
            cmdNum2 = et2->Complete();

                //std::cout << "EXIFTOOL ERROR!!!" << std::endl;

        }

        delete info;

        metadatacount++;

        std::cout << "written " << metadatacount << " files meta" << std::endl;
        std::cout << "written " << metadatacount << " files meta" << std::endl;
        std::cout << "written " << metadatacount << " files meta" << std::endl;
        //if(job_done && q->empty()){
        //if(terminate){
        /*
        if(froms.size() >= 10){

            std::cout << " STARTING TO WRITE METADATA!  " << std::endl;
            std::string desti = tos[0];
            std::string source = froms[0];

            commandi += froms[0];
            commandi += "\n-all:all";
            commandi += "\n-xmp";
            commandi += "\n-overwrite_original";

            for(int i = 1; i < froms.size(); i++){

                desti += "\n" + tos[i];
                source += "\n" + froms[i];
                commandi += "\n-tagsfromfile\n";
                commandi += froms[i];
                commandi += "\n-all:all";
                commandi += "\n-xmp";
                commandi += "\n-overwrite_original";

            }

            std::cout << " GOT HERE ! " << std::endl;

            et2->ExtractInfo(source.c_str(), commandi2.c_str());
            info = et2->ImageInfo(source.c_str(), commandi2.c_str());


            //et2->ExtractInfo(m.src.c_str(), commandi2.c_str());
            //info = et2->ImageInfo(m.src.c_str(), commandi2.c_str());

            //commandi += "-tagsfromfile\n";
            //commandi += m.src;
            //commandi += "\n-all:all";

            //commandi += "\n-stay_open";
            //std::cout << " " << std::endl;
            //std::cout << commandi << std::endl;
            //std::cout << source << std::endl;
            //std::cout << desti << std::endl;
            //cmdNum = et2->WriteInfo(m.dst.c_str(), commandi.c_str());
            cmdNum = et2->WriteInfo(desti.c_str(), commandi.c_str());
            cmdNum2 = et2->Complete();


            //exit(1);

            while(cmdNum2 <= 0){

                cmdNum = et2->WriteInfo(desti.c_str(), commandi.c_str());
                cmdNum2 = et2->Complete();

                //std::cout << "EXIFTOOL ERROR!!!" << std::endl;

            }
            metadatacount += froms.size();

            std::cout << "WROTE METADATRA to: \n" << desti << std::endl;
            std::cout << "n_metadata:  " << metadatacount << std::endl;

            std::cout << commandi << std::endl;
            std::cout << source << std::endl;
            std::cout << desti << std::endl;

            froms.clear();
            tos.clear();

            delete info;
        }


        std::cout << "done writing; qque size: " << q->n_() << " " <<  froms.size() << " " << job_done << std::endl;

        if(terminate == true){

            break;

        }
        //if(job_done && q->empty())
          //  break;
    }
    */
/*
    if(froms.size() > 0){

        commandi = "";
        commandi += "-tagsfromfile\n";
        std::string desti = tos[0];
            //std::string source = from[0];
            //std::string desti = tos[0];
            std::string source = froms[0];

            commandi += froms[0];
            commandi += "\n-all:all";
            commandi += "\n-xmp";
            commandi += "\n-overwrite_original";

            for(int i = 1; i < froms.size(); i++){

                desti += "\n" + tos[i];
                source += "\n" + froms[i];
                commandi += "\n-tagsfromfile\n";
                commandi += froms[i];
                commandi += "\n-all:all";
                commandi += "\n-xmp";
                commandi += "\n-overwrite_original";

            }

            et2->ExtractInfo(source.c_str(), commandi2.c_str());
            info = et2->ImageInfo(source.c_str(), commandi2.c_str());


            //et2->ExtractInfo(m.src.c_str(), commandi2.c_str());
            //info = et2->ImageInfo(m.src.c_str(), commandi2.c_str());

            //commandi += "-tagsfromfile\n";
            //commandi += m.src;
            //commandi += "\n-all:all";

            //commandi += "\n-stay_open";
            //std::cout << " " << std::endl;
            //std::cout << commandi << std::endl;
            //std::cout << source << std::endl;
            //std::cout << desti << std::endl;
            //cmdNum = et2->WriteInfo(m.dst.c_str(), commandi.c_str());
            cmdNum = et2->WriteInfo(desti.c_str(), commandi.c_str());
            cmdNum2 = et2->Complete(1);

             froms.clear();
            tos.clear();
            //exit(1);

            while(cmdNum2 <= 0){

                cmdNum = et2->WriteInfo(desti.c_str(), commandi.c_str());
                cmdNum2 = et2->Complete(1);

                std::cout << "EXIFTOOL ERROR!!!" << std::endl;

            }

            metadatacount += froms.size();

            std::cout << "WROTE REMAINING METADATRA to: \n" << desti << std::endl;
            std::cout << "n_metadata:  " << metadatacount << std::endl;
               froms.clear();
            tos.clear();


    }
*/
    }
    return true;

}



void printHelp(){

    std::cout << "\nAltumAlign 0.1.0" << std::endl;
    std::cout << "Mikko Kukkonen <mikko.kukkonen@uef.fi>\n" << std::endl;
    std::cout << "  ##########################################" << std::endl;
    std::cout << "  -  This tool is used to align MicaSense  -" << std::endl;
    std::cout << "  -   Altum images from separate channels  -" << std::endl;
    std::cout << "  -     Aligned images are output in the   -" << std::endl;
    std::cout << "  -   directory where the original images  -" << std::endl;
    std::cout << "  -   are located (in sub-directory named  -" << std::endl;
    std::cout << "  -               \"aligned\")             -" << std::endl;
    std::cout << "  ##########################################\n" << std::endl;


    std::cout << "-- Mandatory arguments --\n" << std::endl;

    std::cout << " -i [ --images ] <String>\tImage directory" << std::endl;
    std::cout << " -p [ --panel ]  <String>\tCalibration panel " << std::endl;

    std::cout << " " << std::endl;
    std::cout << "-- Optional arguments -- default value in parenthesis ()\n" << std::endl;

    std::cout << " -t [ --trim ]        <Integer>\tCrop black edges (0 - 100) " << std::endl;
    std::cout << " -d [ --def_panel ]   <String>\tPanel image id file " << std::endl;
    std::cout << " -f [ --fuse ]        <String>\tMerge the result " << std::endl;
    std::cout << " -c [ --cores ]       <Integer>\tNumber of CPU threads used (6) " << std::endl;
    std::cout << " -t [ --trim ]        <Integer>\tCrop black edges (0 - 100) " << std::endl;
    std::cout << " -r [ --reflectance ] <None>\tOutput reflectance " << std::endl;
    std::cout << " -b [ --both ]        <None>\tOutput reflectance and RAW " << std::endl;
    std::cout << " -m [ --mode ]        <Integer>\tReflectance mode (1) " << std::endl;
    std::cout << "                          \t   1 = Panel            " << std::endl;
    std::cout << "                          \t   2 = DLS+panel        " << std::endl;
    std::cout << "                          \t   3 = DLS              " << std::endl;
    std::cout << "                          \t   4 = Adaptive*        " << std::endl;
    std::cout << " -s [ --source ]         <Integer>\tReference channel 0 - 4 (4)" << std::endl;
    std::cout << "                          \t   0 = Blue   1 = Green " << std::endl;
    std::cout << "                          \t   2 = Red    3 = NiR   " << std::endl;
    std::cout << "                          \t   4 = Red Edge         " << std::endl;
    std::cout << "" << std::endl;
    std::cout << " *Adaptive means that primarily only panels are used." << std::endl;
    std::cout << "  If, however, the DLS irradiances depict high" << std::endl;
    std::cout << "  standard deviation, both DLS and panels are used." << std::endl;



}

void process_mem_usage(double& vm_usage, double& resident_set)
{

   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0.0;
   resident_set = 0.0;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   //
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   //
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / 1024.0;
   resident_set = rss * page_size_kb;
}


void readCalibrationFiles(std::vector<cv::Mat> &distortion, std::vector<cv::Mat> &cameraMatrix,
                            std::string directory){
    //cv::Mat(5,1,CV_64FC1, 0.0);

    //cv::Mat(cv::Mat(3,3, CV_64FC1, 0.0));
    for(int i = 0; i < 5; i++){

        distortion.push_back(cv::Mat(5,1,CV_64FC1, 0.0));
        cameraMatrix.push_back(cv::Mat(3,3, CV_64FC1, 0.0));
    }

    DIR *dr;
    struct dirent *en;
    dr = opendir(directory.c_str());

    if (dr) {
      while ((en = readdir(dr)) != NULL) {

        //en->d_name;

        int id;
        int channel;

        std::string fullFilePath(directory);
        fullFilePath += "/";
        fullFilePath += en->d_name;

        std::stringstream filename(en->d_name);
        std::string segment;
        std::vector<std::string> seglist;

        while(std::getline(filename, segment, '_'))
        {
           seglist.push_back(segment);
          // std::cout << " \n" << segment;
        }

        if(seglist.size() <= 1)
            continue;


//        std::cout << "seglist size: " << seglist.size() << std::endl;
        id = std::stoi( seglist[1] );
        //channel = std::stoi( seglist[2] );
        //std::cout << filename.str() << std::endl;
        std::ifstream infile(fullFilePath);
        double dist_1, dist_2, dist_3, dist_4, dist_5;
        double cam_1, cam_2,cam_3, cam_4, cam_5, cam_6, cam_7, cam_8, cam_9;
        infile >> dist_1 >> dist_2 >> dist_3 >> dist_4 >> dist_5;
        infile >> cam_1 >> cam_2 >> cam_3 >> cam_4 >> cam_5 >> cam_6 >> cam_7 >> cam_8 >> cam_9;

        distortion[id-1].at<double>(0,0) = dist_1;
        distortion[id-1].at<double>(0,1) = dist_2;
        distortion[id-1].at<double>(0,2) = dist_3;
        distortion[id-1].at<double>(0,3) = dist_4;
        distortion[id-1].at<double>(0,4) = dist_5;

        cameraMatrix[id-1].at<double>(0,0) = cam_1;
        cameraMatrix[id-1].at<double>(0,1) = cam_2;
        cameraMatrix[id-1].at<double>(0,2) = cam_3;

        cameraMatrix[id-1].at<double>(1,0) = cam_4;
        cameraMatrix[id-1].at<double>(1,1) = cam_5;
        cameraMatrix[id-1].at<double>(1,2) = cam_6;

        cameraMatrix[id-1].at<double>(2,0) = cam_7;
        cameraMatrix[id-1].at<double>(2,1) = cam_8;
        cameraMatrix[id-1].at<double>(2,2) = cam_9;

      }
      closedir(dr); //close all directory
    }
/*
    for(int i = 0; i < 5; i++){


        std::cout << distortion[i] << std::endl;


        std::cout << "------------------" << std::endl;


        std::cout << cameraMatrix[i] << std::endl;


        std::cout << "####################" << std::endl;
    }
*/
}

void readPanelImageIds(std::unordered_set<std::string> &id_set, std::string file){

    std::ifstream infile(file);
    std::string id;

    if(!infile.good()){

                std::cout << "input panel id file not good!" << std::endl;
                exit(1);

    }

    while (std::getline(infile, id)) {
    // output the line
        std::cout << id << std::endl;
        id_set.insert(id);
    // now we loop back and get the next line in 'str'
    }

}

void drawPlot(std::map<long, plotItem> plot_map){

    long minTime = LONG_MAX, maxTime = LONG_MIN;
    float maxIrra1 = FLT_MIN, minIrra1 = FLT_MAX;
    float maxIrra2 = FLT_MIN, minIrra2 = FLT_MAX;
    float maxIrra3 = FLT_MIN, minIrra3 = FLT_MAX;
    float maxIrra4 = FLT_MIN, minIrra4 = FLT_MAX;
    float maxIrra5 = FLT_MIN, minIrra5 = FLT_MAX;

    for(auto i : plot_map){

        if(i.second.time < minTime)
            minTime = i.second.time;
        if(i.second.time > maxTime)
            maxTime = i.second.time;

        if(i.second.ir1 > maxIrra1)
            maxIrra1 = i.second.ir1;
        if(i.second.ir2 > maxIrra2)
            maxIrra2 = i.second.ir2;
        if(i.second.ir3 > maxIrra3)
            maxIrra3 = i.second.ir3;
        if(i.second.ir4 > maxIrra4)
            maxIrra4 = i.second.ir4;
        if(i.second.ir5 > maxIrra5)
            maxIrra5 = i.second.ir5;

        if(i.second.ir1 < minIrra1)
            minIrra1 = i.second.ir1;
        if(i.second.ir2 < minIrra2)
            minIrra2 = i.second.ir2;
        if(i.second.ir3 < minIrra3)
            minIrra3 = i.second.ir3;
        if(i.second.ir4 < minIrra4)
            minIrra4 = i.second.ir4;
        if(i.second.ir5 < minIrra5)
            minIrra5 = i.second.ir5;

    }

    double binSize = ((double)(maxTime - minTime) / 30.0);
    double irradiBinSize1 = ((double)(maxIrra1 - minIrra1) / 10.0);
    double irradiBinSize2 = ((double)(maxIrra2 - minIrra2) / 10.0);
    double irradiBinSize3 = ((double)(maxIrra3 - minIrra3) / 10.0);
    double irradiBinSize4 = ((double)(maxIrra4 - minIrra4) / 10.0);
    double irradiBinSize5 = ((double)(maxIrra5 - minIrra5) / 10.0);

    std::vector<double> binit1, binit2, binit3, binit4, binit5;
    std::vector<int> bincounts;

    std::vector<int> containsPanel;

    for(int i = 0; i < 31; i++){

        binit1.push_back(0);
        binit2.push_back(0);
        binit3.push_back(0);
        binit4.push_back(0);
        binit5.push_back(0);
        bincounts.push_back(0);
        containsPanel.push_back(0);

    }

    for(auto i : plot_map){

        int bini = (int) ((double)(i.second.time - minTime) / binSize);

        binit1[bini] += i.second.ir1;
        binit2[bini] += i.second.ir2;
        binit3[bini] += i.second.ir3;
        binit4[bini] += i.second.ir4;
        binit5[bini] += i.second.ir5;

        bincounts[bini]++;
        //std::cout << "count " << bincounts[bini] << std::endl;
        if(i.second.panelOrNot){
/*
            std::cout << " PANEL !!!!! " << bini << std::endl;
            std::cout << " PANEL !!!!! " << bini << std::endl;
            std::cout << " PANEL !!!!! " << bini << std::endl;
            std::cout << " PANEL !!!!! " << bini << std::endl;
            std::cout << binit1[bini] << std::endl;
            */
            containsPanel[bini] = 1;

        }
    }

    for(int i = 0; i < 31; i++){

        if(bincounts[i] == 0)
            bincounts[i] = 1;

        //std::cout << "binival " << i << " " << binit1[i] << " counts: " << bincounts[i] << std::endl;

        binit1[i] /= (double)bincounts[i];
        binit1[i] = (int)((binit1[i] - minIrra1) / irradiBinSize1);
        //std::cout << binit1[i] << " " << minIrra1 << " " << maxIrra1 << " " << irradiBinSize1 << std::endl;
        //std::cout << "binival " << i << " " << binit1[i] << " counts: " << bincounts[i] << std::endl;

        binit2[i] /= (double)bincounts[i];
        binit2[i] = (int)((binit2[i] - minIrra2) / irradiBinSize2);


        binit3[i] /= (double)bincounts[i];
        binit3[i] = (int)((binit3[i] - minIrra3) / irradiBinSize3);

        //std::cout << "binival " << i << " " << binit4[i] << " counts: " << bincounts[i] << std::endl;
        binit4[i] /= (double)bincounts[i];
        binit4[i] = (int)((binit4[i] - minIrra4) / irradiBinSize4);
        //std::cout << binit4[i] << " " << minIrra4 << " " << maxIrra4 << " " << irradiBinSize4 << std::endl;
        //std::cout << "binival " << i << " " << binit4[i] << " counts: " << bincounts[i] << std::endl;
        binit5[i] /= (double)bincounts[i];
        binit5[i] = (int)((binit5[i] - minIrra5) / irradiBinSize5);


    }

    log_file << "\nChannel 1 irradiances:\n";

    for(int i = 10; i >= 0; i--){
        if(i == 10)
         log_file << "\n" << std::setprecision(3) << maxIrra1 << "\t|";
        else if(i == 0)
           log_file << "\n" << std::setprecision(3) << minIrra1 << "\t|";
        else
            log_file << "\n" << "\t|";
        for(int j = 0; j < 31; j++){

            //std::cout << i << " " << j << " Bin value: " << binit1[j] << std::endl;
            if(binit1[j] == i){

                if(containsPanel[j] == 1)
                    log_file << "*";
                else
                    log_file << "-";

            }else{

                log_file << " ";

            }

        }

    }
    log_file << "\n\t|";
    for(int i = 0; i < 31; i++)
        log_file << "_";
    log_file << "\n";

    log_file << "\nChannel 2 irradiances:\n";

    for(int i = 10; i >= 0; i--){
         if(i == 10)
         log_file << "\n" << std::setprecision(3) << maxIrra2 << "\t|";
        else if(i == 0)
           log_file << "\n" << std::setprecision(3) << minIrra2 << "\t|";
        else
            log_file << "\n" << "\t|";
        for(int j = 0; j < 31; j++){

            //std::cout << i << " " << j << " Bin value: " << binit1[j] << std::endl;
            if(binit2[j] == i){

                if(containsPanel[j] == 1)
                    log_file << "*";
                else
                    log_file << "-";

            }else{

                log_file << " ";

            }

        }

    }
    log_file << "\n\t|";
    for(int i = 0; i < 31; i++)
        log_file << "_";
    log_file << "\n";

    log_file << "\nChannel 3 irradiances:\n";

    for(int i = 10; i >= 0; i--){
         if(i == 10)
         log_file << "\n" << std::setprecision(3) << maxIrra3 << "\t|";
        else if(i == 0)
           log_file << "\n" << std::setprecision(3) << minIrra3 << "\t|";
        else
            log_file << "\n" << "\t|";
        for(int j = 0; j < 31; j++){

            //std::cout << i << " " << j << " Bin value: " << binit1[j] << std::endl;
            if(binit3[j] == i){

                if(containsPanel[j] == 1)
                    log_file << "*";
                else
                    log_file << "-";

            }else{

                log_file << " ";

            }

        }

    }
    log_file << "\n\t|";
    for(int i = 0; i < 31; i++)
        log_file << "_";
    log_file << "\n";

    log_file << "\nChannel 4 irradiances:\n";

    for(int i = 10; i >= 0; i--){
         if(i == 10)
         log_file << "\n" << std::setprecision(3) << maxIrra4 << "\t|";
        else if(i == 0)
           log_file << "\n" << std::setprecision(3) << minIrra4 << "\t|";
        else
            log_file << "\n" << "\t|";

        for(int j = 0; j < 31; j++){

            //std::cout << i << " " << j << " Bin value: " << binit1[j] << std::endl;
            if(binit4[j] == i){

                if(containsPanel[j] == 1)
                    log_file << "*";
                else
                    log_file << "-";

            }else{

                log_file << " ";

            }

        }

    }
    log_file << "\n\t|";
    for(int i = 0; i < 31; i++)
        log_file << "_";
    log_file << "\n";

    log_file << "\nChannel 5 irradiances:\n";

    for(int i = 10; i >= 0; i--){
         if(i == 10)
         log_file << "\n" << std::setprecision(3) << maxIrra5 << "\t|";
        else if(i == 0)
           log_file << "\n" << std::setprecision(3) << minIrra5 << "\t|";
        else
            log_file << "\n" << "\t|";
        for(int j = 0; j < 31; j++){

            //std::cout << i << " " << j << " Bin value: " << binit1[j] << std::endl;
            if(binit5[j] == i){

                if(containsPanel[j] == 1)
                    log_file << "*";
                else
                    log_file << "-";

            }else{

                log_file << " ";

            }

        }

    }
    log_file << "\n" << "\t|";
    for(int i = 0; i < 31; i++)
        log_file << "_";
    log_file << "\n";
}

int main(int argc, char** argv)
{


    cv::QRCodeDetector qrDecoder = cv::QRCodeDetector();

    log_file.open("loger.txt", std::ios_base::app);
    auto start = std::chrono::system_clock::now();

    //std::string curTime = date::format("%F %T", std::chrono::system_clock::now());

    std::time_t start_time = std::chrono::system_clock::to_time_t(start);

    log_file << "\n-- " << std::ctime(&start_time);



    // Define the path to executable
    char buff[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);

    std::string pahi;
    if (len != -1) {
      buff[len] = '\0';
      pahi = dirname(buff);
    }

    int crop = 3;

    int referenceImage = 3;
    int mode = 1;
    int format = 1;

    bool outputComposite = false;

    int composite = 123;

    std::vector<int> compositeBandOrder;

    std::string panelFile = "panel.txt";

    std::ifstream infile(panelFile);

    std::vector<altumImage*> altumImages;
    std::string imgDirectory = "test_full/;test_full/subDir/";

    //imgDirectory = "/media/koomikko/LaCie/liperi_2020_photogrammetry/Altum/3_done/ORIG/0020SET/000/";

    std::string distortionDirectory = "calib/";
    std::string panel_Id_file = "nulli";
    namespace po = boost::program_options;
    po::variables_map vm;
    const size_t ERROR_IN_COMMAND_LINE = 1;
    const size_t SUCCESS = 0;
    std::set<std::string> missingParams;


    std::string exftool = pahi;
    exftool += "/exifTool/exiftool";
    ExifTool *et = new ExifTool(exftool.c_str());

    std::string exftool2 = pahi;
    exftool2 += "/exifTool2/exiftool";
    ExifTool *et2 = new ExifTool(exftool2.c_str());

    blocking_queue<metaItem> *q = new blocking_queue<metaItem>(1000);

    std::thread th1(metadataWriterThread, q, et2, exftool2);
    metadata_reader *meta = new metadata_reader();


    std::ifstream panel_id_file;

    int n_cores = 4;

    bool undisti = false;


    po::options_description desc("Option arguments");
    desc.add_options()
      ("help, h", "Print help messages")
      ("images,i", po::value<std::string>(), "Directory to read images from")
      ("def_panels,d", po::value<std::string>(), "Panel ID file")
      ("fuse,f", po::value<int>(), "Merge result")
      ("cores,c", po::value<int>(), "Number of CPU threads used")
      ("undistort,u", "Undistort images")
      ("trim,t", po::value<int>(), "Crop black edges (%)")
      ("panel,p", po::value<std::string>(), "Panel reflectance values")
      ("mode,m", po::value<int>(), "Panel reflectance values")
      ("reflectance,r", "Output reflectance")
      ("both,b", "Output reflectance and RAW")
      ("source,s", po::value<int>(), "Reference image");

        try{
        po::store(po::parse_command_line(argc, argv, desc),
                vm); // can throw

        /** --help option
        */
        if ( vm.count("help")  ){

            printHelp();

            return SUCCESS;
        }

        if ( vm.count("help")  ){

            //calibrate = true;
        }

        if ( vm.count("images") ) {
            imgDirectory = vm["images"].as<std::string>();
        }else{
            missingParams.insert(" Image directory < --images [-i] > REQUIRED!\n");
        }

        if ( vm.count("fuse") ) {

            output_composite = true;

            outputComposite = true;
            composite = vm["fuse"].as<int>();
            int composite_temp = composite;
            int digits = 0; while (composite_temp != 0) { composite_temp /= 10; digits++; }

            int division = 1;
            std::cout << "digits: " << digits << std::endl;

            for(int i = 0; i < digits; i++){

                compositeBandOrder.push_back((composite / division % 10)-1);
                std::cout << (composite / division % 10) << std::endl;
                division *= 10;

            }

        }

        if ( vm.count("panel") ) {
            panelFile = vm["panel"].as<std::string>();
            std::ifstream infile(panelFile);
            if(!infile.good()){

                std::cout << "input panel file not good!" << std::endl;
                exit(1);

            }
        }else{
            missingParams.insert(" Panel reflectance values < --panel [-p] > REQUIRED!\n");
        }

        if ( vm.count("source") ) {
            referenceImage = vm["source"].as<int>();
        }
        if ( vm.count("mode") ) {

            mode = vm["mode"].as<int>();
        }
        if ( vm.count("cores") ) {

            n_cores = vm["cores"].as<int>();
        }
        if ( vm.count("reflectance") ) {

            //format = vm["format"].as<int>();
            format = 2;
            //std::cout << " setting: " << format << std::endl;
        }
        if ( vm.count("both") ) {

            //format = vm["format"].as<int>();
            format = 3;
            //std::cout << " setting: " << format << std::endl;
        }
        if ( vm.count("undistort") ) {

            //format = vm["format"].as<int>();
            undisti = true;
            //std::cout << " setting: " << format << std::endl;
        }


        if ( vm.count("trim") ) {

            //format = vm["format"].as<int>();
            //std::cout << " setting1: " << crop << std::endl;
            crop = vm["trim"].as<int>();;
            std::cout << " setting2: " << crop << std::endl;
        }

        if ( vm.count("def_panels") ) {

            //format = vm["format"].as<int>();
            panel_Id_file = vm["def_panels"].as<std::string>();;
            std::cout << " setting: " << panel_Id_file << std::endl;
        }


    }



    catch(po::error& e){
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
      printHelp();
      return ERROR_IN_COMMAND_LINE;
    }

    std::cout << compositeBandOrder.size() << std::endl;

    //format = 1;
    struct stat info;

    std::vector<std::string> directories;
    std::string segment;
    std::stringstream test(imgDirectory);

    while(std::getline(test, segment, ';')){

        //std::cout << segment << std::endl;
        directories.push_back(segment);

    }
    //std::cout << directories.size() << std::endl;
    //std::cout << imgDirectory << std::endl;
    //exit(1);

    bool cannotAccesAllDirectories = false;
    bool notAdirectory = false;
    std::string whichNotDirectory;
    std::string whichDirectory;

    for(auto i : directories){
        //std::cout << i << std::endl;
        if(stat( i.c_str(), &info ) != 0){
            whichDirectory = i;
            cannotAccesAllDirectories = true;
            break;
        }else if( info.st_mode & S_IFDIR ){



        }else{

            notAdirectory = true;
            whichNotDirectory = i;
            break;

        }
    }


    if( notAdirectory ){
        printf( "Specified image directory \"%s\" is not a directory\n", whichNotDirectory.c_str() );
        exit(1);
    }

    if( cannotAccesAllDirectories ){
        printf( "Cannot access specified image directory \"%s\"\n", whichDirectory.c_str() );
        exit(1);
    }
    /*
    else if( info.st_mode & S_IFDIR ){  // S_ISDIR() doesn't exist on my windows
        //printf( "%s is a directory\n", imgDirectory.c_str() );
    }
    else{
        printf( "Specified image directory \"%s\" is not a directory\n", imgDirectory.c_str() );
        exit(1);
    }

    if( stat( distortionDirectory.c_str(), &info ) != 0 ){
        printf( "Cannot access specified calibration directory \"%s\"\n", distortionDirectory.c_str() );
        exit(1);
    }
    else if( info.st_mode & S_IFDIR ){  // S_ISDIR() doesn't exist on my windows
        //printf( "%s is a directory\n", imgDirectory.c_str() );
    }
    else{
        printf( "Specified image directory \"%s\" is not a directory\n", distortionDirectory.c_str() );
        exit(1);
    }
    */

    std::vector<double> panelReflectances;
    double v;
    while(infile >> v){

        panelReflectances.push_back(v);

    }

    //std::cout << panelReflectances.size() << std::endl;

    std::vector<cv::Mat> distortion;
    std::vector<cv::Mat> cameraMatrix;

    //cv::Mat distortion(5,1,CV_64FC1, 0.0);

    //cv::Mat cameraMatrix(cv::Mat(3,3, CV_64FC1, 0.0));

    readCalibrationFiles(distortion, cameraMatrix, distortionDirectory);

    #ifdef DEBUG
    if(missingParams.size() > 0){

        for(auto a : missingParams){

            std::cout << a;

        }
        std::cout << "" << std::endl;
        std::cerr << desc << std::endl;
        return ERROR_IN_COMMAND_LINE;

    }
    #endif // DEBUG

    std::vector<DIR*> dir_vec;

    for(int i = 0; i < directories.size(); i++){

        DIR *dr_temp;
        dr_temp = opendir(directories[i].c_str());
        dir_vec.push_back(dr_temp);
    }

    DIR *dr;
    struct dirent *en;
    dr = opendir(directories[0].c_str()); //open all directory

    std::string y(directories[0]);

    std::string path("aligned");

    std::string newDirectory = y + path;
    std::string newDirectory2(newDirectory);


    for (int i = 2; boost::filesystem::exists(newDirectory2) && i < 10; ++i) {
      std::stringstream ss;
      ss << newDirectory << "_" << i;
      newDirectory2 = ss.str();
    }

    std::cout << newDirectory2 << std::endl;
    boost::filesystem::create_directory(newDirectory2);

    //If we want to output both raw tiff and quantized reflectance

    std::string newDirectory3(newDirectory2);
    std::string newDirectory3_1(newDirectory2);
    std::string newDirectory4(newDirectory2);
    std::string newDirectory4_1(newDirectory2);

    newDirectory3 += "/RAW";
    newDirectory3_1 += "/RAW";
    newDirectory4 += "/REFL";
    newDirectory4_1 += "/REFL";

    if(format == 3){

            for (int i = 2; boost::filesystem::exists(newDirectory3) && i < 10; ++i) {
              std::stringstream ss;
              std::stringstream ss2;
              ss << newDirectory3_1 << "_" << i;
              ss2 << newDirectory4_1 << "_" << i;
              newDirectory3 = ss.str();
              newDirectory4 = ss2.str();
            }
            boost::filesystem::create_directory(newDirectory3);
            boost::filesystem::create_directory(newDirectory4);
    }

    newDirectory3 += "/";
    newDirectory4 += "/";
    //exit(1);
    std::map<std::string, altumImage*> mappi;
    TagInfo *info2;

    int counteri = 0;

    //std::cout << dir_vec.size() << std::endl;
    //std::cout << directories.size() << std::endl;
    //exit(1);
    std::unordered_set<std::string> meta_found_panels;

    std::string comand = "-stay_open";

    for(auto dr2 : dir_vec)
        if (dr2) {



          while ((en = readdir(dr2)) != NULL) {

            std::string tempName(directories[counteri]);
            tempName += en->d_name;

            //std::cout << tempName << std::endl;
            std::stringstream test(en->d_name);
            std::string nam = en->d_name;
            std::string segment;
            std::vector<std::string> seglist;

            std::string id;
            int channel;
            int isPanel;

            while(std::getline(test, segment, '_'))
            {
               seglist.push_back(segment);
              // std::cout << " \n" << segment;
            }

            //std::cout << tempName << std::endl;
            if(seglist.size() <= 2)
                continue;

            //std::cout << tempName << std::endl;

            //et->ExtractInfo(tempName.c_str());
            info2 = et->ImageInfo(tempName.c_str(), comand.c_str());
            meta->readImageIdentifier(info2, tempName, channel, isPanel, id);

            if(isPanel == 1){

                meta_found_panels.insert(id);

            }

            if(!strcmp(id.c_str(), "what")){

                continue;

            }
            //std::cout << "cha1: " << channel << std::endl;
    //        std::cout << "seglist size: " << seglist.size() << std::endl;
            //id = std::stoi( seglist[1] );
            //channel = std::stoi( seglist[2] );
            //std::cout << "cha2: " << channel << std::endl;

            if(!mappi.count(id) && channel != 5){

                altumImage *altIma = new altumImage(id, pahi, referenceImage, distortion, cameraMatrix, et, meta, q);

                //altIma->fileNames[channel-1] = tempName;
                //std::cout << "channel: " << channel << std::endl;
                altIma->fileNames[channel] = (tempName);

                //altIma->fileNames_2[channel-1] = en->d_name;
                //std::cout << "metadata read! " << channel << " " << id << " " << altIma->fileNames.size() << std::endl;
                altIma->fileNames_2[channel] = (en->d_name);

                mappi[id] = altIma;
                altumImages.push_back(altIma);


            }else if(channel != 5){


                mappi[id]->fileNames[channel] = (tempName);
                //std::cout << mappi[id].fileNames.size() << std::endl;
                mappi[id]->fileNames_2[channel] = (en->d_name);

                mappi[id]->n_images++;
                //std::cout << mappi[id].fileNames_2.size() << std::endl;
            }
            //std::cout<<" \n"<<id; //print all directory name
            //FILE * pFile;
            //pFile = fopen (tempName.c_str() , "r");

          }
          counteri++;
          closedir(dr2); //close all directory
        }

    std::cout << "Images read!" << std::endl;

    newDirectory2 += "/";

    std::string referenceBand;

    if(referenceImage == 0)
        referenceBand = "Blue";
    if(referenceImage == 1)
        referenceBand = "Green";
    if(referenceImage == 2)
        referenceBand = "Red";
    if(referenceImage == 3)
        referenceBand = "NiR";
    if(referenceImage == 4)
        referenceBand = "Red Edge";
    if(referenceImage == 5)
        referenceBand = "Thermal";

    std::string moodi;
    if(mode == 1)
        moodi = "Panel";
    else if(mode == 2)
        moodi = "Panel+DLS";
    else if(mode == 3)
        moodi = "DLS";
    else
        moodi = "Adaptive";

    std::string formatti;


    if(format == 2)
        formatti = "Reflectance";
    else if(format == 3)
        formatti = "Reflectance and original";
    else
        formatti = "Original";

    std::cout << "\nJob description: \n";
    std::cout << "------------------\n";
    printf("\tImages:\t\t\t %d", altumImages.size());
    std::cout << "\n\tReference band:\t\t" << referenceBand << std::endl;
    std::cout << "\tReflectance mode:\t\t" << moodi << std::endl;
    std::cout << "\tOutput format:\t\t" << formatti << std::endl;

    log_file << "\tInput dir          : " << imgDirectory;
    log_file << "\n\tOutput dir         : " << newDirectory2;
    log_file << "\n\tNumber of images : " << altumImages.size();
    log_file << "\n\tReference band   : " << referenceBand;
    log_file << "\n\tReflectance mode : " << moodi;
    log_file << "\n\tOutput format    : " << formatti;


    fflush( stdout );
    progress proge;

    proge.setEnd(altumImages.size());

    calibrationPanels *cP = new calibrationPanels(panelReflectances, mode);
    bool panel = false;
    std::map<std::string, altumImage*>::iterator it;

    //for(altumImage *ai : altumImages){
    //for(int i = 0; i < altumImages.size(); i++){




    //ExifTool *et = new ExifTool("/media/koomikko/B8C80A93C80A4FD4/cplus/altumAlign/alltumAlign/bin/Debug/exifTool/exiftool");

    int counter = 1;

    double vmm, rss;
    process_mem_usage(vmm, rss);
    std::cout << "\nVM: " << vmm/1000.0 << "; RSS: " << rss/1000.0 << std::endl;


    std::vector<altumImage*> ai_vector;

    for ( it = mappi.begin(); it != mappi.end(); it++ ){
        ai_vector.push_back(it->second);
    }


    //First look for the calibration targets
    //for ( it = mappi.begin(); it != mappi.end(); it++ ){


    int n_cores2 = 1;

    if(!panel_Id_file.compare("nulli")){

        #pragma omp parallel for num_threads(n_cores2)
        for ( int i = 0; i < ai_vector.size(); i++){
            //altumImage *ai = it->second;
            //std::cout << "starting" << std::endl;
            //std::cout << "starting" << std::endl;
            //std::cout << "starting" << std::endl;
            altumImage *ai = ai_vector[i];
            //ai->printImage();

            /* No panels found with metadata,
                so we must find rectangles in every image.
                This is very prone to memory errors. */
            if(meta_found_panels.size() == 0){

                if(!meta_found_panels.count(ai->imageId)){
                    ai->chechIfCalibrationImage(cP, qrDecoder, 1, panel);
                }
                else{

                    ai->chechIfCalibrationImage(cP, qrDecoder, 3, panel);

                }

            }else{

                if(!meta_found_panels.count(ai->imageId)){
                    ai->chechIfCalibrationImage(cP, qrDecoder, 2, panel);
                }
                else{

                    ai->chechIfCalibrationImage(cP, qrDecoder, 3, panel);

                }

            }


            #pragma omp critical
            {
                proge.updateProgress();
            }

        }
    }
    else{
        // IF WE HAVE DEFINED PANEL IMAGE IDs
        std::unordered_set<std::string> panelids;

        readPanelImageIds(panelids, panel_Id_file);

        #pragma omp parallel for num_threads(n_cores2)
        for ( int i = 0; i < ai_vector.size(); i++){
            //altumImage *ai = it->second;
            //std::cout << "starting" << std::endl;
            //std::cout << "starting" << std::endl;
            //std::cout << "starting" << std::endl;
            altumImage *ai = ai_vector[i];
            //ai->printImage();

            if(panelids.count(ai->imageId)){

                ai->chechIfCalibrationImage(cP, qrDecoder, 3, panel);



            }else
                ai->chechIfCalibrationImage(cP, qrDecoder, 2, panel);
            #pragma omp critical
            {
                proge.updateProgress();
            }

        }
        /*
        for(auto i : panelids){

            mappi[i]->chechIfCalibrationImage(cP, qrDecoder, true);

        }
*/
    }

    int panels = cP->radiance.size();
    //log_file << "\n\tOutput format    : " << formatti;
    log_file << "\n\tPanels found     : " << panels;


    std::cout << cP->radiance.size() << " panels found!" << std::endl;

    if(cP->radiance.size() == 0){

        std::cout << "No panels found, changing modes" << std::endl;
        mode = 3;
        cP->mode = 3;

    }else
        cP->processPanels();

    if(panels > 0){

        log_file << "\n\tPanel reflectances:";
        log_file << "\n\t";

        for(int i = 0; i < panels; i++){
            for(int j = 0; j < cP->radiance[0].size(); j++){

                log_file << cP->meanReflectance[i][j] << "\t";

            }
            log_file << "\n\t";
        }

        log_file << "Panel reflectance sd (percentage point). >3.0 is suspicious:";
        log_file << "\n\t";

        for(int i = 0; i < panels; i++){
            for(int j = 0; j < cP->radiance[0].size(); j++){

                log_file << (round( cP->sdReflectance[i][j] * 100.0 * 1000.0 ) / 1000.0) << "\t";

            }
            log_file << "\n\t";
        }

        log_file << "Panel correction factors (should be roughly the same for all):";
        log_file << "\n\t";

        for(int i = 0; i < panels; i++){
            for(int j = 0; j < cP->radiance[0].size(); j++){

                log_file << (round( cP->radianceToReflectanceFactor[j][i] * 1000.0 ) / 1000.0) << "\t";
              //  std::cout << (round( cP->radianceToReflectanceFactor[j][i] * 1000.0 ) / 1000.0) << std::endl;
            }
            //std::cout << " ------------------ " << std::endl;
            log_file << "\n\t";
        }
        log_file << "Panel timestamps, should be ascending order:";
        log_file << "\n\t";

        for(int i = 0; i < panels; i++){

            log_file << (long long)cP->times[i];
            std::cout << "PANEL TIME: " << (long long)cP->times[i] << std::endl;
            //std::cout << " --------- --------- " << std::endl;
            log_file << "\n\t";
        }
    }

    //log_file.close();

    process_mem_usage(vmm, rss);
    std::cout << "\nVM: " << vmm/1000.0 << "; RSS: " << rss/1000.0 << std::endl;
    //exit(1);
    proge.reset();
    proge.setEnd(ai_vector.size());

    //omp_set_num_threads(3);

    int processed_images = 0;

    std::map<long, plotItem> plot_map;

    std::vector<long> times;
    std::vector<float> irradiances;
    std::vector<bool> isPanel;

    #pragma omp parallel for num_threads(n_cores)
    for ( int i = 0; i < ai_vector.size(); i++){

        //std::cout << " count: " << counter++ << std::endl;
        //altumImage *ai = it->second;
        altumImage *ai = ai_vector[i];

        if(ai->n_images != 5){

            continue;

        }

        if(ai->calibrationPanel){

            plotItem p_temp;
            p_temp.time = (long)ai->timeStamps[0];
            p_temp.ir1 = ai->irradiances[0];
            p_temp.ir2 = ai->irradiances[1];
            p_temp.ir3 = ai->irradiances[2];
            p_temp.ir4 = ai->irradiances[3];
            p_temp.ir5 = ai->irradiances[4];
            p_temp.panelOrNot = true;
            plot_map[(long)ai->timeStamps[0]] = p_temp;

            #pragma omp critical
            {
            proge.updateProgress();
            }
            ai->releaseMemory();
            delete ai;
            continue;

        }

        ai->attachCalibrationPanels(cP);
        //ai->linkProgressBar(&proge);
        ai->setOutputDirectory(newDirectory2);

        if(format == 3){

            ai->setOutputDirectories(newDirectory3, newDirectory4);

        }

        ai->readImages(undisti);

        bool good = ai->alignImages(format, compositeBandOrder, crop, undisti);


        //std::cout << "align done!" << std::endl;
        #pragma omp critical
            {
        proge.updateProgress();
        }
        ai->releaseMemory();


        if(good){
            #pragma omp critical
            {

                plotItem p_temp;
                p_temp.time = (long)ai->timeStamps[0];
                p_temp.ir1 = ai->irradiances[0];
                p_temp.ir2 = ai->irradiances[1];
                p_temp.ir3 = ai->irradiances[2];
                p_temp.ir4 = ai->irradiances[3];
                p_temp.ir5 = ai->irradiances[4];
                p_temp.panelOrNot = false;
                plot_map[(long)ai->timeStamps[0]] = p_temp;


                processed_images++;
            }
        }

        //std::cout << processed_images << " " << good  << std::endl;

        //std::cout << "release memory" << std::endl;
        delete ai;
        //altumImages[i] = 0;
        //std::cout << "deleted object" << std::endl;
    }

    log_file << "\n\tProcessed images : " << processed_images;

    auto endi = std::chrono::system_clock::now();

    //std::string curTime = date::format("%F %T", std::chrono::system_clock::now());

    std::time_t end_time = std::chrono::system_clock::to_time_t(endi);

    drawPlot(plot_map);

    log_file << "\n\tFinished " << std::ctime(&end_time);
    log_file << "\n######################################### ";
    log_file.close();

    delete et;
    metaItem m;
    //m.dst = "POISON";
    //m.src = "POISON";
    //q->push(m);
    job_done = true;
    th1.join();
    return 0;
}



