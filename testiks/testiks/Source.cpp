#include <stdio.h> 
#include "Exercise1.h" 
#include "Exercise2.h"
#include "Exercise3.h"
#include "Exercise4.h"
#include "Exercise5.h"
#include <conio.h> 
#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "imageAcquisition.h"
#include "PointIdentification.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>



using namespace cv;
using namespace std;


int main(int argc, char* argv[])
{

	std::string imageName[] = {   "../Debug/Exersices/shortRange.unc",
								  "../Debug/Exersices/shortRange3.unc",
								  "../Debug/Exersices/shortRange4.unc "}; 

	ImageAcquisition ImageLoader(imageName[0], 522, 752);
	PointIdentification PointIdentifier(522, 752);

	cv::Mat image, histogram, objects;
	for (const auto &i : imageName) {
		ImageLoader.setPath(i);

		if (!ImageLoader.AcquireImage())
		{
			std::cout << "Unable to load given image" << std::endl;
			return -1;
		}

		PointIdentifier.setImage(ImageLoader.getImage());

		// Actuiring images and outputs for displaying
		image = ImageLoader.getImage();

		// That is here just to show additional info. Currently not useful as I expected
		ImageLoader.constructHistogram();
		histogram = ImageLoader.getHistogram();

	

		// Objects will contain highlighted objects which were identified in an image
		objects = PointIdentifier.getImage();

		namedWindow("picture", cv::WINDOW_AUTOSIZE);// Create a window for display.
		imshow("picture", image);                   // Show our image inside it.

		namedWindow("histogram", cv::WINDOW_AUTOSIZE);// Create a window for display.
		imshow("histogram", histogram);                   // Show our image inside it.

		//namedWindow("objects", cv::WINDOW_AUTOSIZE);// Create a window for display.
		//imshow("objects", objects);                   // Show our image inside it.

		cv::waitKey(0);                                          // Wait for a keystroke in the window
	}





	return 0;
}