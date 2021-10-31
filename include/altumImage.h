#ifndef ALTUMIMAGE_H
#define ALTUMIMAGE_H
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <string>
#include <set>
#include <vector>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <map>
#include <ExifTool.h>
#include <exif.h>
#include <stdio.h>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <progress.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unordered_map>
#include <numeric>
#include <string>
#include <functional>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <calibrationPanels.h>

#include <iostream>
#include <sstream>
#include <locale>
#include <iomanip>
#include <queue.h>

//#include <tiffio.h>

#include <metadata_reader.h>
#define TIFFTAG_COMPRESSION     259 /* data compression technique */
#define     COMPRESSION_NONE        1   /* dump mode */

#define	TIFFTAG_TILEWIDTH		322	/* !tile width in pixels */
#define	TIFFTAG_TILELENGTH		323	/* !tile height in pixels */

#define TIFFTAG_TILEWIDTH_VALUE 256
#define TIFFTAG_TILEHEIGHT_VALUE 256
using namespace boost::accumulators;

class altumImage
{
    public:

        altumImage();
        altumImage(std::string id, std::string dir, int referenceImage, std::vector<cv::Mat> distortion, std::vector<cv::Mat> cameraMatrix, ExifTool *ett, metadata_reader *met,
                    blocking_queue<metaItem> *q_in);
        virtual ~altumImage();

        std::string imageId;
        int referenceImage;
        bool isCalibrationImage = false;

        ExifTool *et;
        blocking_queue<metaItem> *q;

        int n_images;

        metadata_reader *meta;

        std::string directory_of_executable;

        std::vector<cv::Mat> distortion;
        std::vector<cv::Mat> cameraMatrix;

        std::vector<std::vector<float>> radianceVector;

        std::vector<cv::Mat> channels;
        std::vector<cv::Mat> channels_quantizedReflectance;
        //std::vector<cv::Mat> channels_float;
        //std::vector<cv::Mat> channels_radiance;
        std::vector<cv::Mat> channels_8bit;
        //cv::Mat channels[5];
        //cv::Mat channels_float[5];
        //cv::Mat channels_8bit[5];
        //cv::Mat firstChannelLowResolution;

        //std::vector<ExifTool*> exifs;

        std::vector<double> irradianceForDLSCorrection;
        std::vector<double> black_levels;
        std::vector<std::vector<double>> vignetting_centers;
        std::vector<std::vector<double>> vignetting_polynomials;
        std::vector<std::vector<double>> radiometric_calibrations;
        std::vector<double> exposures;
        std::vector<double> gains;
        std::vector<double> irradiances;
        std::vector<int> timeStamps;

        std::vector<int> compositeOrder;

        bool calibrationPanel = false;

        bool calibrationPanelLoaded = false;
        bool outputBoth = false;

        std::vector<std::string> fileNames;
        std::vector<std::string> fileNames_2;
        progress *prog;
        cv::Mat descriptors[5];
        std::vector<cv::KeyPoint> keypoints[5];
        calibrationPanels *cP;

        cv::Mat merged_quantized_reflectance, merged_orig;
        std::vector<cv::Mat> mergeVector;
        std::vector<cv::Mat> mergeVector_quantized;

        bool outputComposite = false;

        bool outputCompositeRaw = false;
        bool outputCompositeReflectance = false;

        void attachCalibrationPanels(calibrationPanels *cP){

            calibrationPanelLoaded = true;
            this->cP = cP;

        }

        void outputCompositeImage(){



        }

        void releaseMemory(){

            for(int i = 0; i < 5; i++){

                //std::cout << i << " del 1 " << channels.size() << std::endl;
                channels[i].release();
                //std::cout << i << " del 2" << std::endl;
                channels_8bit[i].release();
                //std::cout << i << " del 3" << std::endl;
                //channels_float[i].release();
                //channels_radiance[i].release();
                channels_quantizedReflectance[i].release();
                descriptors[i].release();
                //std::cout << i << " del 4" << std::endl;
                distortion[i].release();
                cameraMatrix[i].release();
                //std::cout << i << " del 5" << std::endl;
                //keypoints[i].clear();
            }
/*
            vignetting_centers.clear();
            vignetting_polynomials.clear();
            fileNames.clear();
            fileNames_2.clear();
            black_levels.clear();
*/
            //firstChannelLowResolution.release();

        };


        void printImage(){

            //std::cout << "Image id: " << imageId << std::endl;
            for(int i = 0; i < 5; i++){

                std::cout << fileNames[i] << std::endl;

            }

        };

