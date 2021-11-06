#include "altumImage.h"
#include <omp.h>
#define DEBUG2

#undef DEBUG2



altumImage::altumImage()
{
    //ctor
}

altumImage::altumImage(std::string id, std::string dir, int referenceImage,
                        std::vector<cv::Mat> distortion, std::vector<cv::Mat> cameraMatrix, ExifTool *ett,
                        metadata_reader *met, blocking_queue<metaItem> *q_in)
{
    this->imageId = id;
    this->directory_of_executable = dir;
    this->referenceImage = referenceImage;
    this->distortion = distortion;
    this->cameraMatrix = cameraMatrix;
    this->et = ett;
    this->meta = met;
    this->q = q_in;
    this->n_images = 1;

    fileNames.reserve(5);
    fileNames_2.reserve(5);
    for(int i = 0; i < 5; i++){

        fileNames.push_back("haha");
        fileNames_2.push_back("haha");
        mergeVector.push_back(cv::Mat());
        mergeVector_quantized.push_back(cv::Mat());
    }



/*
    for(int i = 0; i < 5; i++){

        channels.push_back(cv::Mat());
        channels_float.push_back(cv::Mat());
        channels_8bit.push_back(cv::Mat());

    }
    */
}


altumImage::~altumImage()
{
    //dtor
}

void altumImage::findPoints(){
/*
    std::vector<cv::Ptr<cv::xfeatures2d::SURF>> det_vec;
    std::vector<cv::Ptr<cv::DescriptorMatcher>> mat_vec;

    for(int i = 0; i < 5; i++){

        det_vec.push_back(cv::xfeatures2d::SURF::create());
        mat_vec.push_back(cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED));

    }
    */
    cv::Ptr<cv::xfeatures2d::SURF> detector = cv::xfeatures2d::SURF::create();
    //cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);

    //#pragma omp parallel for
    for(int i = 0; i < 5; i++){
        //std::cout << "starting " << i << std::endl;
        //cv::Mat convertedImage = channels[i].clone();
        //channels[i].convertTo( convertedImage, 0);
        //cv::imwrite("TESTI.jpg", convertedImage);
        //exit(1);
        detector->detectAndCompute( channels_8bit[i], cv::noArray(), keypoints[i], descriptors[i] );
        //det_vec[i]->detectAndCompute( channels_8bit[i], cv::noArray(), keypoints[i], descriptors[i] );

    }
}

void replaceExt(std::string& s, const std::string& newExt) {

   std::string::size_type i = s.rfind('.', s.length());

   if (i != std::string::npos) {
      s.replace(i, newExt.length(), newExt);
   }
}

