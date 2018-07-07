#include "ar_app.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/texture.h>
#include <graphics/mesh.h>
#include <graphics/primitive.h>
#include <assets/png_loader.h>
#include <graphics/image_data.h>
#include <graphics/font.h>
#include <input/touch_input_manager.h>
#include <maths/vector2.h>
#include <input/input_manager.h>
#include <maths/math_utils.h>
#include <graphics/renderer_3d.h>
#include <graphics/render_target.h>
#include <sony_sample_framework.h>
#include <sony_tracking.h>
#include <input/sony_controller_input_manager.h>
#include <assets\obj_loader.h>


ARApp::ARApp(gef::Platform& platform) :
	Application(platform),
	input_manager_(NULL),
	sprite_renderer_(NULL),
	font_(NULL),
	renderer_3d_(NULL),
	testObject(NULL),
	markerInformationContainer(NULL),
	projectileObject(NULL)
{
}

void ARApp::Init()
{
	input_manager_ = platform_.CreateInputManager();
	sprite_renderer_ = platform_.CreateSpriteRenderer();
	renderer_3d_ = platform_.CreateRenderer3D();

	InitFont();
	cameraTexture = new gef::TextureVita();

	// initialise sony framework
	sampleInitialize();
	smartInitialize();

	// reset marker tracking
	AppData* dat = sampleUpdateBegin();
	smartTrackingReset();
	sampleUpdateEnd(dat);

	//calculate the scare factor for the screen
	verticleScaleFactor = ((float)platform_.width() / (float)platform_.height()) / ((float)SCE_SMART_IMAGE_WIDTH / (float)SCE_SMART_IMAGE_HEIGHT);

	//calculate the required matrices for AR Transformations
	CreateOrthographicMatrix();
	CreateProjectionMatrix();

	viewMatrix.SetIdentity();
	renderer_3d_->set_view_matrix(viewMatrix);
	renderer_3d_->set_projection_matrix(projectionMatrix3D);

	cameraFeed.set_position(gef::Vector4(0.0f, 0.0f, 1.0f));
	cameraFeed.set_height(2.0f * verticleScaleFactor);
	cameraFeed.set_width(2.0f);

	cameraFeed.set_texture(cameraTexture);

	//Create Meshes Here
	mesh_ = CreateCubeMesh();

	//Model Loader
	gef::OBJLoader objLoader;
	bool loaded = objLoader.Load("tank.obj", platform_, tankModel);


	//Info container for the markers
	markerInformationContainer = new MarkerInformation;

	//Temp Identity Matrix
	gef::Matrix44 tempMatrix;
	tempMatrix.SetIdentity();

	//Set the values within the vector for marker positions to identity
	for (int i = 0; i < ARRAY_SIZE; i++)
	{
		markerInformationContainer->SetMarkerMatrix(tempMatrix, i);
	}

	//start the setup state
	currentState = Setup;

	init = false;
	cleanUp = false;
}

void ARApp::GameLogicInit()
{
	//Initilize game objects here
	testObject = new WaypointObject(tankModel.mesh(), 0.0f, 0.0f, -0.2f, 0);
	testObject->SetScale(gef::Vector4(0.5f, 0.5f, 0.5f));
	projectileObject = new Projectile(mesh_, 0.0f, 0.0f, -0.2f, 0);

	//initialize variables
	collisionState = false;
	allMarkersVisible = true;
}

void ARApp::CleanUp()
{
	smartRelease();
	sampleRelease();

	CleanUpFont();
	delete sprite_renderer_;
	sprite_renderer_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;

	delete input_manager_;
	input_manager_ = NULL;

	delete markerInformationContainer;
	markerInformationContainer = NULL;

	GameLogicCleanUp();
}

void ARApp::GameLogicCleanUp()
{
	delete projectileObject;
	projectileObject = NULL;

	delete testObject;
	testObject = NULL;
}