        void setOutputDirectory(std::string outputDirectory){

            this->outputDirectory = outputDirectory;

        }

        void setOutputDirectories(std::string outputDirectory1, std::string outputDirectory2){

            this->outputBoth = true;

            this->outputDirectory1 = outputDirectory1;
            this->outputDirectory2 = outputDirectory2;

        }

        void readIrradianceForDLSCorrection(calibrationPanels *cP){

            std::string horiz_irradiance = "HorizontalIrradiance";

            std::vector<double> irradia;
            meta->startReadingMetadata();
            std::string commandi = "-stay_open";
            for(int j = 0; j < 5; j++){

                std::cout << "NAM: " << fileNames[j] << std::endl;
                et->ExtractInfo(fileNames[j].c_str(), commandi.c_str());
                TagInfo *info = et->ImageInfo(fileNames[j].c_str(), commandi.c_str());

                if (info) {

                    for (TagInfo *i=info; i; i=i->next) {

                        std::string tempString(i->name);

                        if(!tempString.compare(horiz_irradiance)){

                            irradia.push_back(atof(i->value) * 10000 / 1000000);
                            //std::cout << "HAHAHAHAHAH " << irradia[irradia.size()-1] << std::endl;

                        }
                    }

                } else if (et->LastComplete() <= 0) {
                    std::cerr << "Error executing exiftool11!" << std::endl;
                }

                delete info;
            }

            cP->addIrradianceReading(irradia);
            meta->endReadingMetadata();
                            std::cout << "DONE " << std::endl;

        }

