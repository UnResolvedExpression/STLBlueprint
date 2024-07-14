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
//#include <shellapi.h>
#include <windows.h>
#include <commdlg.h>
struct GameData
{
	glm::vec2 rectPos = {100,100};

}gameData;



gl2d::Renderer2D renderer;
std::list<STLParser::Triangle> triangleList; //if renderer is also a global variable, this is way this repo shares objects between initGame and gameLogic
STLParser::stlLimits limits{};
glm::vec4 zoomCords = { 0,0,0,0 };
bool zoom{ false };
bool sceneChange{ true };
bool viewZoomPoints{ true };
glm::vec2 windSize = { 0,0 };// it would be better to callback, though that is in glfwMain.cpp and I only want to modify this file
std::string stlPath = "D:/STLBlueprint/cmakeSetup/resources/Antliontext-2024-06-21_12-20-09-PM.stl";


std::string pickSTL(){

	// Initialize OPENFILENAME
	//This was modified from gpt 4o
	//	I got lost in some win7 documentation until I tried asking the AI...
	//I learned that the test program I made was unicode, but this project is ASNI
	//I chose to keep it ASNI and adjust acordingly i.e. wchar_t -> char ; L"sdfa" -> "asdfa"
	OPENFILENAME ofn;
	char szFile[260] = { 0 };       // Buffer for the file name

	// Initialize the structure with zeros
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
	ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box
	if (GetOpenFileName(&ofn) == TRUE) {
		std::wcout << L"Selected file: " << ofn.lpstrFile << std::endl;
		return ofn.lpstrFile;
	}
	else {
		std::wcout << L"File selection canceled." << std::endl;
		return "";
	}

}

void readSTL() {
	triangleList= std::list<STLParser::Triangle>(); //we get rid of the previously loaded stl
	STLParser::resetLimits(limits);
	//we will save edges to an array incase we want to get rid of some later
	std::cout << "init" << std::endl;

	//std::string testPath= "D:/STLBlueprint/cmakeSetup/src/gameLayer/test.stl";
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
}


