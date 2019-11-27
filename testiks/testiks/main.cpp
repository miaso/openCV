#include "ImageAcquisition.h"
#include "PointIdentification.h"


int main( int argc, char** argv )
{

    std::string path[] = {//"C:/Users/T520/Documents/Visual Studio 2017/Projects/testiks/Debug/res/shortRange.unc",
                          //"/home/kuba/Documents/ImageAnalysis/res/shortRange2.unc",
                          "C:/Users/T520/Documents/Visual Studio 2017/Projects/testiks/Debug/res/shortRange3.unc",
                          //"C:/Users/T520/Documents/Visual Studio 2017/Projects/testiks/Debug/res/shortRange4.unc"
	};

    ImageAcquisition ImageLoader(path[0],522,752);
    PointIdentification PointIdentifier(522,752);

	cv::Mat image, histogram;
	cv::Mat objects = cv::Mat::zeros(522,752,CV_8U);
    for (const auto &i : path) {
        ImageLoader.setPath(i);

        if(!ImageLoader.AcquireImage())
        {
            std::cout << "Unable to load given image" << std::endl;
            return -1;
        }

		// Actuiring images and outputs for displaying
		image = ImageLoader.getImage();
        PointIdentifier.setImage(image);

        
        

        // That is here just to show additional info. Currently not useful as I expected
       ImageLoader.constructHistogram();
       histogram = ImageLoader.getHistogram();

	   if (!PointIdentifier.identifyObjects(ImageLoader.getHistogramData(), PointIdentifier.FOUR_CONNECTIVITY))
	   {
		   std::cout << "Image for analysis was not set" << std::endl;
		   return -1;
	   }


       //  Objects will contain highlighted objects which were identified in an image
	 
	  objects = PointIdentifier.getImage();
	

        namedWindow( "picture", cv::WINDOW_AUTOSIZE );// Create a window for display.
        imshow( "picture", image );                   // Show our image inside it.

        namedWindow( "histogram", cv::WINDOW_AUTOSIZE );// Create a window for display.
        imshow( "histogram", histogram );                   // Show our image inside it.

        namedWindow( "objects", cv::WINDOW_AUTOSIZE );// Create a window for display.
        imshow( "objects", objects );                   // Show our image inside it.
        cv::waitKey(0);                      // Wait for a keystroke in the window
		

		//
		PointIdentifier.FindLedCentroids();
		PointIdentifier.confirmPattern();
		cv::imwrite("C:/Users/T520/Documents/Visual Studio 2017/Projects/testiks/Debug/res/toZoom.jpg", objects);
    }
    return 0;
}
