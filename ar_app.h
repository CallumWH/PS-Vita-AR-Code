#ifndef _RENDER_TARGET_APP_H
#define _RENDER_TARGET_APP_H

#include <system/application.h>
#include <graphics/sprite.h>
#include <maths/vector2.h>
#include <vector>
#include <graphics/mesh_instance.h>
#include <platform/vita/graphics/texture_vita.h>
#include <list>

// Vita AR includes
#include <camera.h>
#include <gxm.h>
#include <motion.h>
#include <libdbg.h>
#include <libsmart.h>

// My own classes
#include "../ar_app/build/vs2015/WaypointObject.h"
#include "../ar_app/build/vs2015/MarkerInformation.h"
#include "../ar_app/build/vs2015/Projectile.h"

// FRAMEWORK FORWARD DECLARATIONS
namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class Renderer3D;
	class Mesh;
	class RenderTarget;
	class TextureVita;
	class InputManager;
}

#define ARRAY_SIZE		6

//I am so sorry
enum GameState { Setup, Play, End };

class ARApp : public gef::Application
{
public:
	ARApp(gef::Platform& platform);
	void Init();
	void GameLogicInit();
	void CleanUp();
	void GameLogicCleanUp();
	bool Update(float frame_time);
	void Render();
private:
	void InitFont();
	void CleanUpFont();
	void DrawHUD();
	void RenderCameraFeed(struct AppData* dat);
	void Render3DScene();
	void RenderOverlay();
	void CreateOrthographicMatrix();
	void CreateProjectionMatrix();
	void GetMarkerPositions(int markersInUse);
	void CalculateMarkerLocals();

	GameState currentState;

	gef::Mesh* CreateCubeMesh();
	
	class gef::Mesh* mesh_;
	gef::InputManager* input_manager_;
	gef::Matrix44 orthographicProjection;
	gef::Matrix44 projectionMatrix3D;
	gef::Matrix44 fovProjectionMatrix;
	gef::Matrix44 scalarMatrix;
	gef::Matrix44 viewMatrix;

	gef::Model tankModel;

	WaypointObject* testObject;
	Projectile* projectileObject;

	std::list<Projectile> projectileList;

	MarkerInformation* markerInformationContainer;

	gef::SpriteRenderer* sprite_renderer_;
	gef::Font* font_;

	gef::Sprite cameraFeed;
	gef::TextureVita *cameraTexture;

	float fps_;
	float verticleScaleFactor;

	bool collisionState;
	bool allMarkersVisible;
	bool init;
	bool cleanUp;

	class gef::Renderer3D* renderer_3d_;
};




#endif // _RENDER_TARGET_APP_H