bool ARApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;

	//Get input from the controller
	input_manager_->Update();
	gef::SonyControllerInputManager* controllerInput = input_manager_->controller_input();
	const gef::SonyController* controller = controllerInput->GetController(0);

	switch (currentState)
	{
	case Setup:

		if (!init)
		{
			//Run the init for game objects
			GameLogicInit();
			init = true;
		}
		if (cleanUp)
		{
			cleanUp = false;
		}

		for (int markerID = 0; markerID < ARRAY_SIZE; markerID++)
		{
			if (!markerInformationContainer->GetMarkerVisible(markerID))
			{
				allMarkersVisible = false;
				break;
			}
			else
			{
				allMarkersVisible = true;
			}
		}

		if (allMarkersVisible)
		{
			if (controller->buttons_pressed() & gef_SONY_CTRL_CIRCLE)
			{
				currentState = Play;
				break;
			}
		}
			
		break;

	case Play:
		//Shooting
		if (controller->buttons_pressed() & gef_SONY_CTRL_SQUARE)
		{
			projectileList.push_front(*projectileObject);
		}

		//Updates projectiles and checks for collisions
		for (std::list<Projectile>::iterator it = projectileList.begin(); it != projectileList.end();)
		{
			it->Update(markerInformationContainer);

			if (it->AaBbCollision(testObject))
			{
				testObject->SetObjectHP(testObject->GetObjectHP() - 1);
				//removes the projectile if it has collided
				collisionState = true;
				it = projectileList.erase(it);
			}
			else
			{
				//distance to remove the projectile if it has not hit anything
				collisionState = false;
				if (it->transform().GetTranslation().Length() > 1.0f) //CHANGE THIS TO DISTANCE
				{
					it = projectileList.erase(it);
				}
				else
				{
					it++;
				}
			}
		}

		//check if the game object is dead
		if (testObject->GetObjectHP() <= 0)
		{
			currentState = End;
		}

		//sends updated marker info to be updated
		testObject->UpdateWaypoints(markerInformationContainer);
		testObject->FollowWaypoints();
		break;

	case End:

		if (!cleanUp)
		{
			//Clean up all the game logic
			GameLogicCleanUp();
			cleanUp = true;
		}

		//Press to Restart
		if (controller->buttons_pressed() & gef_SONY_CTRL_CIRCLE)
		{
			currentState = Setup;
		}

		if (init)
		{
			init = false;
		}

		break;

	}
	//controller input

	AppData* dat = sampleUpdateBegin();

	//look for a marker and set something to transform it to
	GetMarkerPositions(6);

	//calculate local positions between markers
	CalculateMarkerLocals();

	// use the tracking library to try and find markers
	smartUpdate(dat->currentImage);

	sampleUpdateEnd(dat);

	return true;
}

void ARApp::Render()
{
	AppData* dat = sampleRenderBegin();

	//
	// Render the camera feed
	//

	// REMEMBER AND SET THE PROJECTION MATRIX HERE
	sprite_renderer_->set_projection_matrix(orthographicProjection);
	sprite_renderer_->Begin(true);

	// check there is data present for the camera image before trying to draw it
	if (dat->currentImage)
	{
		cameraTexture->set_texture(dat->currentImage->tex_yuv);
		sprite_renderer_->DrawSprite(cameraFeed);
	}

	// DRAW CAMERA FEED SPRITE HERE

	sprite_renderer_->End();


	//
	// Render 3D scene
	//

	// SET VIEW AND PROJECTION MATRIX HERE

	// Begin rendering 3D meshes, don't clear the frame buffer
	renderer_3d_->Begin(false);

	// DRAW 3D MESHES HERE

	switch (currentState)
	{
	case Setup:
		break;

	case Play:
		renderer_3d_->DrawMesh(*testObject);

		for (std::list<Projectile>::iterator it = projectileList.begin(); it != projectileList.end();)
		{
			renderer_3d_->DrawMesh(*it);
			it++;
		}

		break;

	case End:
		break;
	}

	
	//renderer_3d_->DrawMesh(*projectileObject);

	renderer_3d_->End();

	RenderOverlay();

	sampleRenderEnd();
}


void ARApp::RenderOverlay()
{
	//
	// render 2d hud on top
	//
	gef::Matrix44 proj_matrix2d;

	proj_matrix2d = platform_.OrthographicFrustum(0.0f, platform_.width(), 0.0f, platform_.height(), -1.0f, 1.0f);
	sprite_renderer_->set_projection_matrix(proj_matrix2d);
	sprite_renderer_->Begin(false);
	DrawHUD();
	sprite_renderer_->End();
}


void ARApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("comic_sans");
}

void ARApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

