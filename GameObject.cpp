#include "../ar_app/build/vs2015/GameObject.h"
#include <maths/aabb.h>
#include <graphics/mesh.h>

GameObject::GameObject(gef::Mesh *mesh, float x = 0.0f, float y = 0.0f, float z = 0.0f, int markerID = 0)
{
	set_mesh(mesh);
	this->localTransform.SetIdentity();
	this->transform_.SetIdentity();
	this->finalTransform.SetIdentity();
	this->parentMarkerMatrix.SetIdentity();
	parentMarkerID = markerID;
	parentMarkerMatrix.SetTranslation(gef::Vector4(0.0f, 0.0f, -0.2f));
	this->SetParentMatrix(parentMarkerMatrix);
	boundingBox = this->mesh()->aabb();
	scaleMatrix.SetIdentity();
}

void GameObject::MoveTo(float x, float y, float z)
{
	//applys the input values to a vector
	gef::Vector4 inputVector = gef::Vector4(x, y, z);
	transform_.SetTranslation(inputVector);
}

void GameObject::LocalMoveTo(gef::Vector4 inputVector)
{
	//changes the local transform to the input vector
	localTransform.SetTranslation(inputVector);
	ApplyTransform();
}

void GameObject::SetLocalMatrix(gef::Matrix44 inputMatrix)
{
	localTransform = inputMatrix;
	ApplyTransform();
}

void GameObject::SetParentMatrix(gef::Matrix44 inputMatrix)
{
	//takes the matrix of a marker and sets it as the parent
	parentMarkerMatrix = inputMatrix;
	ApplyTransform();
}

void GameObject::ApplyVelocity(float x, float y, float z)
{
	float transformX = localTransform.GetTranslation().x();
	float transformY = localTransform.GetTranslation().y();
	float transformZ = localTransform.GetTranslation().z();

	localTransform.SetTranslation(gef::Vector4(transformX + x, transformY + y, transformZ + z));
	ApplyTransform();
}

bool GameObject::AaBbCollision(GameObject* a)
{
	gef::Aabb Atransformed_aabb = a->boundingBox.Transform(a->transform_);
	gef::Aabb Btransformed_aabb = boundingBox.Transform(transform_);

	//Bools to track which axis' are registering collisions
	bool xAxisCollision = false;
	bool yAxisCollision = false;
	bool zAxisCollision = false;

	//checks the various axis' for collisions
	if (Atransformed_aabb.min_vtx().x() <= Btransformed_aabb.max_vtx().x() && Atransformed_aabb.max_vtx().x() >= Btransformed_aabb.min_vtx().x())
	{
		xAxisCollision = true;
	}

	if (Atransformed_aabb.min_vtx().y() <= Btransformed_aabb.max_vtx().y() && Atransformed_aabb.max_vtx().y() >= Btransformed_aabb.min_vtx().y())
	{
		yAxisCollision = true;
	}

	if (Atransformed_aabb.min_vtx().z() <= Btransformed_aabb.max_vtx().z() && Atransformed_aabb.max_vtx().z() >= Btransformed_aabb.min_vtx().z())
	{
		zAxisCollision = true;
	}

	//if all the collisions return true, then there has been a true collision
	if (xAxisCollision && yAxisCollision && zAxisCollision == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void GameObject::ApplyTransform()
{
	finalTransform = localTransform * parentMarkerMatrix;
	set_transform(finalTransform);
	sphereBounds.set_position(finalTransform.GetTranslation());
}

void GameObject::SetScale(gef::Vector4 scalarVector)
{
	scaleMatrix.Scale(scalarVector);
	localTransform = scaleMatrix * localTransform;
}