bool initGame()
{
	//initializing stuff for the renderer
	gl2d::init();
	renderer.create();

	//loading the saved data. Loading an entire structure like this makes savind game data very easy.
	platform::readEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));


 #pragma region RenderSTL

	readSTL();
	
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

	if (windSize.x != w || windSize.y != h) {
		sceneChange = true;
		windSize.x = w;
		windSize.y = h;
	}

	if (sceneChange) {
		
		glClear(GL_COLOR_BUFFER_BIT); //clear screen
		renderer.updateWindowMetrics(w, h);
		double targetWidth{ w / (limits.maxZ - limits.minZ) };
		double targetHigth{ h / (limits.maxY - limits.minY) };
		double DRAWINGSCALER{ min(targetWidth,targetHigth) };//these used to be constexpr
		double LINEWIDTH{ 0.4 };
		double YBUFFER{ -limits.minY * DRAWINGSCALER };
		double ZBUFFER{ -limits.minZ * DRAWINGSCALER };
		std::cout << "sceneChange" << std::endl;

		std::cout << "YBUFFER " << YBUFFER << std::endl;
		std::cout << "ZBUFFER " << ZBUFFER << std::endl;
		std::cout << std::boolalpha << zoom << std::endl;
		if (zoom) {
			std::cout << "zoominside" << std::endl;
			//std::cout << "zoomRemaped " << zoomCords.x << " " << zoomCords.y << " " << zoomRemapped.z << " " << zoomRemapped.w << " " << std::endl;

			/*glm::vec4 zoomRemapped = {zoomCords.x / DRAWINGSCALER + limits.minZ, zoomCords.y / DRAWINGSCALER - limits.minY - h / DRAWINGSCALER,
				zoomCords.z / DRAWINGSCALER + limits.minZ, zoomCords.w / DRAWINGSCALER - limits.minY - h/ DRAWINGSCALER };
			*/
			glm::vec4 zoomRemapped = { (zoomCords.x -ZBUFFER)/DRAWINGSCALER, (zoomCords.y -h + YBUFFER) / DRAWINGSCALER,
				(zoomCords.z -ZBUFFER)/DRAWINGSCALER, (zoomCords.w -h +YBUFFER) / DRAWINGSCALER};
			
			/*we draw onto the screen by zbuffer+ point * Scale
			* and h - ybuffer - point *scale
			* the mapping back seems correct

			*/
			std::cout << "zoomRemaped " << zoomRemapped.x << " " << zoomRemapped.y << " " << zoomRemapped.z << " " << zoomRemapped.w << " " << std::endl;

			targetWidth = w / (zoomRemapped.x - zoomRemapped.z);
			targetHigth = h / (zoomRemapped.y - zoomRemapped.w);
			DRAWINGSCALER = min(abs(targetWidth),abs(targetHigth));
			ZBUFFER = -zoomRemapped.x * DRAWINGSCALER;
			YBUFFER = zoomRemapped.w * DRAWINGSCALER;
			std::cout << "targetwidth " << targetWidth << std::endl;
			std::cout << "targethight " << targetHigth << std::endl;
			std::cout << "YBUFFER " << YBUFFER << std::endl;
			std::cout << "ZBUFFER " << ZBUFFER << std::endl;

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
		renderer.updateWindowMetrics(w, h);
	}
	else {
		renderer.updateWindowMetrics(w, h);
	}
	sceneChange = false;



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
		if(viewZoomPoints)
			renderer.renderRectangle({zoomCords.x,zoomCords.y, 10, 10 }, Colors_Red);
	}
	if (platform::isRMousePressed()) {//left mouse can be for first point, etc. Then user does not have to worry about mistakes
		glm::ivec2 tempvec = platform::getRelMousePosition();
		zoomCords.z = tempvec.x;
		zoomCords.w = tempvec.y;//uses xyzw
		std::cout << zoomCords.z << " " << zoomCords.w << std::endl;
		if(viewZoomPoints)
			renderer.renderRectangle({ zoomCords.z,zoomCords.w, 10, 10 }, Colors_Green);
	}
	if (platform::isButtonTyped(platform::Button::Enter))
	{
		std::cout << "zoom" << std::endl;
		zoom=!zoom;
		sceneChange = true;
		if (zoom) {
			viewZoomPoints = false;
		}
		else {
			viewZoomPoints = true;
		}
		
	}
	if (platform::isButtonTyped(platform::Button::Space))
	{ //to banish the zoom points
		std::cout << "viewzoompoints" << std::endl;
		viewZoomPoints = !viewZoomPoints;
		sceneChange = true;
	}
	if (platform::isButtonTyped(platform::Button::O)) {
		//ShellExecuteA(NULL, "open", "C:\\",NULL, NULL, SW_SHOWDEFAULT );
		std::string tmp = pickSTL();
		if (!tmp.empty()) {
			stlPath = tmp;
			readSTL();
			sceneChange = true;
			viewZoomPoints = false;
			zoom = false;
		}
	}

	//gameData.rectPos = glm::clamp(gameData.rectPos, glm::vec2{ 0,0 }, glm::vec2{ w - 100,h - 100 });
	//renderer.renderRectangle({ gameData.rectPos, 100, 100 }, Colors_Blue);

	//renderer.renderRectangle({ 100, 100, 1, 100 }, Colors_White);

	
	

	renderer.flush();


	//ImGui::ShowDemoWindow();
	//ImGui::Begin("Test Imgui");

	//ImGui::DragFloat2("Positions", &gameData.rectPos[0]);

	//ImGui::End();

	return true;
#pragma endregion

}

//This function might not be be called if the program is forced closed
void closeGame()
{

	//saved the data.
	platform::writeEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));

}
