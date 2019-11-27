#include "ImageAcquisition.h"
#include <map>
#include <set>
#pragma once
//
// Created by kuba on 11/15/18.
//

#ifndef IMAGEANALYSIS_POINTIDENTIFICATION_H
#define IMAGEANALYSIS_POINTIDENTIFICATION_H


class PointIdentification {
private:
	struct areaLimits {
		int x_min = UINT16_MAX;
		int x_max = 0;
		int y_min = UINT16_MAX;
		int y_max = 0;
	};

	std::vector<areaLimits> __areasToProcess;

	uchar IterativeThresholdSearch(std::vector<double> t_histogramData);
	bool StartingPointSearch(int*t_x, int*t_y, int threshold);
	void KeyStorage(uchar t_current, uchar t_label = 0);
	uchar KeyRetriever(uchar t_current);
	void AssignAreaLimits(int t_x, int t_y, uchar t_identifier);

	cv::Mat __image;
	cv::Mat __labeledImage;

	int __rows;
	int __cols;

	std::map<int, std::set<int>> __identifiers;

	////
	std::vector<int> m_Vertical;
	std::vector<int> m_Horizontal;
	cv::Moments m_Moments;
	///

public:
	enum connectivitySpecifier {
		FOUR_CONNECTIVITY,
		EIGHT_CONNECTIVITY,
	};

	PointIdentification(int t_rows, int t_cols);
	void setImage(cv::Mat t_image) { __image = t_image; }
	cv::Mat getImage() { return __labeledImage; }
	bool identifyObjects(std::vector<double> t_histogramData, connectivitySpecifier t_specifier);
	void FindLedCentroids();
	/////////

	double m_M00;
	double m_M01;
	double m_M10;
	double m_M11;
	double m_M02;
	double m_M20;
	double mu00, mu11, mu20, mu02;
	void confirmPattern();
	//////////
#define MAX_HIST_VAL  64
};

#endif //IMAGEANALYSIS_POINTIDENTIFICATION_H