        static double angle( cv::Point pt1, cv::Point pt2, cv::Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

    static void reduceTheSize(std::vector<cv::Point> & square, double factor){

        double center_x = (square[0].x + square[1].x + square[2].x + square[3].x) / 4.0;
        double center_y = (square[0].y + square[1].y + square[2].y + square[3].y) / 4.0;

        for(int i = 0; i < square.size(); i++){

            double x_corr = (center_x - square[i].x) * (1.0 - factor);
            double y_corr = (center_y - square[i].y) * (1.0 - factor);

            //std::cout << "x_corr " << x_corr << std::endl;
            //std::cout << "y_corr " << y_corr << std::endl;

            square[i].x = square[i].x + x_corr;
            square[i].y = square[i].y + y_corr;

        }

    }

    static void findSquares( const cv::Mat& image, std::vector<std::vector<cv::Point> >& squares )
    {

    std::vector<double> centerX;
    std::vector<double> centerY;
    std::vector<double> dims;

    int thresh = 100, N = 11;
    squares.clear();
    cv::Mat pyr, timg, gray0(image.size(), CV_8U), gray;
    // down-scale and upscale the image to filter out the noise

    cv::pyrDown(image, pyr, cv::Size(image.cols/2, image.rows/2));
    cv::pyrUp(pyr, timg, image.size());
    std::vector<std::vector<cv::Point> > contours;
    // find squares in every color plane of the image
    //for( int c = 0; c < 3; c++ )
    //{
      //  int ch[] = {c, 0};
        //cv::mixChannels(&timg, 1, &gray0, 1, ch, 1);
        //cv::mixChan
        // try several threshold levels
        for( int l = 0; l < N; l++ )
        {
            // hack: use Canny instead of zero threshold level.
            // Canny helps to catch squares with gradient shading
            if( l == 0 )
            {
                // apply Canny. Take the upper threshold from slider
                // and set the lower to 0 (which forces edges merging)
                cv::Canny(timg, gray, 0, thresh, 5);
                // dilate canny output to remove potential
                // holes between edge segments
                cv::dilate(gray, gray, cv::Mat(), cv::Point(-1,-1));
            }
            else
            {
                // apply threshold if l!=0:
                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
                gray = timg >= (l+1)*255/N;
            }
            // find contours and store them all as a list
            cv::findContours(gray, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
            std::vector<cv::Point> approx;
            // test each contour
            for( size_t i = 0; i < contours.size(); i++ )
            {
                // approximate contour with accuracy proportional
                // to the contour perimeter
                cv::approxPolyDP(contours[i], approx, cv::arcLength(contours[i], true)*0.02, true);

                // square contours should have 4 vertices after approximation
                // relatively large area (to filter out noisy contours)
                // and be convex.
                // Note: absolute value of an area is used because
                // area may be positive or negative - in accordance with the
                // contour orientation
                if( approx.size() == 4 &&
                    fabs(cv::contourArea(approx)) > 10000 &&
                    cv::isContourConvex(approx) )
                {
                    double maxCosine = 0;
                    for( int j = 2; j < 5; j++ )
                    {

                        // find the maximum cosine of the angle between joint edges
                        double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                        maxCosine = MAX(maxCosine, cosine);
                    }
                    // if cosines of all angles are small
                    // (all angles are ~90 degree) then write quandrange
                    // vertices to resultant sequence

                    std::vector<double> sideLengths;


                    if( maxCosine < 0.3 ){

                        double center_x = (approx[0].x + approx[1].x + approx[2].x + approx[3].x) / 4.0;
                        double center_y = (approx[0].y + approx[1].y + approx[2].y + approx[3].y) / 4.0;

                        //center_x = (std::min(approx[0].x, approx[1].x, approx[2].x, approx[3].x) + std::max(approx[0].x, approx[1].x, approx[2].x, approx[3].x)) / 2.0;
                        //center_y = (std::min(approx[0].y, approx[1].y, approx[2].y, approx[3].y) + std::max(approx[0].y, approx[1].y, approx[2].y, approx[3].y)) / 2.0;

                        double distLastFirst = sqrt((approx[3].x - approx[0].x)*(approx[3].x - approx[0].x) + (approx[3].y - approx[0].y)*(approx[3].y - approx[0].y));
                        sideLengths.push_back(distLastFirst);
                        //std::cout << "dist: " << distLastFirst << std::endl;
                        for(int j = 1; j < approx.size(); j++){

                            double distance = sqrt((approx[j].x - approx[j-1].x)*(approx[j].x - approx[j-1].x) + (approx[j].y - approx[j-1].y)*(approx[j].y - approx[j-1].y));
                            //std::cout << "dist: " << distance << std::endl;
                            sideLengths.push_back(distance);

                        }


                        double average = accumulate( sideLengths.begin(), sideLengths.end(), 0.0) / (double)sideLengths.size();
                        //std::cout << "average: " << average << std::endl;

                        bool isOk = true;

                        for(int j = 0; j < sideLengths.size(); j++){

                            if(abs(sideLengths[j] - average) >= (0.05 * average)){

                                isOk = false;
                                break;
                            }

                        }
                        if(isOk){

                            for(int f = 0; f < centerX.size(); f++){

                                double disti = sqrt( (centerX[f] - center_x)*(centerX[f] - center_x) + (centerY[f] - center_y)*(centerY[f] - center_y) );

                                if(disti <= dims[f]){

                                    isOk = false;
                                    break;
                                }

                            }
                            if(isOk){
                                centerX.push_back(center_x);
                                centerY.push_back(center_y);
                                dims.push_back(average);
                                squares.push_back(approx);
                            }

                        }

                        //std::cout << " -------------------" << std::endl;
                    }

                }


            }
        }
        for(int i = 0; i < dims.size(); i++){

            //std::cout << centerX[i] << " " << centerY[i] << " " << dims[i] << std::endl;

        }
    //}
}

static void calcReflectancePanel(std::vector<std::vector<cv::Point> > &squares, cv::Mat imageCol, double &meani, double &sd,
                                    std::vector<std::vector<double>> &panel_radiances, std::vector<float> imgVector){

    int r = 20, g = 150, b = 30;
    int up = 20;

    std::vector<std::vector<double>> values;

    std::vector<double> means;
    std::vector<double> stds;

    double minstd = DBL_MAX;

    double meanHere, sdHere;

    int chosenVector = -1;
    int indexi;

    for(int i = 0; i < squares.size(); i++){

        values.push_back(std::vector<double>());

        int min_x = (int)std::min({squares[i][0].x, squares[i][1].x, squares[i][2].x, squares[i][3].x});
        int max_x = (int)std::max({squares[i][0].x, squares[i][1].x, squares[i][2].x, squares[i][3].x});

        int min_y = (int)std::min({squares[i][0].y, squares[i][1].y, squares[i][2].y, squares[i][3].y});
        int max_y = (int)std::max({squares[i][0].y, squares[i][1].y, squares[i][2].y, squares[i][3].y});

        int r_val, g_val, b_val;
        uint16_t pixValue2;
        float valu, valu2;

        for (int y = min_y; y < max_y; ++y){

            //uint8_t* pixel = imageCol.ptr<uint8_t>(y);  // point to first color in row
            //uint16_t* pixel2 = imageRaster.ptr<uint16_t>(y);  // point to first color in row
            cv::Vec3b* pixel = imageCol.ptr<cv::Vec3b>(y); // point to first pixel in row
            //cv::Vec3b* pixel22 = imageRaster.ptr<cv::Vec3b>(y); // point to first pixel in row

            for (int x = min_x; x < max_x; ++x){
                //pixValue = pixel[j][0];
                r_val = (int)pixel[x][0];
                g_val = (int)pixel[x][1];
                b_val = (int)pixel[x][2];

                if(r_val == r && g_val == g && b_val == b){

                    indexi = imageCol.cols * y + x;
                    valu = imgVector[indexi];
                    values[values.size()-1].push_back(valu);

                }

            }

        }

        boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::variance> > acc;
        for_each(values[values.size()-1].begin(), values[values.size()-1].end(), std::bind<void>(std::ref(acc), std::placeholders::_1));

        meanHere = mean(acc);
        sdHere = sqrt(variance(acc));

        if(sdHere < minstd){

            minstd = sdHere;
            meani = meanHere;
            sd = sdHere;
            chosenVector = i;

        }

        means.push_back(mean(acc));
        stds.push_back(sqrt(variance(acc)));

        r += up;
        g += up;
        b += up;
    }

    panel_radiances.push_back(std::vector<double>());

    for(auto i : values[chosenVector])
        panel_radiances[panel_radiances.size()-1].push_back(i);

}
// the function draws all the squares in the image
static void drawSquares( cv::Mat& image, const std::vector<std::vector<cv::Point> >& squares)
{

    int r = 20, g = 150, b = 30;
    int up = 20;

    for( size_t i = 0; i < squares.size(); i++ )
    {

        //if(cv::contourArea(squares[i]) < 5000)
          //  continue;

        const cv::Point* p = &squares[i][0];
        int n = (int)squares[i].size();
        //cv::polylines(image, &p, &n, 1, true, cv::Scalar(20,198,37), 3, cv::LINE_AA);

        cv::fillPoly(image, &p, &n, 1, cv::Scalar(r+up*i,g+up*i,b+up*i), 8);


    }

    //cv::imwrite("squares.jpg", image);
}

    float calcArea(cv::Mat area){

        float min_x = DBL_MAX, min_y = DBL_MAX, max_x = DBL_MIN, max_y = DBL_MIN;

        for(int i = 0; i < 4; i++){

            //std::cout << "x: " << area.at<float>(i,0) << std::endl;
            if(area.at<float>(i,0) > max_x)
                max_x = area.at<float>(i,0);
            if(area.at<float>(i,0) < min_x)
                min_x = area.at<float>(i,0);
            if(area.at<float>(i,1) > max_y)
                max_y = area.at<float>(i,1);
            if(area.at<float>(i,1) < min_y)
                min_y = area.at<float>(i,1);

        }

        std::cout << max_x << std::endl;
        std::cout << max_y << std::endl;
        std::cout << min_x << std::endl;
        std::cout << min_y << std::endl;
        return (max_x - min_x) * (max_y - min_y);

    }

    void chechIfCalibrationImage(calibrationPanels *cP, cv::QRCodeDetector qrDecoder, int force, bool &panelOrNot){


            if(force == 2){

                this->readIrradianceForDLSCorrection(cP);
                panelOrNot = false;
                return;

            }

            if(force == 1){
            cv::Mat bbox;//, rectifiedImage;

            cv::Mat tempImage;
            bool found = false;
            bool found_resize = false;
            cv::Mat testImage;

            for(int i = 0; i < fileNames.size(); i++){

                testImage = cv::imread(fileNames[i]);
                if(testImage.cols == 0)
                    continue;

                cv::resize(testImage, testImage, cv::Size(1600, 1200));
                bool data = qrDecoder.detect(testImage, bbox);

                if(data){

                    found = true;
                    break;


                }else{

                    testImage.release();

                }
            }

            if(found){

                std::cout << bbox << std::endl;
                float area = calcArea(bbox);

                float pers = area / (testImage.cols * testImage.rows);
                std::cout << pers << " debug111" << std::endl;
                //std::string data2 = qrDecoder.detectAndDecode(testImage, bbox);
                if(pers > 1.00){

                    this->readIrradianceForDLSCorrection(cP);
                    //rectifiedImage.release();
                    testImage.release();
                    tempImage.release();
                    panelOrNot = false;
                    return;

                }
                std::string data2;
                try{

                    std::cout << "heil1 " << std::endl;

                    try{
                    data2 = qrDecoder.decode(testImage, bbox);
                    //} catch(std::bad_alloc& ex) {
                    } catch( ... ) {

                        std::cout << "FUCKIN OPENCV qrDecoder!!!" << std::endl;
                        this->readIrradianceForDLSCorrection(cP);
                    //rectifiedImage.release();
                    testImage.release();
                    tempImage.release();
                    panelOrNot = false;
                    data2.clear();
                    return;
                    }
                    std::cout << "heil2" << std::endl;
                }catch(...){

                    std::cout << "exception occured!!" << std::endl;
                    this->readIrradianceForDLSCorrection(cP);
                    //rectifiedImage.release();
                    testImage.release();
                    tempImage.release();
                    panelOrNot = false;
                    return;

                }
                std::cout << data2.length() << std::endl;
              if(data2.length()>0){

                std::cout << "WAIT WHAT!! " << std::endl;

              }

              else{

                this->readIrradianceForDLSCorrection(cP);
                //rectifiedImage.release();
                testImage.release();
                tempImage.release();
                panelOrNot = false;
                return;
              }

              }
              else{
                //std::cout << "QR Code not detected" << std::endl;

                this->readIrradianceForDLSCorrection(cP);
                //rectifiedImage.release();
                testImage.release();
                tempImage.release();
                panelOrNot = false;
                return;
              }

            this->printImage();

            testImage.release();
            tempImage.release();

            }

            this->calibrationPanel = true;
            this->readImages(true);
            double mean2, sd;

            std::vector<double> meanRadiances;

            std::vector<std::vector<double>> panel_radiances;



            for(int i = 0; i < channels_8bit.size(); i++){

                cv::Mat temppi = channels_8bit[i].clone();
                cv::cvtColor(temppi,temppi, cv::COLOR_GRAY2RGB);
                std::vector<std::vector<cv::Point> > squares;

                findSquares(channels_8bit[i], squares);

                for(int i = 0; i < squares.size(); i++){

                    reduceTheSize(squares[i], 0.75);

                }
                drawSquares(temppi, squares);
                //imwrite("squares.jpg", temppi);

                if(squares.size() == 0){

                    panelOrNot = false;
                    return;
                }


                calcReflectancePanel(squares, temppi, mean2, sd, panel_radiances, radianceVector[i]);
                meanRadiances.push_back(mean2);
                //exit(1);
            }

            if(false)
            for(int i = 0; i < meanRadiances.size(); i++){

                std::cout << "channel " << i+1 << " " << meanRadiances[i] << " W/m2" << std::endl;
                std::cout << cP->reflectance[i] << std::endl;
                std::cout << "channel " << i+1 << " " << (cP->reflectance[i]/meanRadiances[i]) << " factor" << std::endl;

            }

            double meanHere, sdHere;

            std::vector<double> mean_r, sd_r;

            for(int i = 0; i < panel_radiances.size(); i++){

                for(int j = 0; j < panel_radiances[i].size(); j++){

                    panel_radiances[i][j] *= (cP->reflectance[i]/meanRadiances[i]);

                }

                boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::variance> > acc;
                for_each(panel_radiances[i].begin(), panel_radiances[i].end(), std::bind<void>(std::ref(acc), std::placeholders::_1));

                meanHere = mean(acc);
                sdHere = sqrt(variance(acc));

                mean_r.push_back(meanHere);
                sd_r.push_back(sdHere);

                std::cout << "channel " << i+1 << " " << meanRadiances[i] << " W/m2" << std::endl;
                std::cout << "channel " << i+1 << " " << meanHere << " reflectance" << std::endl;
                std::cout << "channel " << i+1 << " " << sdHere << " sd reflectance" << std::endl;

            }

            cP->addPanel(meanRadiances, this->timeStamps[0], mean_r, sd_r);

            this->releaseMemory();
            panelOrNot = true;
        };

        void removeOutliers(std::vector< std::vector< cv::DMatch > > &matches, std::vector<cv::KeyPoint> keypoints_ref, std::vector<cv::KeyPoint> keypoints_2,
                            int refId, int secondId, int format, int crop);
        void findPoints();
        bool alignImages(int format, std::vector<int> compositeOrder, int crop, bool undisti);
        void readImages(bool undisti);

        void radiometricCalibration(cv::Mat &image,
                                    //cv::Mat &image_float,
                                    //cv::Mat &image_radiance,
                                    cv::Mat &image_quantizedReflectance,
                                    double black_levels,
                                    std::vector<double> vignetting_center,
                                    std::vector<double> vignetting_polynomial,
                                    std::vector<double> radiometric_calibration,
                                    double exposure,
                                    double gain,
                                    double irradiance,
                                    int band,
                                    std::vector<float> &radiance_vector
                                    );



        std::string outputDirectory;
        std::string outputDirectory1;
        std::string outputDirectory2;

    protected:

    private:
};

#endif // ALTUMIMAGE_H
