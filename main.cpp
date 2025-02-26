
/*

	Manta Style (Project Flare)
	DotA 2 Model Viewer Engine
	
	Author: KennyZero (@bongikairu)

	why manta? http://www.nerfnow.com/comic/1269

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <emscripten.h>

#include "gl.h"
#include "common.h"
#include "engine/common.hpp"
#include <emscripten/bind.h>

using namespace emscripten;

// Application Context
ESContext* context;

// Time
struct timeval t1, t2;
struct timezone tz;
float deltatime;
float totaltime = 0.0f;
unsigned int frames = 0;

// App data (move to userdata later)
bool axe_data = false;
bool bounty_data = false;
float total = 0;

Model** mx = 0;

int Init ( ESContext *esContext )
{
	
	bool engineSuccess = true;
	
	// Start time reference
	gettimeofday ( &t1 , &tz );
	
	// Scene setting
	Scene::shadowMapCoverage = 450;
	
	// Init
	Manager::Init();
	Scene::InitFeatures();
	Scene::InitDefaultTextures();
	
	// Scene feature setting
	//Scene::enableShadow = false;
	//Scene::enableVAO = false;
	
	// Passable data
	esContext->userData = (char*) malloc(sizeof(UserData));
	UserData *userData = (UserData*) esContext->userData;
	userData->deg = 0;
	
	// Shader Init
	// - For hero
	HeroShader* hShader = new HeroShader();
	engineSuccess &= hShader->Load();
	userData->heroShader = hShader;
	// - For shadowmap
	ShadowShader* sShader = new ShadowShader();
	sShader->Load();
	userData->shadowShader = sShader;
	
	// Canvas transparency
	glClearColor (0, 0, 0, 0.0f );
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	
	// Back Face Culling
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	// Final Render map
	Scene::InitFinalRender();
	
	// Create shadowmap buffer and texture
	Scene::InitShadowmap();
	
	Scene::camXAxis = glm::vec3(1.0f,0.0f,0.05f); //avoid point (0,y,0)
	Scene::camRotationSpeed = glm::vec3(-M_PI/9000,M_PI/900,0.0f);//M_PI/900;
	Scene::camRotationAcc = glm::vec3(0.0f,0.0f,0.0f);
	
	Scene::lightDir = glm::vec3(-1.0,-2.0,-1.0);
	Scene::camPosition = glm::vec3(0.0f,100.0f,300.0f);
	Scene::camTarget = glm::vec3(0.0f,100.0f,0.0f);
	
	return engineSuccess;
}

void Update ( ESContext *esContext, float deltaTime )
{
	Manager::Update( esContext, deltaTime );
	
	// model rotation
	//UserData *userData = (UserData*) esContext->userData;
	//userData->deg += M_PI/90;
	
	//camera rotation deceleration
	if(fabs(Scene::camRotationAcc.y)>fabs(Scene::camRotationSpeed.y)){
		//Scene::camRotationAcc.y = 0.0f;
		Scene::camRotationSpeed.y = 0.0f;
	}else{
		Scene::camRotationSpeed.y=(Scene::camRotationSpeed.y>0 ? 
			Scene::camRotationSpeed.y+=Scene::camRotationAcc.y : 
			Scene::camRotationSpeed.y-=Scene::camRotationAcc.y);
	}
	if(fabs(Scene::camRotationAcc.x)>fabs(Scene::camRotationSpeed.x)){
		//Scene::camRotationAcc.x = 0.0f;
		Scene::camRotationSpeed.x = 0.0f;
	}else{
		Scene::camRotationSpeed.x=(Scene::camRotationSpeed.x>0 ? 
			Scene::camRotationSpeed.x+=Scene::camRotationAcc.x : 
			Scene::camRotationSpeed.x-=Scene::camRotationAcc.x);
	}
	
	//camera rotation
	if(fabs(Scene::camRotationSpeed.y)>0){
		Scene::camPosition = glm::rotateY(Scene::camPosition, Scene::camRotationSpeed.y); //yaw
		Scene::camXAxis = glm::rotateY(Scene::camXAxis, Scene::camRotationSpeed.y);
	}
	if(fabs(Scene::camRotationSpeed.x)>0){
		Scene::camPosition = glm::rotate(Scene::camPosition, Scene::camRotationSpeed.x, Scene::camXAxis ); //pitch
	}
	
	
	
	// data loading
	if(totaltime > 0.1f && !bounty_data)
	{
		bounty_data = true;

		int modelCount = 6;

		const char* modelName[6] = {
			"models/heroes/axe/axe.vmdl_c",
			"models/heroes/axe/axe_armor.vmdl_c",
			"models/heroes/axe/axe_belt.vmdl_c",
			"models/heroes/axe/axe_ponytail.vmdl_c",
			"models/heroes/axe/axe_weapon.vmdl_c",
			//"models/heroes/axe/axe_armor.mdl",
			//"models/items/axe/molten_claw/molten_claw.mdl",
			//"models/heroes/axe/axe_belt.mdl",
			//"models/heroes/axe/axe_ponytail.mdl",
			//"models/heroes/axe/axe_weapon.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_backpack.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_bandana.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_bweapon.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_lweapon.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_rweapon.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_pads.mdl",
			//"models/heroes/bounty_hunter/bounty_hunter_shuriken.mdl",
			//"models/heroes/tidehunter/tidehunter.mdl",
			//"models/heroes/tidehunter/tidehunter_anchor.mdl",
			//"models/heroes/tidehunter/tidehunter_belt.mdl",
			//"models/heroes/tidehunter/tidehunter_bracer.mdl",
			//"models/heroes/tidehunter/tidehunter_fish.mdl"
			//"models/heroes/tidehunter/tidehunter_hook.mdl"
			//"models/heroes/enigma/enigma.mdl"
			"models/heroes/pedestal/pedestal_1_small.vmdl_c"
		};

		//mx = new Model*[modelCount];

		for(int i=0;i<modelCount;i++)
		{
			
			//Model* m = Manager::createModel(modelName[i]);
			//m->shader = userData->heroShader;
			//m->shaderShadow = userData->shadowShader;
			//m->rotation[0] = - M_PI / 2.0f;
		}
		
		//mx[0]->useAnimation = true; // enable animation for model in index 0
		
		//Model* pedes = mx[modelCount-1];
		//pedes->rotation[0] = - M_PI / 2.0f;
		//pedes->position[1] = -10.0f;
		
		
		
	//	Texture* testVtex = new Texture("custom/axe_body_color_psd_63afddb2.vtex_c");
	//	Manager::add(testVtex);
		
	//	Texture* testVtf = new Texture("materials/models/heroes/axe/axe_body_normal.vtf");
	//	Manager::add(testVtf);
	}
	
	if(bounty_data)
	{
		
		//if(mx[0]->material!=0)
		//{
			//Texture* t = Manager::find("custom/axe_body_color_psd_63afddb2.vtex_c");
			//Texture* t2 = Manager::find("materials/models/heroes/axe/axe_body_normal.vtf");
			//printf("%X\n",(unsigned int) t);
			//mx[0]->material->textureDiffuse = t;
			//mx[0]->material->textureNormal = t2;
		//}
		
		// TODO: use scene graph
		Model* m = Manager::headModel;
		while(m!=0)
		{
			//m->rotation[1] = userData->deg;
			m = m->nextModel;
		}
	}
}

void Draw ( ESContext *esContext )
{

	// Reset the viewport
	glViewport(0, 0, floor(Scene::screenWidth), floor(Scene::screenHeight));

	// if set to true, draw shadowmap
	bool showShadowmap = false;
	
	if(!showShadowmap)
	{
		// Normal draw mode
		// Generate shadowmap
		Scene::currentStep = RS_SHADOW;
		glBindFramebuffer(GL_FRAMEBUFFER, Scene::shadowFrameBuffer);
		//glColorMask(false, false, false, false);
		glClearColor (1.0f, 1.0f, 1.0f, 1.0f );
		glClear ( GL_COLOR_BUFFER_BIT );
		glClear ( GL_DEPTH_BUFFER_BIT );
		//glFrontFace(GL_CW);
		
		// TODO: use scene graph
		Model* m = Manager::headModel;
		while(m!=0)
		{
			m->Draw(esContext);
			m = m->nextModel;
		}
		
		glFrontFace(GL_CCW);
		// Draw real scene
		Scene::currentStep = RS_SCENE;
		//glBindFramebuffer(GL_FRAMEBUFFER, Scene::finalRenderFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glColorMask(true, true, true, true);
		glClearColor (0, 0, 0, 0.0f );
		glClear ( GL_COLOR_BUFFER_BIT );
		glClear ( GL_DEPTH_BUFFER_BIT );
		// TODO: use scene graph
		m = Manager::headModel;
		while(m!=0)
		{
			m->Draw(esContext);
			m = m->nextModel;
		}
		/* glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glColorMask(true, true, true, true);
		glClearColor (0, 0, 0, 0.0f );
		glClear ( GL_COLOR_BUFFER_BIT );
		glClear ( GL_DEPTH_BUFFER_BIT );
		Scene::FinalRender(); */
	}
	else
	{
		// Shadow map only mode
		Scene::currentStep = RS_SHADOW;
		//glBindFramebuffer(GL_FRAMEBUFFER, Scene::finalRenderFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glColorMask(true, true, true, true);
		glClearColor (1.0f, 1.0f, 1.0f, 1.0f );
		glClear ( GL_COLOR_BUFFER_BIT );
		glClear ( GL_DEPTH_BUFFER_BIT );
		// TODO: use scene graph
		Model* m = Manager::headModel;
		while(m!=0)
		{
			m->Draw(esContext);
			m = m->nextModel;
		}
		/* glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glColorMask(true, true, true, true);
		glClearColor (0, 0, 0, 0.0f );
		glClear ( GL_COLOR_BUFFER_BIT );
		glClear ( GL_DEPTH_BUFFER_BIT );
		Scene::FinalRender(); */
	}
}

