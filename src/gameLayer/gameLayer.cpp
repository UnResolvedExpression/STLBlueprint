#define GLM_ENABLE_EXPERIMENTAL
#include "gameLayer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include "imfilebrowser.h"
#include <gl2d/gl2d.h>
#include <platformTools.h>

#include <fstream>
#include <string>

#include "STLParser/STLParser.h"

struct GameData
{
	glm::vec2 rectPos = {100,100};

}gameData;



gl2d::Renderer2D renderer;
std::list<STLParser::Triangle> triangleList; //if renderer is also a global variable, this is way this repo shares objects between initGame and gameLogic
STLParser::stlLimits limits{};
glm::vec4 zoomCords = { 0,0,0,0 };
bool zoom{ false };

bool initGame()
{
	//initializing stuff for the renderer
	gl2d::init();
	renderer.create();

	//loading the saved data. Loading an entire structure like this makes savind game data very easy.
	platform::readEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));


 #pragma region RenderSTL

	//we will save edges to an array incase we want to get rid of some later
	std::cout << "init" << std::endl;
	std::string stlPath = "D:/STLBlueprint/cmakeSetup/resources/Tameem class Frigatetext-2024-04-23_04-09-21-PM.stl";
	std::string testPath= "D:/STLBlueprint/cmakeSetup/src/gameLayer/test.stl";
	std::ifstream fin;
	fin.open(stlPath);
	if (!fin)
		std::cout << "ifstream fail" << std::endl;

	std::string delimiter = " ";
	size_t pos = 0;
	std::string token{};

	//std::cout << "funtimes"<<std::endl;
	while (fin) {
		std::string line{ "" };
		std::getline(fin, line);

		pos = line.find(delimiter);
		token = line.substr(0, pos);
		//std::cout << token << std::endl;
		//line.erase(0, pos + delimiter.length());

		//normal
		if (line.substr(0, pos) == "facet") {
			std::array <double, 3> initArr;
			line.erase(0, pos + delimiter.length());
			pos = line.find(delimiter);
			line.erase(0, pos + delimiter.length());

			int counter{ 0 };
			while ((pos = line.find(delimiter)) != std::string::npos) {
				token = line.substr(0, pos);
				initArr[counter] = stod(token);
				counter++;
				//std::cout << token << std::endl;
				line.erase(0, pos + delimiter.length());
			}
			initArr[2] = stod(line);
			STLParser::xyz normal{ initArr[0] , initArr[1] , initArr[2] };

			//p1
			std::getline(fin, line);
			std::getline(fin, line);
			counter=0;
			pos = line.find(delimiter);
			line.erase(0, pos + delimiter.length());
			while ((pos = line.find(delimiter)) != std::string::npos) {
				token = line.substr(0, pos);
				initArr[counter] = stod(token);
				counter++;
				//std::cout << token << std::endl;
				line.erase(0, pos + delimiter.length());
			}
			initArr[2] = stod(line);
			STLParser::xyz p1{ initArr[0] , initArr[1] , initArr[2] };

			//p2
			std::getline(fin, line);
			counter = 0;
			pos = line.find(delimiter);
			line.erase(0, pos + delimiter.length());
			while ((pos = line.find(delimiter)) != std::string::npos) {
				token = line.substr(0, pos);
				initArr[counter] = stod(token);
				counter++;
				//std::cout << token << std::endl;
				line.erase(0, pos + delimiter.length());
			}
			initArr[2] = stod(line);
			STLParser::xyz p2{ initArr[0] , initArr[1] , initArr[2] };

			//p3
			std::getline(fin, line);
			counter = 0;
			pos = line.find(delimiter);
			line.erase(0, pos + delimiter.length());
			while ((pos = line.find(delimiter)) != std::string::npos) {
				token = line.substr(0, pos);
				initArr[counter] = stod(token);
				counter++;
				//;std::cout << token << std::endl;
				line.erase(0, pos + delimiter.length());
			}
			initArr[2] = stod(line);
			STLParser::xyz p3{ initArr[0] , initArr[1] , initArr[2] };

			//triangle
			STLParser::updateLimits(limits, p1);
			STLParser::updateLimits(limits, p2);
			STLParser::updateLimits(limits, p3);
			STLParser::Triangle triangle(normal, p1, p2, p3);
			triangleList.insert(triangleList.begin(), triangle);
			//STLParser::printTriangle(triangleList.front());
		}
		/*
		while ((pos = line.find(delimiter)) != std::string::npos) {
			token = line.substr(0, pos);

		}
		*/
	}
#pragma endregion



	return true;
}


//IMPORTANT NOTICE, IF YOU WANT TO SHIP THE GAME TO ANOTHER PC READ THE README.MD IN THE GITHUB
//https://github.com/meemknight/cmakeSetup
//OR THE INSTRUCTION IN THE CMAKE FILE.
//YOU HAVE TO CHANGE A FLAG IN THE CMAKE SO THAT RESOURCES_PATH POINTS TO RELATIVE PATHS
//BECAUSE OF SOME CMAKE PROGBLMS, RESOURCES_PATH IS SET TO BE ABSOLUTE DURING PRODUCTION FOR MAKING IT EASIER.


void manageTriangles()
{

}

bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w = platform::getFrameBufferSizeX(); //window w
	h = platform::getFrameBufferSizeY(); //window h

	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT); //clear screen


	renderer.updateWindowMetrics(w, h);
	double targetWidth{ w / (limits.maxZ - limits.minZ) };
	double targetHigth{ h / (limits.maxY - limits.minY) };
	double DRAWINGSCALER{ min(targetWidth,targetHigth)};//these used to be constexpr
	double LINEWIDTH{ 0.4 };
	double YBUFFER{ -limits.minY*DRAWINGSCALER };
	double ZBUFFER{ -limits.minZ*DRAWINGSCALER };

	if (zoom) {
		glm::vec4 zoomRemapped = {zoomCords.x/DRAWINGSCALER-ZBUFFER, zoomCords.y / DRAWINGSCALER + YBUFFER - h,
			zoomCords.x / DRAWINGSCALER - ZBUFFER, zoomCords.y / DRAWINGSCALER + YBUFFER - h };

		targetWidth= w / (zoomRemapped.x-zoomRemapped.z) ;
		targetHigth= h / (zoomRemapped.y-zoomCords.w) ;
		DRAWINGSCALER= min(targetWidth,targetHigth) ;
		YBUFFER= -limits.minY * DRAWINGSCALER ;
		ZBUFFER= -limits.minZ * DRAWINGSCALER ;

		for (auto const& triangle : triangleList) {
			renderer.renderLine({ ZBUFFER + triangle.getP1().z * DRAWINGSCALER , h - YBUFFER - triangle.getP1().y * DRAWINGSCALER }, { ZBUFFER + triangle.getP2().z * DRAWINGSCALER, h - YBUFFER - triangle.getP2().y * DRAWINGSCALER }, Colors_White, LINEWIDTH);
			renderer.renderLine({ ZBUFFER + triangle.getP2().z * DRAWINGSCALER , h - YBUFFER - triangle.getP2().y * DRAWINGSCALER }, { ZBUFFER + triangle.getP3().z * DRAWINGSCALER, h - YBUFFER - triangle.getP3().y * DRAWINGSCALER }, Colors_White, LINEWIDTH);
			renderer.renderLine({ ZBUFFER + triangle.getP3().z * DRAWINGSCALER , h - YBUFFER - triangle.getP3().y * DRAWINGSCALER }, { ZBUFFER + triangle.getP1().z * DRAWINGSCALER, h - YBUFFER - triangle.getP1().y * DRAWINGSCALER }, Colors_White, LINEWIDTH);
		}
	}
	else {

		for (auto const& triangle : triangleList) {
			renderer.renderLine({ ZBUFFER + triangle.getP1().z * DRAWINGSCALER , h - YBUFFER - triangle.getP1().y * DRAWINGSCALER }, { ZBUFFER + triangle.getP2().z * DRAWINGSCALER, h - YBUFFER - triangle.getP2().y * DRAWINGSCALER }, Colors_White, LINEWIDTH);
			renderer.renderLine({ ZBUFFER + triangle.getP2().z * DRAWINGSCALER , h - YBUFFER - triangle.getP2().y * DRAWINGSCALER }, { ZBUFFER + triangle.getP3().z * DRAWINGSCALER, h - YBUFFER - triangle.getP3().y * DRAWINGSCALER }, Colors_White, LINEWIDTH);
			renderer.renderLine({ ZBUFFER + triangle.getP3().z * DRAWINGSCALER , h - YBUFFER - triangle.getP3().y * DRAWINGSCALER }, { ZBUFFER + triangle.getP1().z * DRAWINGSCALER, h - YBUFFER - triangle.getP1().y * DRAWINGSCALER }, Colors_White, LINEWIDTH);
		}
	}



#pragma endregion


	if (platform::isButtonHeld(platform::Button::Left))
	{
		gameData.rectPos.x -= deltaTime * 100;
	}
	if (platform::isButtonHeld(platform::Button::Right))
	{
		gameData.rectPos.x += deltaTime * 100;
	}
	if (platform::isButtonHeld(platform::Button::Up))
	{
		gameData.rectPos.y -= deltaTime * 100;
	}
	if (platform::isButtonHeld(platform::Button::Down))
	{
		gameData.rectPos.y += deltaTime * 100;
	}

	if (platform::isLMousePressed()) {//left mouse can be for first point, etc. Then user does not have to worry about mistakes
		glm::ivec2 tempvec = platform::getRelMousePosition();
		zoomCords.x = tempvec.x;
		zoomCords.y = tempvec.y;//uses xyzw
		std::cout << zoomCords.x << " " << zoomCords.y << std::endl;
	}
	if (platform::isRMousePressed()) {//left mouse can be for first point, etc. Then user does not have to worry about mistakes
		glm::ivec2 tempvec = platform::getRelMousePosition();
		zoomCords.z = tempvec.x;
		zoomCords.w = tempvec.y;//uses xyzw
		std::cout << zoomCords.z << " " << zoomCords.w << std::endl;

	}
	if (platform::isButtonTyped(platform::Button::Enter))
	{
		std::cout << "zoom" << std::endl;
		zoom!=zoom;
	}

	gameData.rectPos = glm::clamp(gameData.rectPos, glm::vec2{ 0,0 }, glm::vec2{ w - 100,h - 100 });
	renderer.renderRectangle({ gameData.rectPos, 100, 100 }, Colors_Blue);

	//renderer.renderRectangle({ 100, 100, 1, 100 }, Colors_White);

	

	renderer.flush();


	//ImGui::ShowDemoWindow();
	ImGui::Begin("Test Imgui");

	ImGui::DragFloat2("Positions", &gameData.rectPos[0]);

	ImGui::End();

	return true;
#pragma endregion

}

//This function might not be be called if the program is forced closed
void closeGame()
{

	//saved the data.
	platform::writeEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));

}
