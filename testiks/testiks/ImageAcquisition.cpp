#include <utility>

//
// Created by kuba on 11/14/18.
//

#include "ImageAcquisition.h"

ImageAcquisition::ImageAcquisition(std::string t_path, int t_rows, int t_cols)
{
    __path = t_path;
    __rows = t_rows;
    __cols = t_cols;
}

int8_t ImageAcquisition::AcquireImage()
{
    //clear histogram values
    __histogramData = std::vector<double>(256,0.0);

    if(__path.empty())
        return 0;

    int counter = 0;
    char value = 0;
    bool header = true;
    unsigned char picture[392544];          // Container acquiring 522 rows and 752 columns

	char const* testtt = __path.data();
	/////////////////////
	FILE* f = fopen(testtt, "rb");
	unsigned char* data = new unsigned char[392544+34]; // allocate 3 bytes per pixel
	fread(data, sizeof(unsigned char), 392544 + 34, f); // read the rest of the data at once
	fclose(f);

	int xaxaxa = strlen((char*)data);
	//hist data
	for (int i = 34; i <= 392544; i++) {
		__histogramData[reinterpret_cast<unsigned char&>(data[i + 34])] ++;
	}
    __image = cv::Mat(__rows,__cols,CV_8U,data+34);   ////insert data read to MAT
    if(! __image.data ) {                                   // Check for invalid input
        return -1;
    }
    return 1;
}

void ImageAcquisition::constructHistogram()
{
    double max;
    double min;

    min = *std::min_element(__histogramData.begin(),__histogramData.end());
    max = *std::max_element(__histogramData.begin(),__histogramData.end());


    for (int i = 0; i < 256; i++)
        __histogramData[i] = 255 * ((__histogramData[i] - min) / (max - min));

    __histogram = cv::Mat::zeros(cv::Size(510, 255), CV_8U);

    for (int i = 0; i < 255; i++)
        cv::line(__histogram, cv::Point(i * 2, 255), cv::Point(i * 2, 255 - (int)__histogramData[i]), cv::Scalar(255, 255, 0), 2, 8, 0);
}