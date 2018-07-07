#pragma once
#ifndef PROJECTILE_H
#define PROJECTILE_H
#include "GameObject.h"

class Projectile : public GameObject
{
public:
	Projectile(gef::Mesh* mesh, float x, float y, float z, int markerID);
	void Update(MarkerInformation* markerContainer);
	gef::Matrix44 GetMarkerLocalPosition(gef::Matrix44 markerMatrix);
private:
	bool projectileInt;
};

#endif //PROJECTILE_H