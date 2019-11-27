#include <utility>

//
// Created by kuba on 11/15/18.
//

#include "PointIdentification.h"



PointIdentification::PointIdentification(int t_rows, int t_cols)
{
	__rows = t_rows;
	__cols = t_cols;
}

uchar PointIdentification::IterativeThresholdSearch(std::vector<double> t_histogramData)
{
	uchar border;
	double leftSideSum;//, rightSideSum, componentsLeft, componentsRight;

	//componentsLeft = 0;
	leftSideSum = 0;
	for (int k = 0; k < t_histogramData.size(); ++k)
		leftSideSum += k * t_histogramData[k];
	border = (uchar)(leftSideSum / t_histogramData.size());

	/*
	for (int j = 0; j < 2; ++j) {
		leftSideSum = 0;
		rightSideSum = 0;
		componentsLeft = 0;
		componentsRight = 0;
		for (int i = 0; i < t_histogramData.size(); ++i)
		{
			if(i <= border)
			{
				componentsLeft += t_histogramData[i];
				leftSideSum += i*t_histogramData[i];
			}
			else
			{
				componentsRight += t_histogramData[i];
				rightSideSum += i*t_histogramData[i];
			}
		}
		border = (int) (leftSideSum/componentsLeft + rightSideSum/componentsRight)/2;
		std::cout << "Our threshold is: " << border << std::endl;
	}*/

	return border;
}

bool PointIdentification::StartingPointSearch(int * t_x, int * t_y, int threshold)
{
	for (int x = *t_x; x < __image.rows; x++)
	{
		for (int y = 0; y < __image.cols; y++) {
			if (__image.at<uchar>(x, y) >= threshold && __labeledImage.at<uchar>(x, y) == 0) {
				*t_x = x;
				*t_y = y;
				return true;
			}
		}
	}
	return false;
}

void PointIdentification::KeyStorage(uchar t_current, uchar t_label)
{
	if (t_label == 0)
	{
		__identifiers.insert(std::pair<int, std::set<int>>(t_current, { t_current }));
		return;
	}

	std::map<int, std::set<int>>::iterator it;

	int position_current = 0;
	int position_label = 0;
	// fist we search trough all labels we already found.
	// if the label is found then we return the value of identifier which we
	// put to the pixel
	for (it = __identifiers.begin(); it != __identifiers.end(); it++)
	{
		for (int i = 0; i < it->second.size(); i++)
		{
			if (it->second.find(t_label) != it->second.end())
				position_label = it->first;
			if (it->second.find(t_current) != it->second.end())
				position_current = it->first;
		}
	}

	if (position_current != position_label)
	{
		__identifiers.at(position_label).insert(__identifiers.at(position_current).begin(),
			__identifiers.at(position_current).end());
		__identifiers.erase(position_current);
	}
}

uchar PointIdentification::KeyRetriever(uchar t_current)
{
	std::map<int, std::set<int>>::iterator it;
	for (it = __identifiers.begin(); it != __identifiers.end(); it++)
		for (int i = 0; i < it->second.size(); i++)
		{
			if (it->second.find(t_current) != it->second.end())
				return it->first;
		}
}

