#include "Projectile.h"

Projectile::Projectile(gef::Mesh* mesh, float x, float y, float z, int markerID) : GameObject(mesh, x, y, z, markerID)
{
	projectileInt = false;
}

void Projectile::Update(MarkerInformation* markerContainer)
{
	//Check if the parent is visible
	if (!markerContainer->GetMarkerVisible(GetParentID()))
	{
		//if the parent is not visible, look for a marker that is
		for (int i = 0; i < ARRAY_SIZE; i++)
		{
			if (markerContainer->GetMarkerVisible(i))
			{
				//Takes the position of the selected marker and sets it to be the local co-ordinates
				SetLocalMatrix(GetMarkerLocalPosition(markerContainer->GetMarkerMatrix(i)));

				//set the matrix of the marker as the new parent
				SetParentMatrix(markerContainer->GetMarkerMatrix(i));
				SetParentID(i);
				projectileInt = true;
				break;
			}
		}
	}
	else
	{
		if (!projectileInt)
		{
			SetLocalMatrix(GetMarkerLocalPosition(markerContainer->GetMarkerMatrix(GetParentID())));
			projectileInt = true;
		}

		//update the parent location
		SetParentMatrix(markerContainer->GetMarkerMatrix(GetParentID()));

		//temorary matrix to store movement co-ordinates
		gef::Matrix44 moveMatrix;
		moveMatrix.SetIdentity();
		
		//Define the direction the projectile will move in
		moveMatrix.SetTranslation(gef::Vector4(0.0f, 0.0f, -0.005f));

		//Uses the objects local position to the marker and multiplies it by where we want to move
		SetLocalMatrix(moveMatrix * GetMarkerLocalPosition(parentMarkerMatrix));
	}

}

gef::Matrix44 Projectile::GetMarkerLocalPosition(gef::Matrix44 markerMatrix)
{
	//inverts the matrix for the marker
	gef::Matrix44 inverse_objectMatrix = markerMatrix;
	inverse_objectMatrix.AffineInverse(markerMatrix);

	//calculate the local co-ordinate of the target marker
	gef::Matrix44 localTargetTransform = this->transform_ * inverse_objectMatrix;
	return localTargetTransform;
}