bool altumImage::alignImages(int format, std::vector<int> compositeOrder, int crop, bool undisti){

    std::vector<int> tags = {TIFFTAG_COMPRESSION, COMPRESSION_NONE};
    //std::vector<int> tags = {TIFFTAG_COMPRESSION, COMPRESSION_NONE, TIFFTAG_TILELENGTH, TIFFTAG_TILEHEIGHT_VALUE, TIFFTAG_TILEWIDTH, TIFFTAG_TILEWIDTH_VALUE};


    double scaleFactor = (double)crop / 100.0d;

    this->compositeOrder = compositeOrder;

    //std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();

    this->findPoints();
    //std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
    //std::cout << "\nFind points: = " << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - begin1).count() << "[ms]" << std::endl;

    int refe = referenceImage;

    std::vector<cv::Ptr<cv::DescriptorMatcher>> mat_vec;
    for(int i = 0; i < 5; i++){

        mat_vec.push_back(cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED));

    }
    cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);

    bool abort = false;

    //#pragma omp parallel for
    for(int i = 0; i < 5; i++){

        if(!abort){
            if(i == refe)
                continue;

            std::vector< std::vector< cv::DMatch > > matches;
            mat_vec[i]->knnMatch( descriptors[refe], descriptors[i], matches, 2 );

            if(matches.size() <= 500){

                abort = true;
                //break;
                continue;
            }


            //std::cout << " hot here111: " << i << " " << matches.size() << std::endl;
            //std::cout << " hot here111: " << fileNames[refe] << " " << fileNames[i] << std::endl;
            //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            removeOutliers(matches, keypoints[refe], keypoints[i], refe, i, format, crop);
            //std::cout << " hot here222: " << i << std::endl;
            //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            //std::cout << "\nRemove outliers: = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

        }
    }

    if(abort){

        return false;

    }


    if(format != 3){

        if(format == 1){
            std::string out(this->outputDirectory);
            out += fileNames_2[refe];
            cv::Rect area(channels[refe].cols * scaleFactor, channels[refe].rows * scaleFactor, channels[refe].cols - (channels[refe].cols * scaleFactor)*2, channels[refe].rows - (channels[refe].rows * scaleFactor)*2);




                    //}
            cv::Mat output_cropped;

            if(scaleFactor > 0.0 && scaleFactor < 1.0)
                output_cropped = channels[refe](area);
            else
                output_cropped = channels[refe];

/*
             long long summa = 0;
                    uint16_t* uc_pixel = (uint16_t*)output_cropped.data;
                    int indexi;
                    long long pixelSum = 0;

                    for (int y = 0; y < output_cropped.rows; y++)
                    {
                        for (int x = 0; x < output_cropped.cols; x++)
                        {

                            indexi = output_cropped.cols * y + x ;
                            pixelSum += uc_pixel[indexi];
                        }
                    }

                    //if(secondId == 1){
                        printImage();
                        //std::cout << homo << std::endl;
                        std::cout << pixelSum << std::endl;
*/
            meta->startWritingImage();
            cv::imwrite(out, output_cropped, tags);
            meta->endWritingImage();
            mergeVector[refe] = output_cropped.clone();

            /*
            std::string command(directory_of_executable);
                //std::cout << command << std::endl;
                //exit(1);
                //command += "exifTool/exift"
            command += "/exifTool/exiftool -tagsfromfile ";
            command += fileNames[refe]; // This is the original file
            //command += " -xmp ";
            command += " -all:all ";
            command += " -xmp ";
            command += this->outputDirectory;
            command += fileNames_2[refe]; //this is the output file
            command += " -overwrite_original > /dev/null 2>&1";
            //system(command.c_str());

            std::string commandi = "";
*/
            std::string outputFile = "";
            outputFile += this->outputDirectory;
            outputFile += fileNames_2[refe];

            metaItem tempItem;
            tempItem.src = fileNames[refe];
            tempItem.dst = outputFile;
            q->push(tempItem);

/*
            meta->startReadingMetadata();

            et->ExtractInfo(fileNames[refe].c_str());
            TagInfo *info = et->ImageInfo(fileNames[refe].c_str());
            //int cmdNum3 = et->Complete(5);

            commandi += "-tagsfromfile\n";
            commandi += fileNames[refe];
            commandi += "\n-all:all";
            commandi += "\n-xmp";
            commandi += "\n-overwrite_original";
            //std::cout << commandi.c_str() << std::endl;
            //std::cout << outputFile.c_str() << std::endl;
            int cmdNum = et->WriteInfo(outputFile.c_str(), commandi.c_str());
            int cmdNum2 = et->Complete(2);
            meta->endReadingMetadata();
            */
        }
        if(format == 2){

            std::string out(this->outputDirectory);
            out += fileNames_2[refe];

            cv::Rect area(channels_quantizedReflectance[refe].cols * scaleFactor,
            channels_quantizedReflectance[refe].rows * scaleFactor,
            channels_quantizedReflectance[refe].cols - (channels_quantizedReflectance[refe].cols * scaleFactor)*2,
            channels_quantizedReflectance[refe].rows - (channels_quantizedReflectance[refe].rows * scaleFactor)*2);

            cv::Mat output_cropped;

            if(scaleFactor > 0.0 && scaleFactor < 1.0)
                output_cropped = channels_quantizedReflectance[refe](area);
            else
                output_cropped = channels_quantizedReflectance[refe];

            meta->startWritingImage();
            cv::imwrite(out, output_cropped, tags);
            meta->endWritingImage();
            //cv::imwrite(out, channels_quantizedReflectance[refe]);

            mergeVector_quantized[refe] = (output_cropped.clone());
            /*
            std::string command(directory_of_executable);
                //std::cout << command << std::endl;
                //exit(1);
                //command += "exifTool/exift"
            command += "/exifTool/exiftool -tagsfromfile ";
            command += fileNames[refe]; // This is the original file
            //command += " -xmp ";
            command += " -all:all ";
            command += " -xmp ";
            command += this->outputDirectory;
            command += fileNames_2[refe]; //this is the output file
            command += " -overwrite_original > /dev/null 2>&1";
            //system(command.c_str());

            std::string commandi = "";
*/
            std::string outputFile = "";
            outputFile += this->outputDirectory;
            outputFile += fileNames_2[refe];

            metaItem tempItem;
            tempItem.src = fileNames[refe];
            tempItem.dst = outputFile;
            q->push(tempItem);

            std::cout << "DEBUGGG!!! " << tempItem.src  << " " << tempItem.dst << std::endl;
            //exit(1);

/*
            meta->startReadingMetadata();

            et->ExtractInfo(fileNames[refe].c_str());
            TagInfo *info = et->ImageInfo(fileNames[refe].c_str());
            //int cmdNum3 = et->Complete(5);

            commandi += "-tagsfromfile\n";
            commandi += fileNames[refe];
            commandi += "\n-all:all";
            commandi += "\n-xmp";
            commandi += "\n-overwrite_original";
            //std::cout << commandi.c_str() << std::endl;
            //std::cout << outputFile.c_str() << std::endl;
            int cmdNum = et->WriteInfo(outputFile.c_str(), commandi.c_str());
            int cmdNum2 = et->Complete(2);
            meta->endReadingMetadata();
*/
        }

    }else{

        std::string out(this->outputDirectory1);
        out += fileNames_2[refe];

        meta->startWritingImage();

        cv::imwrite(out, channels[refe], tags);

        meta->endWritingImage();
/*
        std::string command(directory_of_executable);
                //std::cout << command << std::endl;
                //exit(1);
                //command += "exifTool/exift"
        command += "/exifTool/exiftool -tagsfromfile ";
        command += fileNames[refe]; // This is the original file
        command += " -all:all ";
        command += " -xmp ";
        command += this->outputDirectory1;
        command += fileNames_2[refe]; //this is the output file
        command += " -overwrite_original > /dev/null 2>&1";
        //system(command.c_str());
        std::string commandi = "";
*/
            std::string outputFile = "";
            outputFile += this->outputDirectory1;
            outputFile += fileNames_2[refe];

            metaItem tempItem;
            tempItem.src = fileNames[refe];
            tempItem.dst = outputFile;
            q->push(tempItem);

/*
            meta->startReadingMetadata();

            et->ExtractInfo(fileNames[refe].c_str());
            TagInfo *info = et->ImageInfo(fileNames[refe].c_str());
            //int cmdNum3 = et->Complete(5);

            commandi += "-tagsfromfile\n";
            commandi += fileNames[refe];
            commandi += "\n-all:all";
            commandi += "\n-xmp";
            commandi += "\n-overwrite_original";
            //std::cout << commandi.c_str() << std::endl;
            //std::cout << outputFile.c_str() << std::endl;
            int cmdNum = et->WriteInfo(outputFile.c_str(), commandi.c_str());
            int cmdNum2 = et->Complete(2);
            */
            //meta->endReadingMetadata();

/*
        std::string commandi = "";

        commandi += "-tagsfromfile\"";
        commandi += fileNames[refe];
        commandi += "\"n-all:all\n-xmp";
        int cmdNum = et->WriteInfo(fileNames_2[refe], commandi);

*/
        std::string out2(this->outputDirectory2);
        out2 += fileNames_2[refe];

        meta->startWritingImage();

        cv::imwrite(out2, channels_quantizedReflectance[refe], tags);
        meta->endWritingImage();
/*
        std::string command2(directory_of_executable);
                //std::cout << command << std::endl;
                //exit(1);
                //command += "exifTool/exift"
        command2 += "/exifTool/exiftool -tagsfromfile ";
        command2 += fileNames[refe]; // This is the original file
        command2 += " -all:all ";
        command2 += " -xmp ";
        command2 += this->outputDirectory2;
        command2 += fileNames_2[refe]; //this is the output file
        command2 += " -overwrite_original > /dev/null 2>&1";
        //system(command2.c_str());

        commandi = "";
*/
            outputFile = "";
            outputFile += this->outputDirectory2;
            outputFile += fileNames_2[refe];

            tempItem.src = fileNames[refe];
            tempItem.dst = outputFile;
            q->push(tempItem);

/*
            meta->startReadingMetadata();

            et->ExtractInfo(fileNames[refe].c_str());
            info = et->ImageInfo(fileNames[refe].c_str());
            //int cmdNum3 = et->Complete(5);

            commandi += "-tagsfromfile\n";
            commandi += fileNames[refe];
            commandi += "\n-all:all";
            commandi += "\n-xmp";
            commandi += "\n-overwrite_original";
            //std::cout << commandi.c_str() << std::endl;
            //std::cout << outputFile.c_str() << std::endl;
            cmdNum = et->WriteInfo(outputFile.c_str(), commandi.c_str());
            cmdNum2 = et->Complete(2);
            meta->endReadingMetadata();
*/
    }

    //If we want to output composite image or not
    if(compositeOrder.size() > 1){

       // std::cout << "HERE " << std::endl;
        //ONLY RAW
        if(format == 3){

            std::vector<cv::Mat> mergeVector2;

            for(int i = 0; i < compositeOrder.size(); i++){

                mergeVector2.push_back(mergeVector[compositeOrder[i]]);

            }

            cv::merge(mergeVector2, merged_orig);

            std::string out(this->outputDirectory);

            std::string newName(fileNames_2[refe]);

            replaceExt(newName, "_COMP.tif");
            out += newName;

            meta->startWritingImage();

            cv::imwrite(out, merged_orig, tags);

            meta->endWritingImage();
            /*
            mergeVector2.push_back(mergeVector[4]);
            mergeVector2.push_back(mergeVector[2]);
            mergeVector2.push_back(mergeVector[1]);

            cv::merge(mergeVector2, merged_orig);

            cv::imwrite("testi.tif", merged_orig);
            exit(1);
            */
        }



        //if(mergeVector_quantized.size() > 0 && mergeVector.size() == 0){
        if(format == 2){

            std::vector<cv::Mat> mergeVector3;

            for(int i = 0; i < compositeOrder.size(); i++){

                mergeVector3.push_back(mergeVector_quantized[compositeOrder[i]]);

            }

            cv::merge(mergeVector3, merged_quantized_reflectance);

            std::string out(this->outputDirectory);

            std::string newName(fileNames_2[refe]);

            replaceExt(newName, "_MULTI.tif");
            out += newName;

            std::string command = "";

            command += "gdal_merge.py -o ";
            command += out;
            command += " -separate";

            for(int i = 0; i < compositeOrder.size(); i++){

                command += " ";
                command += (this->outputDirectory + fileNames_2[compositeOrder[i]]);


            }

            meta->startWritingImage();

            system(command.c_str());

            meta->endWritingImage();
            std::cout << command << std::endl;
            std::cout << command << std::endl;
            std::cout << command << std::endl;
            std::cout << command << std::endl;

            //cv::imwrite(out, merged_quantized_reflectance, tags);

            metaItem tempItem;
            tempItem.src = fileNames[refe];
            tempItem.dst = out;
            q->push(tempItem);



        }

        if(format == 3){

            std::vector<cv::Mat> mergeVector3;

            for(int i = 0; i < compositeOrder.size(); i++){

                mergeVector3.push_back(mergeVector_quantized[compositeOrder[i]]);

            }

            cv::merge(mergeVector3, merged_quantized_reflectance);

            std::string out(this->outputDirectory2);

            std::string newName(fileNames_2[refe]);

            replaceExt(newName, "_COMP.tif");
            out += newName;

            meta->startWritingImage();

            cv::imwrite(out, merged_quantized_reflectance, tags);

            meta->endWritingImage();


             std::vector<cv::Mat> mergeVector2;

            for(int i = 0; i < compositeOrder.size(); i++){

                mergeVector2.push_back(mergeVector[compositeOrder[i]]);

            }

            cv::merge(mergeVector2, merged_orig);

            std::string out2(this->outputDirectory1);

            std::string newName2(fileNames_2[refe]);

            replaceExt(newName2, "_COMP.tif");
            out2 += newName2;


            meta->startWritingImage();

            cv::imwrite(out2, merged_orig, tags);

            meta->endWritingImage();
        }
    }

    return true;
}

