#include "calibrationPanels.h"

//pthread_mutex_t m;

calibrationPanels::calibrationPanels(std::vector<double> panelReflectances, int mode)
{
    //ctor
    this->reflectance = panelReflectances;
    this->mode = mode;

    for(int i = 0; i < 5; i++){

        allImageIrradiances.push_back(std::vector<double>());

    }
}

calibrationPanels::~calibrationPanels()
{
    //dtor
}

