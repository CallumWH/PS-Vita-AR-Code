#include "../ar_app/build/vs2015/MarkerInformation.h"

void MarkerInformation::SetMarkerMatrix(gef::Matrix44 inputMatrix, int markerID)
{
	markerPositions[markerID] = inputMatrix;
}