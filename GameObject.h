#pragma once
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include <graphics\model.h>
#include <graphics\mesh_instance.h>
#include <maths\matrix44.h>
#include <maths\sphere.h>
#include <maths\aabb.h>
#include <graphics/mesh.h>
#include "../ar_app/build/vs2015/MarkerInformation.h"

class GameObject : public gef::MeshInstance
{

public:

	GameObject(gef::Mesh *mesh, float x, float y, float z, int markerID); //parameters go in here for constructor;

	//model declarations
	gef::Model model;
	gef::Vector4 velocity;

	//transform matrices
	gef::Matrix44 finalTransform;
	gef::Matrix44 localTransform;
	gef::Matrix44 parentMarkerMatrix;
	gef::Matrix44 scaleMatrix;

	//Sphere for the object
	gef::Sphere sphereBounds;

	//bounding box for the object
	gef::Aabb boundingBox;

	//moves an object to the set location
	void MoveTo(float x, float y, float z);

	//moves an object in local space
	void LocalMoveTo(gef::Vector4 inputVector);

	//Moves and object independent of rotation
	void ApplyVelocity(float x, float y, float z);

	//takes the matrix to be used as a parent transform
	void SetParentMatrix(gef::Matrix44 inputMatrix);

	//Sets Local Matrix (DO NOT USE)
	void SetLocalMatrix(gef::Matrix44 inputMatrix);

	//returns true / false depending on the collision state with the target object
	bool AaBbCollision(GameObject* targetObject);

	//applies any new matrix calculations
	void ApplyTransform();

	//Getters and Setters
	int GetParentID() { return parentMarkerID; }

	void SetParentID(int markerID) { parentMarkerID = markerID; }

	void SetScale(gef::Vector4);

private:
	//ID of the current parent
	int parentMarkerID;
};

#endif //GAMEOBJECT_H