void altumImage::removeOutliers(std::vector< std::vector< cv::DMatch > > &matches, std::vector<cv::KeyPoint> keypoints_ref, std::vector<cv::KeyPoint> keypoints_2,
                                int refId, int secondId, int format, int crop){

    //std::vector<int> tags = {TIFFTAG_COMPRESSION, COMPRESSION_NONE, TIFFTAG_TILELENGTH, TIFFTAG_TILEHEIGHT_VALUE, TIFFTAG_TILEWIDTH, TIFFTAG_TILEWIDTH_VALUE};
    std::vector<int> tags = {TIFFTAG_COMPRESSION, COMPRESSION_NONE};


    double scaleFactor = (double)crop / 100.0d;
    float nndrRatio = 0.75f;
    std::vector< cv::DMatch > good_matches;

    std::vector<double> matchDistances;
    std::vector<double> matchRatios;
    double maxDistance = DBL_MIN;
    double minDistance = DBL_MAX;

    for (size_t j = 0; j < matches.size(); ++j){

                cv::Point2f from = keypoints_ref[matches[j][0].queryIdx].pt;
                cv::Point2f to = keypoints_2[matches[j][0].trainIdx].pt;

                cv::Point2f from2 = keypoints_ref[matches[j][1].queryIdx].pt;
                cv::Point2f to2 = keypoints_2[matches[j][1].trainIdx].pt;

                cv::DMatch m1 = matches[j][0];
                cv::DMatch m2 = matches[j][1];
                if (m1.distance < nndrRatio * m2.distance){ //dist < tresholdDist && abs(from.y-to.y) < 50000  && ; !tarkastus.count(location1) && !tarkastus2.count(location2) &&

                    good_matches.push_back(m1);

                    matchDistances.push_back(m1.distance);
                    matchRatios.push_back(m2.distance - m1.distance);
                }

        }

        std::vector< cv::Point2f >  myPoints1;
        std::vector< cv::Point2f >  myPoints2;

        //std::vector< cv::Point2f > inlierPointsForHomo1;
        //std::vector< cv::Point2f > inlierPointsForHomo2;

        std::vector< cv::KeyPoint >  myPoints1k;
        std::vector< cv::KeyPoint >  myPoints2k;

        //std::vector< cv::KeyPoint >  myPoints1k_inlier;
        //std::vector< cv::KeyPoint >  myPoints2k_inlier;

        std::vector<int> index1;
        std::vector<int> index2;

        bool isLeftRightCorrect = true;

        double meanx1 = 0.0d;
        double meanx2 = 0.0d;

        //std::vector<cv::DMatch> matches_to_draw;
        //std::vector<cv::DMatch> matches_to_draw_inlier;

        for( unsigned int j = 0; j < good_matches.size(); j++ ){
            //-- Get the keypoints from the good matches

            cv::Point2f temp1( keypoints_ref[ good_matches[j].queryIdx ].pt.x, keypoints_ref[ good_matches[j].queryIdx ].pt.y);

            cv::Point2f temp2( keypoints_2[ good_matches[j].trainIdx ].pt.x, keypoints_2[ good_matches[j].trainIdx ].pt.y);

            meanx1 += temp1.x;
            meanx2 += temp2.x;

            myPoints1.push_back( temp1 );
            myPoints2.push_back( temp2 );

            myPoints1k.push_back( cv::KeyPoint(temp1, 1.0f) );
            myPoints2k.push_back( cv::KeyPoint(temp2, 1.0f) );

            //matches_to_draw.push_back(cv::DMatch(j,j,1));

        }

        cv::Mat mask;
        cv::Mat F = cv::findFundamentalMat(cv::Mat(myPoints1), cv::Mat(myPoints2), cv::RANSAC, 1.0, 0.99, mask);

        //cv::Mat essentialMatrix = cv::findEssentialMat((myPoints1), (myPoints2), fx, pp, cv::RANSAC, 0.999, 1.0, mask);
        //cv::Mat essentialMatrix;// = cv::findEssentialMat((myPoints1), (myPoints2), imagePairs[i]->Getimg1()->GetKd(), cv::RANSAC, 0.9, 0.1, mask);

        //cv::findEssentialMat()
        std::vector<cv::Point2d> inlier_match_points1, inlier_match_points2;
        //std::vector<cv::Point2f> points_for_disparity_range1, points_for_disparity_range2;
        //std::vector<cv::Point2f> points_quadrants1, points_quadrants2;
        //std::vector<cv::Point2d> points_solvepnp1, points_solvepnp2;
        //std::vector<cv::Point2f> points_quadrants1_undist, points_quadrants2_undist;

        float img1_match_maxX = FLT_MIN;
        float img1_match_minX = FLT_MAX;
        float img1_match_maxY = FLT_MIN;
        float img1_match_minY = FLT_MAX;

        //std::vector<double> inlier_match_distances;
        //std::vector<double> inlier_match_ratios;

        maxDistance = DBL_MIN;
        minDistance = DBL_MAX;
        //std::vector<int> img1_ids, img2_ids;
        //std::vector<int> img1_ids_inlier, img2_ids_inlier;

        //std::map<double, int> aaha;

        for(int i = 0; i < mask.rows; i++) {
/*
            if(mask.at<unsigned char>(i)){
            //if(true){
                points_for_disparity_range1.push_back(myPoints1[i]);
                points_for_disparity_range2.push_back(myPoints2[i]);

            }
            */
            if(mask.at<unsigned char>(i)){

                //myPoints1k_inlier.push_back(myPoints1k[i]);
                //myPoints2k_inlier.push_back(myPoints2k[i]);

                inlier_match_points1.push_back(myPoints1[i]);
                inlier_match_points2.push_back(myPoints2[i]);

                //inlierPointsForHomo1.push_back(myPoints1[i]);
                //inlierPointsForHomo2.push_back(myPoints2[i]);

                //inlier_match_distances.push_back(matchDistances[i]);
                //inlier_match_ratios.push_back(matchRatios[i]);

                //matches_to_draw_inlier.push_back(matches_to_draw[i]);
/*
                if(matchDistances[i] > maxDistance)
                    maxDistance = matchDistances[i];

                if(matchDistances[i] < minDistance)
                    minDistance = matchDistances[i];
*/
                //aaha[i] = (matchDistances[i]);

            }
        }

        //cv::Mat matchImage;

        cv::Mat homo = cv::findHomography(inlier_match_points2, inlier_match_points1, cv::RANSAC, 3, mask, 5000, 0.999);
        //cv::Mat homo = cv::findHomography(inlier_match_points2, inlier_match_points1, cv::LMEDS, 3, mask, 5000, 0.999);
        cv::Mat outputti;


        if(format == 1 || format == 2){
            if(format == 2){

                cv::warpPerspective(channels_quantizedReflectance[secondId], outputti,  homo, cv::Size(channels_quantizedReflectance[secondId].cols, channels_quantizedReflectance[secondId].rows));
                //mergeVector_quantized.push_back(outputti.clone());

            }else{


                cv::warpPerspective(channels[secondId], outputti,  homo, cv::Size(channels[secondId].cols, channels[secondId].rows));
                //mergeVector.push_back(outputti.clone());


            }


            double pp_x = cameraMatrix[secondId].at<double>(0,2);
            double pp_y = cameraMatrix[secondId].at<double>(1,2);


            cv::Rect area(outputti.cols * scaleFactor, outputti.rows * scaleFactor, outputti.cols - (outputti.cols * scaleFactor)*2, outputti.rows - (outputti.rows * scaleFactor)*2);

            cv::Mat output_cropped;

            if(scaleFactor > 0.0 && scaleFactor < 1.0)
                output_cropped = outputti(area);
            else
                output_cropped = outputti;


            if(format == 2)
                mergeVector_quantized[secondId] = (output_cropped.clone());
            else
                mergeVector[secondId] = (output_cropped.clone());

            std::string out(this->outputDirectory);
            out += fileNames_2[secondId];

            meta->startWritingImage();

            cv::imwrite(out, output_cropped,tags);

            meta->endWritingImage();


            outputti.release();
/*
            std::string command(directory_of_executable);
            //std::cout << command << std::endl;
            //exit(1);
            //command += "exifTool/exift"
            command += "/exifTool/exiftool -tagsfromfile ";
            command += fileNames[secondId]; // This is the original file
            //command += " -xmp ";
            //command += " -GPS:All ";
            //command += " \"-xmp<all\" ";
            command += " -all:all ";
            command += " -xmp ";
            //command += " -all:all>all:all ";
            command += this->outputDirectory;
            command += fileNames_2[secondId]; //this is the output file
            command += " -overwrite_original > /dev/null 2>&1";
            //system(command.c_str());
            //std::cout << command << std::endl;

            std::string commandi = "";
*/
            std::string outputFile = "";
            outputFile += this->outputDirectory;
            outputFile += fileNames_2[secondId];

            metaItem tempItem;
            tempItem.src = fileNames[secondId];
            tempItem.dst = outputFile;
            q->push(tempItem);
/*
            meta->startReadingMetadata();
            et->ExtractInfo(fileNames[secondId].c_str());
            TagInfo *info = et->ImageInfo(fileNames[secondId].c_str());
            //int cmdNum3 = et->Complete(5);

            commandi += "-tagsfromfile\n";
            commandi += fileNames[secondId];
            commandi += "\n-all:all";
            commandi += "\n-xmp";
            commandi += "\n-overwrite_original";
            //std::cout << commandi.c_str() << std::endl;
            //std::cout << outputFile.c_str() << std::endl;
            int cmdNum = et->WriteInfo(outputFile.c_str(), commandi.c_str());
            int cmdNum2 = et->Complete(2);
            //std::cout << cmdNum << " " << cmdNum2 << " " << cmdNum3 << std::endl;

            meta->endReadingMetadata();
*/
        }

        if(format == 3){

            cv::warpPerspective(channels[secondId], outputti,  homo, cv::Size(channels[secondId].cols, channels[secondId].rows));
            std::string out(this->outputDirectory1);
            out += fileNames_2[secondId];

            meta->startWritingImage();

            cv::imwrite(out, outputti,tags);

            meta->endWritingImage();
            mergeVector[secondId] = (outputti.clone());
            //outputti.release();
/*
            std::string command(directory_of_executable);
            //std::cout << command << std::endl;
            //exit(1);
            //command += "exifTool/exift"
            command += "/exifTool/exiftool -tagsfromfile ";
            command += fileNames[secondId]; // This is the original file
            command += " -all:all ";
            command += " -xmp ";
            command += this->outputDirectory1;
            command += fileNames_2[secondId]; //this is the output file
            command += " -overwrite_original > /dev/null 2>&1";
            //system(command.c_str());

            std::string commandi = "";
*/
            std::string outputFile = "";
            outputFile += this->outputDirectory1;
            outputFile += fileNames_2[secondId];

            metaItem tempItem;
            tempItem.src = fileNames[secondId];
            tempItem.dst = outputFile;
            q->push(tempItem);
            /*
            meta->startReadingMetadata();

            et->ExtractInfo(fileNames[secondId].c_str());
            TagInfo *info = et->ImageInfo(fileNames[secondId].c_str());
            //int cmdNum3 = et->Complete(5);

            commandi += "-tagsfromfile\n";
            commandi += fileNames[secondId];
            commandi += "\n-all:all";
            commandi += "\n-xmp";
            commandi += "\n-overwrite_original";
            //std::cout << commandi.c_str() << std::endl;
            //std::cout << outputFile.c_str() << std::endl;
            int cmdNum = et->WriteInfo(outputFile.c_str(), commandi.c_str());
            //int cmdNum2 = et->Complete(5);
            //std::cout << cmdNum << " " << cmdNum2 << " " << cmdNum3 << std::endl;
            int cmdNum2 = et->Complete(2);
*/


            cv::warpPerspective(channels_quantizedReflectance[secondId], outputti,  homo, cv::Size(channels_quantizedReflectance[secondId].cols, channels_quantizedReflectance[secondId].rows));
            std::string out2(this->outputDirectory2);
            out2 += fileNames_2[secondId];

            meta->startWritingImage();
            cv::imwrite(out2, outputti,tags);
            meta->endWritingImage();

            mergeVector_quantized.push_back(outputti.clone());
            outputti.release();
/*
            std::string command2(directory_of_executable);
            //std::cout << command << std::endl;
            //exit(1);
            //command += "exifTool/exift"
            command2 += "/exifTool/exiftool -tagsfromfile ";
            command2 += fileNames[secondId]; // This is the original file
            command2 += " -all:all ";
            command2 += " -xmp ";
            command2 += this->outputDirectory2;
            command2 += fileNames_2[secondId]; //this is the output file
            command2 += " -overwrite_original > /dev/null 2>&1";
            //system(command2.c_str());

            commandi = "";
*/
            outputFile = "";
            outputFile += this->outputDirectory2;
            outputFile += fileNames_2[secondId];
/*
            meta->startReadingMetadata();
            et->ExtractInfo(fileNames[secondId].c_str());
            info = et->ImageInfo(fileNames[secondId].c_str());
            //int cmdNum3 = et->Complete(5);

            commandi += "-tagsfromfile\n";
            commandi += fileNames[secondId];
            commandi += "\n-all:all";
            commandi += "\n-xmp";
            commandi += "\n-overwrite_original";
            //std::cout << commandi.c_str() << std::endl;
            //std::cout << outputFile.c_str() << std::endl;
            cmdNum = et->WriteInfo(outputFile.c_str(), commandi.c_str());
            cmdNum2 = et->Complete(2);
            meta->endReadingMetadata();

            */
            tempItem.src = fileNames[secondId];
            tempItem.dst = outputFile;
            q->push(tempItem);
        }
        //std::cout << command << std::endl;

        //exit(1);
}

