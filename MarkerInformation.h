#pragma once
#ifndef MARKERINFORMATION_H
#define MARKERINFORMATION_H

#define ARRAY_SIZE		6

#include <maths\matrix44.h>
#include <system/application.h>

class MarkerInformation
{
public:
	//Utility functions for getting and setting
	//returns the matrix for the markerID given
	gef::Matrix44 GetMarkerMatrix(int markerID) { return markerPositions[markerID]; }

	//returns the relative transformations for marker A to marker B
	gef::Matrix44 GetMarkerRelativeMatrix(int parentID, int targetID) { return markerRelativePosition[parentID][targetID]; }

	//returns the visibility state of a marker
	bool GetMarkerVisible(int markerID) { return markerFound[markerID]; }

	//Most of these are used in ar_app.cpp for updating marker data
	//sets the relative position of a marker
	void SetMarkerRelativePosition(int parentID, int targetID, gef::Matrix44 localMatrix) { markerRelativePosition[parentID][targetID] = localMatrix; }

	//sets the visibility state of a marker
	void SetMarkerVisible(int markerID, bool visible) { markerFound[markerID] = visible; }

	//updates the marker's matrix
	void SetMarkerMatrix(gef::Matrix44 markerPosition, int markerID);

private:
	//Information stored within the class
	gef::Matrix44 markerPositions[ARRAY_SIZE];
	gef::Matrix44 markerRelativePosition[ARRAY_SIZE][ARRAY_SIZE];
	bool markerFound[ARRAY_SIZE];
};

#endif //MARKERINFORMATION_H