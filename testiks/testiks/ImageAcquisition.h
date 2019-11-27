#include <utility>
#pragma once
#pragma optimize("",off)
//
// Created by kuba on 11/14/18.
//

#ifndef IMAGEANALYSIS_IMAGEACQUISITION_H
#define IMAGEANALYSIS_IMAGEACQUISITION_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>

class ImageAcquisition {

private:
    std::vector<double> __histogramData;
    std::string __path;
    cv::Mat __histogram;
    cv::Mat __image;
    int __rows;
    int __cols;

public:
    ImageAcquisition(std::string t_path, int t_rows, int t_cols);
    int8_t AcquireImage();

    void constructHistogram();
    void setPath(std::string t_path){__path = std::move(t_path);}

    cv::Mat getImage(){return __image.clone();}
    cv::Mat getHistogram(){return __histogram;}
    std::vector<double> & getHistogramData() {return __histogramData;}
};


#endif //IMAGEANALYSIS_IMAGEACQUISITION_H
#pragma optimize("",on)