bool PointIdentification::identifyObjects(std::vector<double> t_histogramData, connectivitySpecifier t_specifier)
{
	if (__image.rows != __rows || __image.cols != __cols)
		return false;

	int x = 1, y = 1;
	uchar currentLabel = 1;

	std::vector<int> labels;
	std::map<int, std::vector<int>> map;
	std::vector<std::vector<int>> values;
	__labeledImage = cv::Mat::zeros(__rows, __cols, CV_8U);
	__identifiers.clear();

	switch (t_specifier)
	{
	case EIGHT_CONNECTIVITY:
		values = { {0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1} };  break;
	case FOUR_CONNECTIVITY:
		values = { {0,-1},{1,0},{0,1},{-1,0} };  break;
	}

	auto threshold = PointIdentification::IterativeThresholdSearch(std::move(t_histogramData));

	while (PointIdentification::StartingPointSearch(&x, &y, threshold))
	{
		for (int i = 0; i < values.size(); i++)
			map.insert(std::pair<int, std::vector<int>>(i, values.at(static_cast<unsigned long>(i))));

		int direction = 2;
		int x_cur = x, y_cur = y;
		int nextDirection = 0;
		int i;
		bool directionFound = true;
		int maxIterations = 20000;

		KeyStorage(currentLabel);
		__labeledImage.at<uchar>(x_cur, y_cur) = currentLabel;


		while (directionFound) {
			nextDirection = (direction > 1) ? direction - (int)std::ceil(values.size() / 4) : direction + (int)std::ceil(3 * values.size() / 4);
			std::rotate(values.begin(), values.begin() + nextDirection, values.end());

			directionFound = false;
			for (i = 0; i < values.size(); i++) {
				if (__image.at<uchar>(x_cur + values.at(i)[0], y_cur + values.at(i)[1]) > threshold) {
					if (__labeledImage.at<uchar>(x_cur + values.at(i)[0], y_cur + values.at(i)[1]) == 0 ||
						__labeledImage.at<uchar>(x_cur + values.at(i)[0], y_cur + values.at(i)[1]) == currentLabel)
					{
						if (!directionFound) {
							direction = i;
							directionFound = true;
						}
					}
					else
						KeyStorage(currentLabel, __labeledImage.at<uchar>(x_cur + values.at(i)[0], y_cur + values.at(i)[1]));
					__labeledImage.at<uchar>(x_cur + values.at(i)[0], y_cur + values.at(i)[1]) = currentLabel;
				}
			}
			if (directionFound) {
				x_cur = x_cur + values.at(direction)[0];
				y_cur = y_cur + values.at(direction)[1];
			}

			maxIterations--;
			if (!maxIterations)
				break;
		}
		currentLabel += 1;
	}

	std::cout << "Number of labels: " << __identifiers.size() << std::endl;
	__areasToProcess = std::vector<areaLimits>(currentLabel);

	// Second go and relabeling
	uchar identifier;
	for (int x = 0; x < __rows; x++) {
		for (y = 0; y < __cols; y++) {
			if (__labeledImage.at<uchar>(x, y) > 0)
			{
				identifier = KeyRetriever(__labeledImage.at<uchar>(x, y));
				AssignAreaLimits(x, y, identifier);
				__labeledImage.at<uchar>(x, y) = (uchar)(100 + identifier * 40 / __identifiers.size());
			}
		}
	}

	std::vector<areaLimits>::iterator it;
	for (it = __areasToProcess.begin(); it != __areasToProcess.end(); it++)
		if (it->x_min == UINT16_MAX && it->y_min == UINT16_MAX)
		{
			__areasToProcess.erase(it);
			it = __areasToProcess.begin();
		}

	for (it = __areasToProcess.begin(); it != __areasToProcess.end(); it++)
		//cv::rectangle(__labeledImage, cv::Point(it->y_min, it->x_min), cv::Point(it->y_max, it->x_max), cv::Scalar(255, 255, 255), 1);
	/////////////////////////////////////////

		return true;
}

void PointIdentification::AssignAreaLimits(int t_x, int t_y, uchar t_identifier)
{
	if (t_x < __areasToProcess.at(t_identifier).x_min)
		__areasToProcess.at(t_identifier).x_min = t_x - 1;
	if (t_x > __areasToProcess.at(t_identifier).x_max)
		__areasToProcess.at(t_identifier).x_max = t_x + 1;
	if (t_y < __areasToProcess.at(t_identifier).y_min)
		__areasToProcess.at(t_identifier).y_min = t_y - 1;
	if (t_y > __areasToProcess.at(t_identifier).y_max)
		__areasToProcess.at(t_identifier).y_max = t_y + 1;
}


