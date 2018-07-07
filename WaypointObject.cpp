#include "WaypointObject.h"

//class constructor, takes in a mesh and position to create a game object
WaypointObject::WaypointObject(gef::Mesh* mesh, float x, float y, float z, int markerID) : GameObject(mesh, x, y, z, markerID)
{
	waypointInit = false;
	distance = 0.0f;
	targetMarkerID = 0;
	objectHP = 50;
}

void WaypointObject::UpdateWaypoints(MarkerInformation* markerContainer)
{
	//default the waypoint to waypoint 1 when follow waypoint is first called
	if (!waypointInit)
	{
		currentWaypoint = markerContainer->GetMarkerMatrix(0);
		SetParentMatrix(currentWaypoint);
		targetMarkerID = 1;
		waypointInit = true;
	}

	//Check if the parent is visible
	if (!markerContainer->GetMarkerVisible(GetParentID()))
	{
		//if the parent is not visible, look for a marker that is
		for (int i = 0; i < ARRAY_SIZE; i++)
		{
			if (markerContainer->GetMarkerVisible(i))
			{
				//Takes the position of the selected marker and sets it to be the local co-ordinates
				LocalMoveTo(GetMarkerLocalPosition(markerContainer->GetMarkerMatrix(i)).GetTranslation());

				//set the matrix of the marker as the new parent
				SetParentMatrix(markerContainer->GetMarkerMatrix(i));
				SetParentID(i);
				break;
			}
		}
	}
	else
	{
		//update the parent location if it is visible
		SetParentMatrix(markerContainer->GetMarkerMatrix(GetParentID()));
	}

	//get the matrix for the waypoint to be moved to

	if (markerContainer->GetMarkerVisible(targetMarkerID))
	{
		targetWaypointMatrix = markerContainer->GetMarkerMatrix(targetMarkerID);
	}
	else
	{
		int parentID = GetParentID();
		gef::Matrix44 local = markerContainer->GetMarkerRelativeMatrix(parentID, targetMarkerID);
		targetWaypointMatrix = local * parentMarkerMatrix;
	}
}

void WaypointObject::FollowWaypoints()
{
	//put the local co-ordinates into a vector to be used with the apply vector
	gef::Matrix44 localTransformTarget = GetMarkerLocalPosition(targetWaypointMatrix);
	gef::Vector4 finalVector = localTransformTarget.GetTranslation();

	//invert the values so it gives a vector from object to marker rather than marker to object
	finalVector.set_x(finalVector.x()*-1.0f);
	finalVector.set_y(finalVector.y()*-1.0f);
	finalVector.set_z(finalVector.z()*-1.0f);

	//calculate the distance between the object and the target marker
	distance = gef::Vector4(targetWaypointMatrix.GetTranslation() - transform_.GetTranslation()).Length();

	//make sure there's no 0 values being passed in
	if (finalVector.x() != 0 && finalVector.y() != 0 && finalVector.z() != 0)
	{
		//rate of movement variable
		float speed = 1000.0f;

		//check if the object is at the marker
		if (distance > 0.015f)
		{
			//moves the object towards the marker via a normalised vector
			finalVector.Normalise();
			finalVector /= speed;

			//put this into a matrix
			gef::Matrix44 tempMatrix;
			tempMatrix.SetIdentity();

			//hand the matrix the local coordinates of object to marker
			tempMatrix.SetTranslation(finalVector);


			//set the matrix
			SetLocalMatrix(tempMatrix * GetMarkerLocalPosition(parentMarkerMatrix));

			//ApplyVelocity(finalVector.x() / speed, finalVector.y() / speed, finalVector.z() / speed);
		}
		else
		{
			AdvanceWaypoint();
		}
	}
}

void WaypointObject::AdvanceWaypoint()
{
	//advances to the next waypoint
	targetMarkerID++;

	if (targetMarkerID > 5)
	{
		targetMarkerID = 0;
	}
}

int WaypointObject::GetCurrentTarget()
{
	//give back the waypoint ID that is currently being targeted
	return targetMarkerID;
}

float WaypointObject::GetDistanceToTarget()
{
	//returns the distance from object to waypoint target
	return distance;
}

gef::Matrix44 WaypointObject::GetMarkerLocalPosition(gef::Matrix44 markerMatrix)
{
	//inverts the matrix for the marker
	gef::Matrix44 inverse_objectMatrix = markerMatrix;
	inverse_objectMatrix.AffineInverse(markerMatrix);

	//calculate the local co-ordinate of the target marker
	gef::Matrix44 localTargetTransform = this->transform_ * inverse_objectMatrix;
	return localTargetTransform;
}