void mainloop()
{
	
	gettimeofday(&t2, &tz);
	deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
	t1 = t2;

	if (context->updateFunc != NULL)
	{
		context->updateFunc(context, deltatime);
	}
	if (context->drawFunc != NULL)
	{
		context->drawFunc(context);
	}

	totaltime += deltatime;
	frames++;
	
	const float fpsRefresh = 5.0f;
	
	if (totaltime >  fpsRefresh)
	{
		//printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames/totaltime);
		totaltime -= fpsRefresh;
		frames = 0;
	}
	
	glfwSwapBuffers(context->m_window);
	glfwPollEvents();
	
}

void errorCallback(int error, const char *description) {
	printf("GLFW Error %d: %s\n",error,description);
}

int main ( int argc, char *argv[] )
{
	
	printf("Starting Flare DotA Model Viewer Engine ..\n");
	
	// don't change it after you set it here
	Scene::screenWidth = 2048.0f;
	Scene::screenHeight = 2048.0f;
	
	context = new ESContext();
	context->userData = new UserData();
	
	glfwSetErrorCallback(errorCallback);
	
	if( !glfwInit() )
	{
		printf("Unable to initialize GLFW (%s,%d)\n",__FILE__,__LINE__);
		return 1;
	}

	context->m_window = glfwCreateWindow(floor(Scene::screenWidth), floor(Scene::screenHeight), "Flare", 0, 0);
	//context->m_window = glfwOpenWindow(floor(Scene::screenWidth), floor(Scene::screenHeight), 0,0,0,0,16,0, GLFW_WINDOW);

	if(!context->m_window )
	{
		glfwTerminate();
		printf("Unable to create GLFW window (%s,%d)\n",__FILE__,__LINE__);
		return 1;
	}

	glfwMakeContextCurrent(context->m_window);

	if ( glewInit() != GLEW_OK )
	{
		printf("Unable to initialize GLeW (%s,%d)\n",__FILE__,__LINE__);
		return 1;
	}

	// HID Callback
	//glfwSetKeyCallback(m_window,key_callback);
	//glfwSetCursorPosCallback(m_window, cursor_pos_callback);
	//glfwSetFramebufferSizeCallback(m_window,framebuffer_size_callback);

	glViewport(0, 0, floor(Scene::screenWidth), floor(Scene::screenHeight));

	// bind draw and update
	context->updateFunc = Update;
	context->drawFunc = Draw;
	
	bool engineSuccess = Init(context);
	
	if(!engineSuccess) 
	{
		printf("Engine failed to start\n");
		exit(1);
	}
	// set main loop function
	emscripten_set_main_loop(mainloop, -1,0);
	
	return 0;
	
}