std::vector<cv::Point> centroids;
void PointIdentification::FindLedCentroids() {

	std::vector<areaLimits>::iterator its;
	int nr = 0;
	for (its = __areasToProcess.begin(); its != __areasToProcess.end(); its++) {

		// convert grayscale to binary image
		cv::Mat tempIm;
		cv::threshold(__labeledImage(cv::Range(its->x_min, its->x_max + 2), cv::Range(its->y_min, its->y_max + 2)), tempIm, 100, 255, cv::THRESH_BINARY);

		// find moments of the image
		cv::Moments m = cv::moments(tempIm, true);
		cv::Point p((m.m10 / m.m00) + its->y_min, (m.m01 / m.m00) + its->x_min);
		// coordinates of centroid
		std::cout << cv::Mat(p) << std::endl;

		// show the image with a point mark at the centroid
		//cv::drawMarker(__labeledImage, p, cv::Scalar(70, 0, 0));
		//
		cv::Point offset(5, 5);
		centroids.push_back(p);
		__labeledImage.at<uchar>(p) = 255;
		cv::putText(__labeledImage, std::to_string(nr), p + offset,
			cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(200, 200, 250), 1);

		namedWindow("Image sub stuff", cv::WINDOW_NORMAL);// Create a window for display.
		cv::imshow("Image sub stuff", tempIm);


		cv::imwrite("C:/Users/T520/Documents/Visual Studio 2017/Projects/testiks/Debug/res/toZoomCentroidsCVtempIm" + std::to_string(nr) + ".jpg", tempIm);// S
		cv::imshow("Image with center", __labeledImage);
		cv::imwrite("C:/Users/T520/Documents/Visual Studio 2017/Projects/testiks/Debug/res/toZoomCentroidsCV" + std::to_string(nr++) + ".jpg", __labeledImage);// S




//cv::waitKey(0);



		//m_Horizontal.resize(its->x_max - its->x_min , 0);
		//m_Vertical.resize(its->y_max - its->y_min, 0);
		//std::cout << "Resized vertical Y is: " << its->y_max - its->y_min << std::endl;
		//for (int y = its->y_min; y < its->y_max; y++)
		//{
		//	for (int x = its->x_min; x < its->x_max; x++)
		//	{
		//		//m_M11 += x * y*__labeledImage.at<uchar>(y, x);
		//		if (!__labeledImage.at<uchar>(x, y))
		//		{
		//			m_Vertical.at(y- its->y_min) += 1;
		//			m_Horizontal.at(x- its->x_min) += 1;
		//		}
		//	}
		//}

		//for (int i = 0; i < m_Horizontal.size(); i++)
		//{
		//	m_M00 += m_Horizontal.at(i);
		//	m_M10 += i * m_Horizontal.at(i);
		//	m_M20 += i * i * m_Horizontal.at(i);
		//}

		//for (int i = 0; i < m_Vertical.size(); i++)
		//{
		//	m_M01 += i * m_Vertical.at(i);
		//	m_M02 += i * i * m_Vertical.at(i);
		//}

		//int X = m_M10 / m_M00;
		//int Y = m_M01 / m_M00;
		//// and display now 
		////double angle = tan((2 * mu11) / (mu20 - mu02));
		////double length = 60;
		////cv::line(__labeledImage, cv::Point(X, Y), cv::Point(X + length * cos(angle), Y + length * sin(angle)), cv::Scalar(133, 0, 0));
		////cv::drawMarker(__labeledImage, cv::Point( its->y_min+Y, its->x_min+X), cv::Scalar(70, 0, 0));
		////cv::drawMarker(__labeledImage, cv::Point( its->y_min, its->x_min), cv::Scalar(133, 0, 0));
		////cv::drawMarker(__labeledImage, cv::Point(its->y_max, its->x_max), cv::Scalar(133, 0, 0));

		//std::cout << "Our centroid X is: " << X << std::endl;
		//std::cout << "Our centroid Y is: " << Y << std::endl;
		//namedWindow("Centroids", cv::WINDOW_AUTOSIZE);// Create a window for display.
		//imshow("Centroids", __labeledImage);   
		//cv::imwrite("C:/Users/T520/Documents/Visual Studio 2017/Projects/testiks/Debug/res/toZoomCentroids.jpg", __labeledImage);// S
		//cv::waitKey(0);
	}

	/// TEST AREA 
	//[ Ax * (By - Cy) + Bx * (Cy - Ay) + Cx * (Ay - By) ] / 2
	int isCollinear = (centroids.at(7).x* (centroids.at(3).y - centroids.at(1).y) + centroids.at(3).x  * (centroids.at(1).y - centroids.at(7).y) + centroids.at(1).x  * (centroids.at(7).y - centroids.at(3).y)) / 2;


	std::cout << "isCollinear:" << isCollinear << std::endl;



	std::cout << "DISTANCEx from 1 to 3 :" << centroids.at(1).x - centroids.at(7).x << std::endl;
	std::cout << "DISTANCEx from 3 to 1 :" << centroids.at(7).x - centroids.at(1).x << std::endl;
	std::cout << "DISTANCEy from 1 to 3 :" << centroids.at(1).y - centroids.at(7).y << std::endl;
	std::cout << "DISTANCEy from 3 to 1 :" << centroids.at(7).y - centroids.at(1).y << std::endl;

	//cv::line(__labeledImage, centroids.at(1), centroids.at(3), cv::Scalar(133, 0, 0));
	//cv::line(__labeledImage, centroids.at(3), centroids.at(7), cv::Scalar(100, 0, 0));
	//cv::line(__labeledImage, centroids.at(7), centroids.at(1), cv::Scalar(70, 0, 0));

	cv::imshow("lines", __labeledImage);

	cv::imwrite("C:/Users/T520/Documents/Visual Studio 2017/Projects/testiks/Debug/res/lines.jpg", __labeledImage);// S
	cv::waitKey(0);
}

struct colinearPoints {
	cv::Point p1;
	cv::Point p2;
	cv::Point p3;
};

struct colinearPointsInt {
	int p1;
	int p2;
	int p3;
};

inline bool operator<(const colinearPointsInt& lhs, const colinearPointsInt& rhs)
{
	return (lhs.p1 < rhs.p1 || lhs.p2 < rhs.p2 || lhs.p3 < rhs.p3);

}