double fractionToDouble(std::string fraction){

    std::stringstream test(fraction);
    std::string segment;
    //std::vector<std::string> seglist;
    std::vector<double> seglist_double;

    while(std::getline(test, segment, '/')){

        seglist_double.push_back(atof(segment.c_str()));
                       //std::cout << " \n" << seglist_double[seglist_double.size()-1];
    }

    //std::cout << seglist_double[0] << " " << seglist_double[1] << std::endl;

    return seglist_double[0] / seglist_double[1];
}

void altumImage::readImages(bool undisti){

    std::string exftool = directory_of_executable;
    exftool += "/exifTool/exiftool";

    //ExifTool *et2 = new ExifTool(exftool.c_str());

    double pixelSize_mm = 3.45 / 1000.0d;
    std::string dist = "PerspectiveDistortion";
    std::string pp = "PrincipalPoint";
    std::string pfl = "PerspectiveFocalLength";

    std::string black_level = "BlackLevel";
    std::string radiometric_calibration = "RadiometricCalibration";
    std::string vignette_center = "VignettingCenter";
    std::string vignette_polynomial = "VignettingPolynomial";
    std::string exposure_time = "ExposureTime";
    std::string iso_speed = "ISOSpeed";
    //char *spectral_irradiance = "SpectralIrradiance";
    std::string horizontal_irradiance = "HorizontalIrradiance";
    std::string datetime = "DateTimeOriginal";

    for(int j = 0; j < 5; j++){

        channels.push_back(cv::imread(fileNames[j], CV_16UC1));
        channels_quantizedReflectance.push_back(cv::Mat::zeros(channels[0].rows,channels[0].cols,CV_16UC1));
        channels_8bit.push_back(cv::imread(fileNames[j], cv::IMREAD_GRAYSCALE));
        radianceVector.push_back(std::vector<float>(channels_quantizedReflectance[0].cols*channels_quantizedReflectance[0].rows,0));

    }

    merged_quantized_reflectance = cv::Mat::zeros(channels[0].rows,channels[0].cols,CV_16UC3);
    merged_orig = cv::Mat::zeros(channels[0].rows,channels[0].cols,CV_16UC3);
    std::string commandi = "-stay_open";

    for(int j = 0; j < 5; j++){

        std::unordered_map<std::string, std::string> exf;

        meta->startReadingMetadata();

        et->ExtractInfo(fileNames[j].c_str(), commandi.c_str());
        TagInfo *info = et->ImageInfo(fileNames[j].c_str(), commandi.c_str());


        if (info) {



            for (TagInfo *i=info; i; i=i->next) {

                std::string tempString(i->name);
                std::string tempString2(i->value);
                exf[tempString] = tempString2;
                int result1 = strcmp(dist.c_str(), i->name);
                int result2 = strcmp(pp.c_str(), i->name);
                int result3 = strcmp(pfl.c_str(), i->name);


                if(!result1){

                    std::stringstream test(i->value);
                    std::string segment;
                    std::vector<std::string> seglist;
                    std::vector<double> seglist_double;

                    while(std::getline(test, segment, ',')){
                        if(seglist_double.size() != 0)
                            seglist_double.push_back(atof(segment.erase(0,1).c_str()));
                        else
                            seglist_double.push_back(atof(segment.c_str()));
                       //std::cout << " \n" << seglist_double[seglist_double.size()-1];
                    }

                    distortion[j].at<double>(0,0) = seglist_double[0];
                    distortion[j].at<double>(0,1) = seglist_double[1];
                    distortion[j].at<double>(0,2) = seglist_double[3];
                    distortion[j].at<double>(0,3) = seglist_double[4];
                    distortion[j].at<double>(0,4) = seglist_double[2];

                }
                if(!result2){
                    //std::cout << dist << std::endl;
                    //std::cout << i->name << " = " << i->value << std::endl;
                    std::stringstream test(i->value);
                    std::string segment;
                    std::vector<std::string> seglist;
                    std::vector<double> seglist_double;

                    while(std::getline(test, segment, ','))
                    {
                        if(seglist_double.size() != 0)
                            seglist_double.push_back(atof(segment.erase(0,1).c_str()));
                        else
                            seglist_double.push_back(atof(segment.c_str()));
                       //std::cout << " \n" << seglist_double[seglist_double.size()-1];
                    }

                    cameraMatrix[j].at<double>(0,2) = seglist_double[0] / pixelSize_mm;
                    cameraMatrix[j].at<double>(1,2) = seglist_double[1] / pixelSize_mm;

                }
                if(!result3){

                    cameraMatrix[j].at<double>(0,0) = atof(i->value) / pixelSize_mm;
                    cameraMatrix[j].at<double>(1,1) = atof(i->value) / pixelSize_mm;
                }

            }

            // we are responsible for deleting the information when done


        } else if (et->LastComplete() <= 0) {
            std::cerr << "Error executing exiftool22! " << fileNames[j] << std::endl;
        }
        delete info;
        std::stringstream test(exf[black_level]);
        std::string segment;
        std::vector<std::string> seglist;
        std::vector<double> seglist_double;

        while(std::getline(test, segment, ' ')){

            seglist_double.push_back(atof(segment.c_str()));
                       //std::cout << " \n" << seglist_double[seglist_double.size()-1];
        }

        double average = std::accumulate(seglist_double.begin(), seglist_double.end(), 0.0) / seglist_double.size();

        black_levels.push_back(average);


        test.str("");
        test.clear();
        test << exf[vignette_center];
        seglist.clear();
        seglist_double.clear();

        while(std::getline(test, segment, ',')){
            if(seglist_double.size() != 0)
                seglist_double.push_back(atof(segment.erase(0,1).c_str()));
            else
                seglist_double.push_back(atof(segment.c_str()));
                       //std::cout << " \n" << seglist_double[seglist_double.size()-1];
        }
        std::vector<double> seglistvector(seglist_double);
        vignetting_centers.push_back(seglistvector);

        test.str("");
        test.clear();
        test << exf[vignette_polynomial];
        seglist.clear();
        seglist_double.clear();

        while(std::getline(test, segment, ',')){
            if(seglist_double.size() != 0)
                seglist_double.push_back(atof(segment.erase(0,1).c_str()));
            else
                seglist_double.push_back(atof(segment.c_str()));
                       //std::cout << " \n" << seglist_double[seglist_double.size()-1];
        }

        std::vector<double> seglistvector2(seglist_double);
        vignetting_polynomials.push_back(seglistvector2);

        test.str("");
        test.clear();
        test << exf[radiometric_calibration];
        seglist.clear();
        seglist_double.clear();

        while(std::getline(test, segment, ',')){
            if(seglist_double.size() != 0)
                seglist_double.push_back(atof(segment.erase(0,1).c_str()));
            else
                seglist_double.push_back(atof(segment.c_str()));
                       //std::cout << " \n" << seglist_double[seglist_double.size()-1];
        }

        std::vector<double> seglistvector3(seglist_double);


        radiometric_calibrations.push_back(seglistvector3);

        std::istringstream ss(exf[datetime]);
        std::tm t = {};
        ss >> std::get_time(&t, "%Y:%m:%d %H:%M:%S");

        timeStamps.push_back(mktime(&t));

        exposures.push_back(fractionToDouble(exf[exposure_time]));
        gains.push_back(atof(exf[iso_speed].c_str())/100.0);
        double specIrr = atof(exf[horizontal_irradiance].c_str());

        specIrr *= 10000;
        specIrr /= 1000000;
        //std::cout << "spectrali rradiance: " << specIrr << std::endl;
        irradiances.push_back(specIrr);

        //exit(1);
        meta->endReadingMetadata();

        radiometricCalibration(channels[j], channels_quantizedReflectance[j], black_levels[black_levels.size()-1], vignetting_centers[vignetting_centers.size()-1],
                                vignetting_polynomials[vignetting_polynomials.size()-1], radiometric_calibrations[radiometric_calibrations.size()-1],
                                exposures[exposures.size()-1], gains[gains.size()-1], irradiances[irradiances.size()-1], j, radianceVector[j]);

        if(undisti){
            cv::Mat undistort_orig = channels[j].clone(), undistort_8bit = channels_8bit[j].clone(), undistort_quantizedReflectance = channels_quantizedReflectance[j].clone();
            cv::undistort(undistort_orig, channels[j], cameraMatrix[j], distortion[j]);
            cv::undistort(undistort_8bit, channels_8bit[j], cameraMatrix[j], distortion[j]);
            cv::undistort(undistort_quantizedReflectance, channels_quantizedReflectance[j], cameraMatrix[j], distortion[j]);
        }
    }


}

