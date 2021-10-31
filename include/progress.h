#ifndef PROGRESS_H
#define PROGRESS_H
#include <iostream>
#include <chrono>
#include <limits.h>
#include <climits>

class progress
{
    public:
        progress();
        virtual ~progress();

        int n_images;
        int prog = 0;

        int elapsedTime;
        float timePerImage;
        int reminingTime = 99999;

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        void reset(){

            prog = 0;
            begin = std::chrono::steady_clock::now();

        }

        void updateProgress(){

            prog++;
            int percent = (100 * (prog)) / n_images;

            end = std::chrono::steady_clock::now();
            elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();

            reminingTime = elapsedTime / prog * (n_images - prog);

            std::cout << "\r" << "[" << std::string(percent / 5, (char)254u) << std::string(100 / 5 - percent / 5, ' ') << "]";
			std::cout << percent << "%" << " [Image " << prog << " of " << n_images << "] elapsed: " << elapsedTime << "s remaining: " << reminingTime << "s";

			std::cout.flush();
			//displayNext += step;

        };

        void setEnd(int endi){

            prog = 0;
            n_images = endi;

        };

    protected:

    private:
};

#endif // PROGRESS_H
