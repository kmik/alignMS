#ifndef CALIBRATIONPANELS_H
#define CALIBRATIONPANELS_H
#include <vector>
#include <stdio.h>
#include <iostream>
#include <numeric>
#include <math.h>
#include <map>
#include <pthread.h>
#include <thread>
#include <mutex>

struct paneldata{

    double time;
    std::vector<double> radiance;
    std::vector<double> meanReflectance;
    std::vector<double> sdReflectance;


};

class calibrationPanels
{

    std::mutex m;

    public:
        calibrationPanels(std::vector<double> panelReflectances, int mode);
        virtual ~calibrationPanels();

        std::vector<double> reflectance;
        std::vector<std::vector<double>> radiance;
        std::vector<std::vector<double>> meanReflectance;
        std::vector<std::vector<double>> sdReflectance;



        std::vector<std::vector<double>> panelIrradiance;
        //std::vector<std::vector<double>> reflectance;
        std::vector<double> times;
        std::vector<std::vector<double>> dlsCorrections;
        std::vector<std::vector<double>> radianceToReflectanceFactor;
        std::vector<std::vector<double>> allImageIrradiances;
        std::vector<double> allImageTimes;
        std::map<double, paneldata> panelMap;

        int mode;

        void addPanel(std::vector<double> meanRadiance, int time, std::vector<double> meanReflectance1, std::vector<double> sdReflectance1){

            m.lock();
            radiance.push_back(std::vector<double>());
            meanReflectance.push_back(std::vector<double>());
            sdReflectance.push_back(std::vector<double>());

            for(int i = 0; i < meanRadiance.size(); i++){

                radiance[radiance.size()-1].push_back(meanRadiance[i]);
                meanReflectance[meanReflectance.size()-1].push_back(meanReflectance1[i]);
                sdReflectance[sdReflectance.size()-1].push_back(sdReflectance1[i]);

            }

            paneldata p_temp;
            p_temp.time = (double)time;
            p_temp.radiance = radiance[radiance.size()-1];
            p_temp.meanReflectance = meanReflectance[meanReflectance.size()-1];
            p_temp.sdReflectance = sdReflectance[sdReflectance.size()-1];

            panelMap[time] = p_temp;

            times.push_back((double)time);


            m.unlock();
        }

        void addIrradianceReading(std::vector<double> irradiance){

            //allImageIrradiances.push_back(std::vector<double>());
            m.lock();

            for(int i = 0; i < irradiance.size(); i++)
                allImageIrradiances[i].push_back(irradiance[i]);

            m.unlock();
        }

        void addPanelReflectance(std::vector<double> reflectances){

            for(int i = 0; i < reflectances.size(); i++)
                reflectance.push_back(reflectances[i]);

        }