// JS Binding

void AddModel(std::string name)
{
	UserData *userData = (UserData*) context->userData;

	const char* cName = name.c_str();

	Model* m = Manager::findModel(cName);
	
	if(m==0)
	{
		Model* m2 = Manager::createModel(cName);
		m2->shader = userData->heroShader;
		m2->shaderShadow = userData->shadowShader;
		m2->rotation[0] = - M_PI / 2.0f;
	}
}

void RemoveModel(std::string name)
{
	Model* m = Manager::findModel(name.c_str());
	if(m!=0)
	{
		Manager::remove(m);
	}
}

int ModelCount()
{
	// TODO: move all of these into manager
	Model* cur = Manager::headModel;
	int  i = 0;
	while(cur!=0)
	{
		i++;
		cur = cur->nextModel;
	}
	return i;
}

std::string GetModel(int index)
{
	// TODO: move all of these into manager
	Model* cur = Manager::headModel;
	for(int i = 0;i<=index && cur!=0;i++)
	{
		if(i == index) return std::string(cur->fileName);
		cur = cur->nextModel;
	}
	return std::string("");
}

void SetPosition(std::string name, float x, float y, float z)
{
	Model* m = Manager::findModel(name.c_str());
	if(m!=0)
	{
		m->position = glm::vec4(x,y,z,1);
	}
}