double vignettePolynomialFunction(std::vector<double> vignetting_center, std::vector<double> vignetting_polynomial, int orig_intensity,
                                int x, int y)
{

    double r = sqrt( (x - vignetting_center[0])*(x - vignetting_center[0]) + (y - vignetting_center[1])*(y - vignetting_center[1]));

    double k = 1 +  vignetting_polynomial[0] * r +
                    vignetting_polynomial[1] * (r * r) +
                    vignetting_polynomial[2] * (r * r * r) +
                    vignetting_polynomial[3] * (r * r * r * r) +
                    vignetting_polynomial[4] * (r * r * r * r * r) +
                    vignetting_polynomial[5] * (r * r * r * r * r * r);

    return 1.0 / k;
}

void altumImage::radiometricCalibration(cv::Mat &image,
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
){

    uint16_t pixValue, pixValue4, pixValue_array, pixValue4_array;
    float pixValue2, pixValue3, pixValue2_array, pixValue3_array;
    float newValue, reflectance;
    double V, R, L, norm_L;

    float minReflectance = FLT_MAX, maxReflectance = FLT_MIN;

    double a1 = radiometric_calibration[0],a2 = radiometric_calibration[1],a3 = radiometric_calibration[2];

    uint16_t* uc_pixel = (uint16_t*)image.data;
    //float* uc_pixel2 = (float*)image.data;
    //float* uc_pixel3 = (float*)image.data;
    uint16_t* uc_pixel4 = (uint16_t*)image_quantizedReflectance.data;

    int indexi;

    int rows = image.rows;
    int cols = image.cols;

    for (int i = 0; i < rows; i++)
    {
        //uc_pixel = (uint16_t*)image.data + i*image.step;

        //uint16_t* pixel = image.ptr<uint16_t>(i);  // point to first color in row
        //float* pixel2 = image_float.ptr<float>(i);  // point to first color in row
        //float* pixel3 = image_radiance.ptr<float>(i);  // point to first color in row
        //uint16_t* pixel4 = image_quantizedReflectance.ptr<uint16_t>(i);  // point to first color in row

        for (int j = 0; j < cols; j++)
        {


            indexi = cols * i + j ;

            pixValue = uc_pixel[indexi];


            V = vignettePolynomialFunction(vignetting_center, vignetting_polynomial,
                        pixValue, j,i);

            R = 1.0 / (1.0 + a2 * i / exposure - a3 * i);
            L = V * R * (pixValue - black_levels);

            newValue = (float)L /((float)gain * (float)exposure)*(float)a1/65536.0f;

            radiance_vector[indexi] = newValue;

            if(calibrationPanelLoaded){

                if(this->cP->mode == 3)
                    reflectance = (float)M_PI * newValue / (float)irradiance;

                else if(this->cP->mode == 1){

                    #ifdef DEBUG2
                    if(i == 1383 && j == 835){

                        reflectance = cP->calculateReflectanceWithPanels(newValue, this->timeStamps[0], band, true);

                        std::cout << "name: " << fileNames[band] << std::endl;
                        std::cout << "band: " << band << " radiance: " << pixel3[j] << std::endl;
                        std::cout << "band: " << band << " reflectance: " << reflectance << std::endl;
                        exit(1);

                    }
                    else
                    #endif // DEBUG2
                        reflectance = cP->calculateReflectanceWithPanels(newValue, this->timeStamps[0], band, false);

                }
                else if(this->cP->mode == 2){

                    reflectance = cP->calculateReflectanceWithPanelsAndDLS(newValue, irradiance, this->timeStamps[0], band);

                }
            }else{

                reflectance = (float)M_PI * newValue / (float)irradiance;
                //reflectance = (float)M_PI * newValue / 0.566;

                #ifdef DEBUG2

                if(i == 1114 && j == 1097 && band == 4){

                    std::cout << irradiance << std::endl;
                    std::cout << reflectance << std::endl;
                    exit(1);

                }

                #endif // DEBUG2

            }

            int maxi = 65535, mini = 0;

            int quantizedValue = mini + reflectance * (maxi-mini);

            quantizedValue = std::max(0,quantizedValue);
            quantizedValue = std::min(65535,quantizedValue);

            uc_pixel4[indexi] = quantizedValue;

        }

    }

}