        void processPanels(){


            int counter = 0;
/*
            std::cout << times[0] << " " << times[1] << std::endl;
            std::cout << (times[0] - times[1]) << std::endl;

            //panelMap[times[1]].

            std::cout << panelMap[times[0]].radiance[0] << std::endl;
            std::cout << panelMap[times[0]].radiance[1] << std::endl;
            std::cout << panelMap[times[1]].radiance[0] << std::endl;
            std::cout << panelMap[times[1]].radiance[1] << std::endl;
            std::cout << radiance[0][0] << std::endl;
            std::cout << radiance[0][1] << std::endl;
            radiance[0] = panelMap[times[1]].radiance;
            std::cout << " -------------- " << std::endl;
            std::cout << radiance[1][0] << std::endl;
            std::cout << radiance[1][1] << std::endl;
            std::cout << radiance[0][0] << std::endl;
            std::cout << radiance[0][1] << std::endl;

            std::cout << " -------------- " << std::endl;

            std::cout << panelMap[times[0]].radiance[0] << std::endl;
            std::cout << panelMap[times[0]].radiance[1] << std::endl;
            std::cout << panelMap[times[1]].radiance[0] << std::endl;
            std::cout << panelMap[times[1]].radiance[1] << std::endl;
*/

            for(auto i : panelMap){

                times[counter] = i.second.time;
                radiance[counter] = i.second.radiance;
                meanReflectance[counter] = i.second.meanReflectance;
                sdReflectance[counter] = i.second.sdReflectance;
                counter++;

            }

            //std::cout << times[0] << " " << times[1] << std::endl;
            //std::cout << (times[0] - times[1]) << std::endl;
            //exit(1);
            std::vector<double> dlsIrradi;

            for(int i = 0; i < allImageIrradiances.size(); i++){

                double sum = std::accumulate(allImageIrradiances[i].begin(), allImageIrradiances[i].end(), 0.0);
                double mean = sum / allImageIrradiances[i].size();
                dlsIrradi.push_back(mean);
            }

            for(int i = 0; i < radiance[0].size(); i++){

                radianceToReflectanceFactor.push_back(std::vector<double>());
                dlsCorrections.push_back(std::vector<double>());

            }

            for(int i = 0; i < radiance.size(); i++){

                //radianceToReflectanceFactor.push_back(std::vector<double>());
                //dlsCorrections.push_back(std::vector<double>());
                //std::cout << radiance[i].size() << std::endl;
                //exit(1);
                for(int j = 0; j < radiance[i].size(); j++){

                    double irrFromPanel = M_PI * radiance[i][j] / reflectance[j];

                    dlsCorrections[j].push_back(irrFromPanel / dlsIrradi[j]);
                    radianceToReflectanceFactor[j].push_back(reflectance[j] / radiance[i][j]);

                    std::cout << "factor " << j << " : " << (reflectance[j] / radiance[i][j]) << std::endl;

                }

            }

        }

        double interpolate( std::vector<double> &xData, std::vector<double> &yData, double x, bool extrapolate )
        {
           int size = xData.size();

           int i = 0;                                                                  // find left end of interval for interpolation
           if ( x >= xData[size - 2] )                                                 // special case: beyond right end
           {
              i = size - 2;
           }
           else
           {
              while ( x > xData[i+1] ) i++;
           }
           double xL = xData[i], yL = yData[i], xR = xData[i+1], yR = yData[i+1];      // points on either side (unless beyond ends)
           if ( !extrapolate )                                                         // if beyond ends of array and not extrapolating
           {
              if ( x < xL ) yR = yL;
              if ( x > xR ) yL = yR;
           }

           double dydx = ( yR - yL ) / ( xR - xL );                                    // gradient

           return yL + dydx * ( x - xL );                                              // linear interpolation
        }

        double calculateReflectanceWithPanels(double radiance, int time, int band, bool print){

            //std::cout << " here 1  " << std::endl;
            double returni;

            if(radianceToReflectanceFactor[0].size() > 1){

                //if(band == 2)
                   // std::cout << "start " << radianceToReflectanceFactor[band].size() << std::endl;
                returni = interpolate(times, radianceToReflectanceFactor[band], time, true);
                returni *= radiance;
                //if(band == 2)

/*
                std::cout << "interpolated : " << returni << std::endl;
                std::cout << "norm: " << (radiance * radianceToReflectanceFactor[band][0]) << std::endl;
                std::cout << radianceToReflectanceFactor[band][0] << " " << radianceToReflectanceFactor[band][1] << std::endl;
                std::cout << (radiance * radianceToReflectanceFactor[band][0]) << " " << (radiance * radianceToReflectanceFactor[band][1]) << std::endl;
                std::cout << "end " << std::endl;
*/
            }else{

                returni = radiance * radianceToReflectanceFactor[band][0];
                if(print){

                    std::cout << radianceToReflectanceFactor[band][0] << std::endl;

                }

            }

            //std::cout << " here 2  " << std::endl;
            return returni;
        }

        double calculateReflectanceWithPanelsAndDLS(double radiance, double dlsIrradiance, int time, int band){

            double dlscorr;

            if(radianceToReflectanceFactor[0].size() > 1){

                 dlscorr = interpolate(times, dlsCorrections[band], time, true);

            }else{

                dlscorr = dlsIrradiance * dlsCorrections[band][0];
                //std::cout << "dlscorr " << dlscorr << std::endl;
            }

            return M_PI * radiance / (dlsIrradiance * dlscorr);

        }

    protected:

    private:
};

#endif // CALIBRATIONPANELS_H