void ARApp::DrawHUD()
{
	if(font_)
	{
		switch (currentState)
		{
		case Setup:
			//search through the marker found array and display if the Vita can currently see them 
			for (int markerID = 0; markerID < 6; markerID++)
			{
				if (markerInformationContainer->GetMarkerVisible(markerID))
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(150.0f, 300.0f + (20.0f * (float)markerID), -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Marker %d Detected", markerID + 1);
				}
				else
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(150.0f, 300.0f + (20.0f * (float)markerID), -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Marker %d Not Found", markerID + 1);
				}
			}

			if (allMarkersVisible)
			{
				font_->RenderText(sprite_renderer_, gef::Vector4(150.0f, 200.0f, -0.9f), 3.0f, 0xff0000ff, gef::TJ_LEFT, "Press Circle to Start");
			}

			break;

		case Play:
			//gets the current target waypoint
			font_->RenderText(sprite_renderer_, gef::Vector4(150.0f, 280.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Target HP : %d", testObject->GetObjectHP());
			break;

		case End:
			//Mission Complete!
			font_->RenderText(sprite_renderer_, gef::Vector4(150.0f, 240.0f, -0.9f), 3.0f, 0xff0000ff, gef::TJ_LEFT, "Mission Complete!");
			font_->RenderText(sprite_renderer_, gef::Vector4(150.0f, 340.0f, -0.9f), 2.0f, 0xff0000ff, gef::TJ_LEFT, "Press O to restart");
			break;
		}
	}
}

void ARApp::CreateOrthographicMatrix()
{
	orthographicProjection.OrthographicFrustumGL(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
}

void ARApp::CreateProjectionMatrix()
{
	fovProjectionMatrix.PerspectiveFovGL((float)SCE_SMART_IMAGE_FOV, ((float)SCE_SMART_IMAGE_WIDTH / (float)SCE_SMART_IMAGE_HEIGHT), 0.05f, 5.0f);
	scalarMatrix.SetIdentity();
	scalarMatrix.SetRow(1, gef::Vector4(0, verticleScaleFactor, 0));
	projectionMatrix3D = fovProjectionMatrix * scalarMatrix;
}

gef::Mesh* ARApp::CreateCubeMesh()
{
	gef::Mesh* mesh = platform_.CreateMesh();

	// initialise the vertex data to create a 1, 1, 1 cube
	const float half_size = 0.02f;
	const gef::Mesh::Vertex vertices[] = {
		{ half_size, -half_size, -half_size,  0.577f, -0.577f, -0.577f, 0.0f, 0.0f },
		{ half_size,  half_size, -half_size,  0.577f,  0.577f, -0.577f, 0.0f, 0.0f },
		{ -half_size,  half_size, -half_size, -0.577f,  0.577f, -0.577f, 0.0f, 0.0f },
		{ -half_size, -half_size, -half_size, -0.577f, -0.577f, -0.577f, 0.0f, 0.0f },
		{ half_size, -half_size,  half_size,  0.577f, -0.577f,  0.577f, 0.0f, 0.0f },
		{ half_size,  half_size,  half_size,  0.577f,  0.577f,  0.577f, 0.0f, 0.0f },
		{ -half_size,  half_size,  half_size, -0.577f,  0.577f,  0.577f, 0.0f, 0.0f },
		{ -half_size, -half_size,  half_size, -0.577f, -0.577f,  0.577f, 0.0f, 0.0f }
	};

	mesh->InitVertexBuffer(platform_, static_cast<const void*>(vertices), sizeof(vertices) / sizeof(gef::Mesh::Vertex), sizeof(gef::Mesh::Vertex));

	// we will create a single triangle list primitive to draw the triangles that make up the cube
	mesh->AllocatePrimitives(1);
	gef::Primitive* primitive = mesh->GetPrimitive(0);

	const UInt32 indices[] = {
		// Back
		0, 1, 2,
		2, 3, 0,
		// Front
		6, 5, 4,
		4, 7, 6,
		// Left
		2, 7, 3,
		2, 6, 7,
		// Right
		0, 4, 1,
		5, 1, 4,
		// Top
		6, 2, 1,
		5, 6, 1,
		// Bottom
		0, 3, 7,
		0, 7, 4
	};

	primitive->InitIndexBuffer(platform_, static_cast<const void*>(indices), sizeof(indices) / sizeof(UInt32), sizeof(UInt32));
	primitive->set_type(gef::TRIANGLE_LIST);

	// set size of bounds, we need this for collision detection routines
	gef::Aabb aabb(gef::Vector4(-half_size, -half_size, -half_size), gef::Vector4(half_size, half_size, half_size));
	gef::Sphere sphere(aabb);

	mesh->set_aabb(aabb);
	mesh->set_bounding_sphere(sphere);

	return mesh;
}

void ARApp::GetMarkerPositions(int markerInput)
{
	int markersInUse = markerInput - 1;
	//Check if the marker count is in bounds
	if(markersInUse <= ARRAY_SIZE && markersInUse > -1)
	{
	// check to see if a particular marker can be found
		for (int markerNum = 0; markerNum <= markersInUse; markerNum++)
		{
			if (sampleIsMarkerFound(markerNum))
			{
				// marker is being tracked, get it’s transform
				gef::Matrix44 marker_transform;
				sampleGetTransform(markerNum, &marker_transform);

				// set the transform of the 3D mesh instance to draw on
				markerInformationContainer->SetMarkerMatrix(marker_transform, markerNum);
				markerInformationContainer->SetMarkerVisible(markerNum, true);
			}
			else
			{
				//This marker cannot be seen
				markerInformationContainer->SetMarkerVisible(markerNum, false);
			}
		}
	}

	else
	{
		return;
	}
}

void ARApp::CalculateMarkerLocals()
{
	for (int parentMarkerID = 0; parentMarkerID < ARRAY_SIZE; parentMarkerID++)
	{
		for (int targetMarkerID = 0; targetMarkerID < ARRAY_SIZE; targetMarkerID++)
		{
			if (markerInformationContainer->GetMarkerVisible(targetMarkerID) && markerInformationContainer->GetMarkerVisible(parentMarkerID))
			{
				//inverts the matrix for the marker
				gef::Matrix44 inverse_markerMatrix = markerInformationContainer->GetMarkerMatrix(parentMarkerID);
				inverse_markerMatrix.AffineInverse(markerInformationContainer->GetMarkerMatrix(parentMarkerID));

				//calculate the local co-ordinate of the target marker
				gef::Matrix44 localTargetTransform = markerInformationContainer->GetMarkerMatrix(targetMarkerID) * inverse_markerMatrix;

				markerInformationContainer->SetMarkerRelativePosition(parentMarkerID, targetMarkerID, localTargetTransform);
			}
		}
		
	}
}