int unique = 0;
//Of course welcome to optimize. :/set <int, greater <int> > gquiz1;         
void PointIdentification::confirmPattern() {
	std::set <colinearPointsInt> sortedColinearPointsInt;
	colinearPoints _colinearPoints;
	colinearPointsInt _colinearPointsInt;

	// Maybe add switch or if case for the number of present centroids. :/ if less than 6 , 
	// kinda ignore , as we cannot distinguish orientation or position of "dice" thing
	// do some stuff, maybe first find the center point of "dice" pattern. OR maybe find corner point of dice , as it will have all 3 points in a line in 3 cases.
	// OK , dice ended up being not perfect , so ignoring it for now., Main idea in code below, to find Which centroids are collinear(on the same line) 
	// Once found, Do something about it :D , now it finds them by taking combinations of 3 different centroids.

	for (unsigned i = 0; i < centroids.size(); i++) {
		for (unsigned i2 = 0; i2 < centroids.size(); i2++) {
			for (unsigned i3 = 0; i3 < centroids.size(); i3++) {
				if (centroids.at(i) != centroids.at(i2) && centroids.at(i3) != centroids.at(i2) && centroids.at(i) != centroids.at(i3)) {
					// calculate area of triangle formed by 3 points
					//[ Ax * (By - Cy) + Bx * (Cy - Ay) + Cx * (Ay - By) ] / 2
					int isCollinear = (centroids.at(i).x * (centroids.at(i2).y - centroids.at(i3).y) + centroids.at(i2).x  * (centroids.at(i3).y - centroids.at(i).y) + centroids.at(i3).x  * (centroids.at(i).y - centroids.at(i2).y)) / 2;
					//std::cout << "isCollinear:" << isCollinear << std::endl;

					//assumption is made , that centroids or maybe leds are not perfectly alligned, so some deviation exists
					//Therefore another assumption that calculated area will be more than 0 , but if area is less than ammount of pixels between point 1 and 3
					// it is on a straight line.....
					int distancex1to3 = (centroids.at(i).x - centroids.at(i3).x);
					int distancey1to3 = (centroids.at(i).y - centroids.at(i3).y);

					int distanceToCompare;
					if (abs(distancex1to3) > abs(distancey1to3)) {
						distanceToCompare = (abs(distancex1to3));
					}
					else {
						distanceToCompare = (abs(distancey1to3));
					}

					// in perfect conditions  isCollinear must be 0 
					if (abs(isCollinear) >= 0 && abs(isCollinear) < distanceToCompare) {
						//cv::putText(__labeledImage, "COL P1:"+std::to_string(i)+"P2:" + std::to_string(i2) + "P3:" + std::to_string(i3), temp,
						//	cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(200, 200, 250), 1);

						cv::imshow("CCCCCc", __labeledImage);
						std::cout << "YES COL P1: " + std::to_string(i) + " P2: " + std::to_string(i2) + " P3: " + std::to_string(i3) << std::endl;
						cv::line(__labeledImage, centroids.at(i), centroids.at(i2), cv::Scalar(133, 0, 0));
						cv::line(__labeledImage, centroids.at(i2), centroids.at(i3), cv::Scalar(100, 0, 0));
						cv::line(__labeledImage, centroids.at(i3), centroids.at(i), cv::Scalar(70, 0, 0));

						//sort points to keep track of repeating ones
						std::vector<int> numbers;
						numbers.push_back(i);
						numbers.push_back(i2);
						numbers.push_back(i3);
						std::sort(numbers.begin(), numbers.end());
						_colinearPointsInt.p1 = numbers.at(0);
						_colinearPointsInt.p2 = numbers.at(1);
						_colinearPointsInt.p3 = numbers.at(2);
						sortedColinearPointsInt.insert(_colinearPointsInt);
						///////////////// Ok at this point only sorted instances of colinears



						cv::waitKey(0);

					}
					else {
						//	std::cout << "NO" << std::endl;
					}
				}
			}
		}
		// When confirmed find the rest points inside area of Reactangle
		// Leave only related.
	}/// LOOP FINISHED
	/// Lets try to find 3 centroid sets  from availiable set, where 1 point is the same, it could be origin of mire pattern
	//
	int hist[MAX_HIST_VAL] = {};  // Full kinda histogram  maybe  for further analysis
	for (std::set <colinearPointsInt>::iterator it = sortedColinearPointsInt.begin(); it != sortedColinearPointsInt.end(); ++it)
	{
		hist[it->p1]++;
		hist[it->p2]++;
		hist[it->p3]++;
	}
	// For now assume point with highest value the origin
	cv::Point originPoint=centroids.at( std::max_element(hist, hist + MAX_HIST_VAL) - hist);
	
	///
	//Check angles


	///
	//Find ratios


	///
	// NEED SOME SOLLUTION TO KEEP TRACK OF COLLINEAR POINTS. Also  think how to remove colinear points , which are not part of LED MIRE things
	/// WHEN ALL COMPUTED , NEXT STEP SHOUL BE CHEKING THE RATIOS BETWEEN POINTS , 
	// If wanted ratios , than it is our thing. 





	cv::waitKey(0);
}