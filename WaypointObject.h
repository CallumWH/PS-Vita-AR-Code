#pragma once
#ifndef GAMEPLAY_H
#define GAMEPLAY_H
#include "GameObject.h"

class WaypointObject : public GameObject
{
public:
	//public functions
	WaypointObject(gef::Mesh* mesh, float x, float y, float z, int markerID);

	void UpdateWaypoints(MarkerInformation* markerContainer);
	void FollowWaypoints();
	void AdvanceWaypoint();
	int GetCurrentTarget();
	float GetDistanceToTarget();
	int GetObjectHP() { return objectHP; }
	void SetObjectHP(int hpValue) { objectHP = hpValue; }
	gef::Matrix44 GetMarkerLocalPosition(gef::Matrix44 marker);
	

private:
	//private variables
	
	gef::Matrix44 targetWaypointMatrix;
	gef::Matrix44 currentWaypoint;

	bool waypointInit;
	float distance;
	int targetMarkerID;
	int objectHP;

	//private functions
	

};

#endif //GAMEOBJECT_H