void SetMatOffset(std::string name, int offset)
{
	Model* m = Manager::findModel(name.c_str());
	if(m!=0)
	{
		m->matOffset = offset;
	}
}

void setCamPosition(float x, float y, float z)
{
	Scene::camPosition = glm::vec3(x,y,z);
	Scene::camXAxis = glm::triangleNormal(Scene::camPosition, Scene::camTarget, 
		glm::vec3(Scene::camTarget.x,Scene::camTarget.y+1000.0f,Scene::camTarget.z));
	Scene::camXAxis = glm::rotateY(Scene::camXAxis, 0.06f);
}
void setCamTarget(float x, float y, float z)
{
	Scene::camTarget = glm::vec3(x,y,z);
}
void setcamRotationSpeed(float x, float y, float z)
{
	Scene::camRotationSpeed = glm::vec3(x,y,z);
}
void setcamRotationAcc(float x, float y, float z)
{
	Scene::camRotationAcc = glm::vec3(x,y,z);
}
void setcamZoom(float zoom)
{
	//camera dolly
	Scene::camPosition += (Scene::camPosition-Scene::camTarget)/glm::l2Norm(Scene::camPosition-Scene::camTarget)*zoom;
}

void setViewPort(float w,float h)
{
	Scene::screenWidth = w;
	Scene::screenHeight = h;
	glViewport(0, 0, floor(Scene::screenWidth), floor(Scene::screenHeight));
	Scene::InitShadowmap();
}

EMSCRIPTEN_BINDINGS(model_control)
{
	function("MC_AddModel",&AddModel);
	function("MC_RemoveModel",&RemoveModel);
	function("MC_ModelCount",&ModelCount);
	function("MC_GetModel",&GetModel);
	function("MC_SetPosition",&SetPosition);
	function("MC_SetMatOffset",&SetMatOffset);
	
	function("SC_SetCamPosition",&setCamPosition);
	function("SC_SetCamTarget",&setCamTarget);
	function("SC_SetCamRotationSpeed",&setcamRotationSpeed);
	function("SC_SetCamRotationAcc",&setcamRotationAcc);
	function("SC_SetCamZoom",&setcamZoom);
	
	function("GL_SetViewPort",&setViewPort);
}