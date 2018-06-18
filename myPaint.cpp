#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include<gl\gl3w.h>
#include <GLFW/glfw3.h>
#include<iostream>
#include<gl/GLU.h>
#include<glm/glm.hpp>
#include<glm\gtc\matrix_transform.hpp>
#include<string.h>

//照相机类Camera
class Camera {
private:

public:

	GLfloat pfov, pratio, pnear, pfar,yaw, pitch;
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	glm::vec3 cameraRight;


	Camera()
	{
		resetCamera();
	}
	void moveForward(GLfloat const distance)
	{
		cameraPos += distance * cameraFront;
	}
	void moveBack(GLfloat const distance)
	{
		cameraPos -= distance * cameraFront;
	}
	void moveRight(GLfloat const distance)
	{
		cameraPos += distance * cameraRight ;
	}
	void moveLeft(GLfloat const distance)
	{
		cameraPos -= distance * cameraRight;
	}
	void rotate()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);
		cameraRight = glm::cross(cam.cameraFront, cameraUp);
	}
	void resetCamera()
	{
		cameraPos = glm::vec3(3, 3, 3); // Camera is at (3,3,3), in World Space
		cameraFront = glm::normalize(glm::vec3(-3, -3, -3)); // and looks at the origin
		cameraUp = glm::vec3(0, 1, 0);
		cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
		pitch = glm::degrees(asin(cameraFront.y));
		yaw = glm::degrees(asin(cameraFront.z / cos(glm::radians(cam.pitch))));
		if (cameraFront.x < 0)
			yaw = 180 - yaw;
	}
}cam;

//菜单栏按钮对应的操作
enum PrimitivesCmd
{
	cube = 0001,triangle=0010,rectangle=0100,line = 01000,ball =010000
};
enum TransformCmd
{
	scale = 0001, translate = 0010, rotate = 0100,roll = 01000,swing = 010000
};
enum ShadingCmd
{
	phongShading = 01, gouraudShading = 10
};

//----变量声明-----
//鼠标状态
//判断鼠标是否刚按下
BOOL firstDown = FALSE;
//判断鼠标左键时候按下
BOOL mouseLdown = FALSE;
//用以记录鼠标按下状态时鼠标的上一位置
float lastX, lastY;
//绘制的点数组
float vertices[500] = { 0.0f };
//VBO VAO声明
unsigned int VBO, VAO, EBO;
//创建渲染程序
unsigned int *shaderProgram, shaderProgram_Phong, shaderProgram_Gouraud;
//屏幕宽高
int display_w, display_h;
//变换相关矩阵
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

//三角形颜色（...）
ImVec4 triangleVertColor[3] = {
	ImVec4(1.0f, 0.0f, 0.0f,1.0),
	ImVec4(0.0f, 1.0f, 0.0f,1.0),
	ImVec4(0.0f, 0.0f, 1.0f,1.0)
};
//矩形顶点颜色（...）
ImVec4 rectangleVertColor[4] = {
	ImVec4(1.0,0.0,0.0,1.0),
	ImVec4(1.0,0.0,0.0,1.0),
	ImVec4(1.0,1.0,1.0,1.0),
	ImVec4(1.0,1.0,1.0,1.0)
};





//-----着色器声明--------
/*//顶点着色器vertexShader
const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n" //顶点位置
"layout(location = 1) in vec3 aColor;\n"//顶点颜色
"layout (location = 2) in vec3 aNor;\n"//顶点法向量
"out vec4 vertNor;\n"//向片段着色器输出法向量
"out vec4 vertPos;\n"//计算顶点的世界坐标以计算世界坐标系的照明情况
"out vec4 vertColor;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection*view*model*vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"   vertColor = vec4(aColor,1.0); // 输出顶点颜色\n"
"}\0";*/
//顶点着色器vertexShader
const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n" //顶点位置
"layout(location = 1) in vec3 aColor;\n"//顶点颜色
"layout (location = 2) in vec3 aNor;\n"//顶点法向量
"out vec3 vertPos;\n"//计算顶点的世界坐标以计算世界坐标系的照明情况
"out vec3 vertNor;\n"//向片段着色器输出法向量
"out vec4 vertColor;\n"//向片段着色器输出顶点颜色
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection*view*model*vec4(aPos.x, aPos.y, aPos.z, 1.0f);\n"
"   vertPos = vec3(model*vec4(aPos.x, aPos.y, aPos.z, 1.0f));\n"
"   vertColor = vec4(aColor,1.0);\n"
"   vertNor = mat3(transpose(inverse(model)))*aNor; "
"}\0";

//片段着色器fragmentShader
const char *fragmentShaderSource = "#version 330 core\n"
"in vec3 vertPos;\n"
"in vec3 vertNor;\n"
"in vec4 vertColor;\n"
"out vec4 FragColor;\n"
"uniform float ambientAlbedo;\n"
"uniform float diffuseAlbedo;\n"
"uniform float specularAlbedo;\n"
"uniform int nShiny;\n"
"uniform vec3 lightPos;\n"
"uniform vec3 viewPos;\n"
"uniform vec3 lightColor;\n"
"void main()\n"
"{	\n"
"	vec3 ambient = ambientAlbedo * vec3(1.0,0.0,0.0) ;\n"//计算环境光

"	vec3 norm = normalize(vertNor);\n"//标准化法向量
"	vec3 lightDir = normalize(vertPos - lightPos);\n"//光源入射方向
"	float diffusion = max(dot(norm,-lightDir),0.0);\n"//计算散射系数
"	vec3 diffuse = diffusion*lightColor*diffuseAlbedo;\n"//计算漫反射光

"   vec3 viewDir = normalize(vertPos - viewPos);\n"//观察者角度
"   vec3 reflectDir = reflect(lightDir,norm);\n"//镜面反射光方向
"	float spe = pow(max(dot(-viewDir,reflectDir),0.0),nShiny);\n"
"   vec3 specular = spe*lightColor*specularAlbedo;\n"//镜面反射

"   FragColor = vertColor*(vec4((ambient+diffuse+specular),1.0f));\n"//最终输出物体颜色
"}\n\0";


//顶点着色器vertexShader
const char *vertexShaderSource_Gouraud = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n" //顶点位置
"layout(location = 1) in vec3 aColor;\n"//顶点颜色
"layout (location = 2) in vec3 aNor;\n"//顶点法向量
"out vec4 vertColor;\n"//向片段着色器输出顶点颜色
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"uniform float ambientAlbedo;\n"
"uniform float diffuseAlbedo;\n"
"uniform float specularAlbedo;\n"
"uniform int nShiny;\n"
"uniform vec3 lightPos;\n"
"uniform vec3 viewPos;\n"
"uniform vec3 lightColor;\n"
"void main()\n"
"{\n"
"   gl_Position = projection*view*model*vec4(aPos.x, aPos.y, aPos.z, 1.0f);\n"
"   vec3 vertPos = vec3(model*vec4(aPos.x, aPos.y, aPos.z, 1.0f));\n"
"   vec3 vertNor = mat3(transpose(inverse(model)))*aNor; "

"	vec3 ambient = ambientAlbedo * vec3(1.0,0.0,0.0);\n"//计算环境光

"	vec3 norm = normalize(vertNor);\n"//标准化法向量
"	vec3 lightDir = normalize(vertPos - lightPos);\n"//光源入射方向
"	vec3 diffuse = max(dot(norm,-lightDir),0.0)*lightColor*diffuseAlbedo;\n"//计算漫反射光

"   vec3 viewDir = normalize(vertPos - viewPos);\n"//观察者角度
"   vec3 reflectDir = reflect(lightDir,norm);\n"//镜面反射光方向
"   vec3 specular = pow(max(dot(-viewDir,reflectDir),0.0),nShiny)*lightColor*specularAlbedo;\n"//镜面反射

"   vertColor = vec4(aColor,1.0)*(vec4((ambient+diffuse+specular),1.0f));\n"//输出物体颜色

"}\0";

const char *fragmentShaderSource_Gouraud = "#version 330 core\n"
"in vec4 vertColor;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{	\n"
"   FragColor = vertColor;\n"//最终输出物体颜色
"}\n\0";


static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

//----函数声明-----
//着色器编译函数
void shaderCreate(const char*, const char*, unsigned int *);
//primitive绘制的函数
void drawRec();
void drawTri();
void drawCub();
void drawBLine();
void drawBCir();
void drawLin();
void drawSphere();
//执行transform的函数
void tranTran();
void tranRota();
void tranScal(float,float,float);
void tranRoll();
void tranSwin();
//视角变换相关的函数
void changingView();
void setDefaultPerspective();
//光照相关的函数
void setDefaultLight();
//响应键盘鼠标事件的函数
void processInput(GLFWwindow *, Camera&);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);


int main()
{
	//创建窗口
		glfwSetErrorCallback(error_callback);
		if (!glfwInit())
			return 1;
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		GLFWwindow* window = glfwCreateWindow(800,495, "ELLIE'S PROJECT", NULL, NULL);
		glfwMakeContextCurrent(window);
		glfwSwapInterval(1); // Enable vsync
		gl3wInit();
		ImGui_ImplGlfwGL3_Init(window, true);
		ImVec4 clear_color = ImVec4(0.0f,0.0f,0.1f, 1.00f);

		glfwGetFramebufferSize(window, &display_w, &display_h);
	
		//着色器创建编译绑定
		shaderProgram_Phong = glCreateProgram();
		shaderCreate(vertexShaderSource, fragmentShaderSource, &shaderProgram_Phong);
		shaderProgram_Gouraud = glCreateProgram();
		shaderCreate(vertexShaderSource_Gouraud, fragmentShaderSource_Gouraud, &shaderProgram_Gouraud);
		shaderProgram = &shaderProgram_Phong;
		//VBO VAO
		glGenBuffers(1, &VBO);
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &EBO);

		//VBO VAO绑定
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		//设置顶点位置属性
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9* sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//设置顶点颜色属性
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(1);
		//设置顶点法向量
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(2);
		
		glEnable(GL_DEPTH_TEST);
		
		//解绑
		//glBindBuffer(GL_ARRAY_BUFFER, 0);

		//判断变量
		int primitivecmd = 0000;
		int transformcmd = 0000;
		int shadingcmd = 01;
		BOOL drawCircleBresenhamB = FALSE;
		BOOL drawLineBresenhamB = FALSE;
		BOOL drawCross = FALSE;
		BOOL setOrth = FALSE;
		BOOL setPers = FALSE;
		BOOL setCha = FALSE;
		float left = -3.0, right = 3.0, up=-3.0, bottom = 3.0, near_ = 0.1, far_ = 100,ang = 45.0;
		float ambientAlbedo = 0.5, specularAlbedo = 0.1, diffuseAlbedo = 0.5;
		int nShiny = 32;
		setDefaultPerspective();
		setDefaultLight();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window, cursor_position_callback);		

	while (!glfwWindowShouldClose(window))
	{
		int viewPosLoc = glGetUniformLocation(*shaderProgram, "viewPos");
		glUseProgram(*shaderProgram);
		// pass them to the shaders (3 different ways)
		glUniform3f(viewPosLoc, cam.cameraPos.x,cam.cameraPos.y,cam.cameraPos.z);


		processInput(window, cam);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui_ImplGlfwGL3_NewFrame();

		//transform setting
		if (transformcmd == translate)
			tranTran();
		if (transformcmd == rotate)
			tranRota();
		if (transformcmd == roll)
			tranRoll();
		if (transformcmd == swing)
			tranSwin();
		if (transformcmd == scale)
		{
			float s = int(glfwGetTime())%2 == 0?0.99:1.01;
			tranScal(s,s,s);
		}
		
		//addprimitive setting
		if (primitivecmd & rectangle)
		{
			drawRec();
		}
		if (primitivecmd & triangle)
		{
			drawTri();	
		}
		if (primitivecmd & line)
		{
			drawLin();
		}
		if (primitivecmd & cube)
		{
			drawCub();
		}
		if (primitivecmd & ball)
		{
			drawSphere();
		}
		//Bresenham Algorithm
		if (drawLineBresenhamB)
		{
			drawBLine();
		}
		if (drawCircleBresenhamB)
		{
			drawBCir();
		}
		
		if (drawCross)
		{
			tranScal(0.5, 0.8, 1.8);
			drawCub();
			tranScal(1.8, 0.8, 0.5);
			drawCub();

		}

		if (setOrth)
		{
			
			ImGui::Begin((const char*)"Set Orthonal", (bool*)&setOrth);
			ImGui::SliderFloat("left", &left, -50.0f, 50.0f);
			ImGui::SliderFloat("right", &right, -50.0f, 50.0f);
			ImGui::SliderFloat("up", &up, -50.0f, 50.0f);
			ImGui::SliderFloat("bottom", &bottom, -50.0f, 50.0f);
			ImGui::SliderFloat("near", &near_, -50.0f, 50.0f);
			ImGui::SliderFloat("far", &far_, 20.0f, 150.0f);

			view = glm::lookAt(
				glm::vec3(0, 0, 30), // 
				glm::vec3(0, 0, 0), // and looks at the origin
				glm::vec3(0, 1, 0) // Head is up (set to 0,-1,0 to look upside-down)		
			);
			projection = glm::ortho(left, right, up, bottom, near_, far_);

			// retrieve the matrix uniform locations
			int viewLoc = glGetUniformLocation(*shaderProgram, "view");
			int proLoc = glGetUniformLocation(*shaderProgram, "projection");
			glUseProgram(*shaderProgram);

			// pass them to the shaders (3 different ways)
			glUniformMatrix4fv(proLoc, 1, GL_FALSE, &projection[0][0]);
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

			ImGui::End();
		}

		if (setPers)
		{
			ImGui::Begin((const char*)"Set Perspective", (bool*)&setPers);
			ImGui::SliderFloat("Angle", &ang, -50.0f, 50.0f);
			ImGui::SliderFloat("near", &near_, -50.0f, 50.0f);
			ImGui::SliderFloat("far", &far_, 20.0f, 100.0f);
			view = glm::lookAt(
				glm::vec3(0, 0, 30), // 
				glm::vec3(0, 0, 0), // and looks at the origin
				glm::vec3(0, 1, 0) // Head is up (set to 0,-1,0 to look upside-down)		
			);

			projection = glm::perspective(glm::radians(ang), (float)display_w / (float)display_h, near_, far_);

			int viewLoc = glGetUniformLocation(*shaderProgram, "view");
			int proLoc = glGetUniformLocation(*shaderProgram, "projection");
			glUseProgram(*shaderProgram);

			// pass them to the shaders (3 different ways)
			glUniformMatrix4fv(proLoc, 1, GL_FALSE, &projection[0][0]);
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

			ImGui::End();
			
		}

		if (setCha)
		{
			changingView();
		}
		
		ImGui::Begin("Position and Lookat");
		{

			ImGui::Text("Position(x,y,z):(%f , %f , %f)",cam.cameraPos.x, cam.cameraPos.y, cam.cameraPos.z);
			ImGui::Text("Front(x,y,z):(%f , %f , %f)", cam.cameraFront.x, cam.cameraFront.y, cam.cameraFront.z);

		}
		ImGui::End();

		ImGui::Begin((const char*)"Light Parameter");
		{
			ImGui::SliderFloat("Ambient Albedo", &ambientAlbedo, 0.0f,1.0f);
			ImGui::SliderFloat("Diffuse Albedo", &diffuseAlbedo, 0.0f, 1.0f);
			ImGui::SliderFloat("Specular Albedo", &specularAlbedo, 0.0f,1.0f);
			
			ImGui::SliderInt("nShiny", &nShiny, 0,256);
			// retrieve the  uniform locations
			int amLoc = glGetUniformLocation(*shaderProgram, "ambientAlbedo");
			int difLoc = glGetUniformLocation(*shaderProgram, "diffuseAlbedo");
			int speLoc = glGetUniformLocation(*shaderProgram, "specularAlbedo");
			int nShLoc = glGetUniformLocation(*shaderProgram, "nShiny");
			glUseProgram(*shaderProgram);
			glUniform1f(amLoc,ambientAlbedo);
			glUniform1f(difLoc, diffuseAlbedo);
			glUniform1f(speLoc, specularAlbedo);
			glUniform1i(nShLoc, nShiny);
		}
		ImGui::End();


		// Rendering
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Add Primitives")) {
				if (ImGui::MenuItem("Rectangle")) {
					primitivecmd = (primitivecmd ^ rectangle);
				}
				if (ImGui::MenuItem("Triangle")) {
					primitivecmd = (primitivecmd ^ triangle);
				}
				if (ImGui::MenuItem("Line")) {
					primitivecmd = (primitivecmd ^ line);
				}
				ImGui::Separator();
				if (ImGui::BeginMenu("Bresenman Alogorithm")) {
					if (ImGui::MenuItem("Draw Line")) {
						drawLineBresenhamB = !drawLineBresenhamB;
					}
					if (ImGui::MenuItem("Draw Circle")) {
						drawCircleBresenhamB = !drawCircleBresenhamB;

					}
					ImGui::EndMenu();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Cube")) {
					cam.resetCamera();
					//model = glm::mat4(1.0f);
					//model = glm::translate(model, glm::vec3(0.5f, 0.5f, -1.5f));
					//int modelLoc = glGetUniformLocation(*shaderProgram, "model");
					//glUseProgram(*shaderProgram);
					// pass them to the shaders (3 different ways)
					//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
					primitivecmd = primitivecmd ^ cube;
				}
				if (ImGui::MenuItem("Sphere")) {
					cam.resetCamera();
					primitivecmd = primitivecmd ^ ball;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Clear screen")) {
					primitivecmd = transformcmd = drawLineBresenhamB = drawCircleBresenhamB = 0;
					setDefaultPerspective();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Transform")) {
				if (ImGui::MenuItem("Stop")) {
					transformcmd = 0;
				}
				if (ImGui::MenuItem("Rotate")) {
					transformcmd = rotate;
				}
				if (ImGui::MenuItem("Translate")) {
					transformcmd = translate;
				}
				if (ImGui::MenuItem("Scale")) {
					transformcmd = scale;
				}
				if (ImGui::BeginMenu("Combine")) {
					if (ImGui::MenuItem("Roll")) {
						transformcmd = roll;
					}
					if (ImGui::MenuItem("Swing")) {
						transformcmd = swing;
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Projection")) {
				if (ImGui::MenuItem("SetOrthonal")) {
					setOrth = !setOrth;
					
				}
				if (ImGui::MenuItem("SetPerspective")) {
					setPers = !setPers;
					projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -100.0f, 100.0f);

					// retrieve the matrix uniform locations
					int proLoc = glGetUniformLocation(*shaderProgram, "projection");
					glUseProgram(*shaderProgram);

					// pass them to the shaders (3 different ways)
					glUniformMatrix4fv(proLoc, 1, GL_FALSE, &projection[0][0]);
				}
				if (ImGui::MenuItem("Changing view")) {
					setOrth=setPers = 0;
					setCha = !setCha;
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Shading")) {
				if (ImGui::MenuItem("Phong shading")) {
					//shadingcmd = phongShading;
					shaderProgram = &shaderProgram_Phong;
					setDefaultPerspective();
					setDefaultLight();
				}
				if (ImGui::MenuItem("Gouraud shaing")) {
					//shadingcmd = gouraudShading;
					shaderProgram = &shaderProgram_Gouraud;
					setDefaultPerspective();
					setDefaultLight();
				}

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		ImGui::Render();
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	return 0;
}

void drawRec()
{
	memset(vertices, 0, sizeof(float) * 48);
	unsigned int indicesRectangle[] = {
		0,1,2, // 第一个三角形
		1,2, 3  // 第二个三角形
	};
	float rec[] = {
		0.75, 0.25, 0.0f, 1.0f, 0.0f, 0.0f,0.0f,0.0f,1.0f,
		0.75, -0.25, 0.0f, 0.0f, 1.0f, 0.0f,1.0,0.0f,0.0f,1.0f,
		-0.75, 0.25, 0.0f, 0.0f, 0.0f, 1.0f,1.0,0.0f,0.0f,1.0f,
		-0.75, -0.25, 0.0f, 0.0f, 0.0f, 1.0f,0.0f,0.0f,1.0f };
	memcpy(vertices, rec, sizeof(rec));

	ImGui::Begin("Rectangle Setting");
	ImGui::Text("Please design your own rectangle!");
	ImGui::ColorEdit3("Vertex 1 Color", (float*)&rectangleVertColor[0]);
	ImGui::ColorEdit3("Vertex 2 Color", (float*)&rectangleVertColor[1]);
	ImGui::ColorEdit3("Vertex 3 Color", (float*)&rectangleVertColor[2]);
	ImGui::ColorEdit3("Vertex 4 Color", (float*)&rectangleVertColor[3]);
	for (int i = 0; i < 4; i++)
	{
		vertices[i*9 + 3] = rectangleVertColor[i].x;
		vertices[i*9 + 4] = rectangleVertColor[i].y;
		vertices[i*9 + 5] = rectangleVertColor[i].z;
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesRectangle), indicesRectangle, GL_DYNAMIC_DRAW);
	glUseProgram(*shaderProgram);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	ImGui::End();
	
}

void drawTri()
{
	unsigned int indicesTriangle[] = {
		0, 1, 2
	};
	float triangle[] = {
		0.5f,-0.5f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,
		-0.5f,-0.5f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,
		0,0.5f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f
	};
	memcpy(vertices, triangle, sizeof(triangle));

	ImGui::Begin("Triangle Setting");
	ImGui::Text("Please design your own triangle!");
	ImGui::ColorEdit3("Vertex 1 Color", (float*)&triangleVertColor[0]);
	ImGui::ColorEdit3("Vertex 2 Color", (float*)&triangleVertColor[1]);
	ImGui::ColorEdit3("Vertex 3 Color", (float*)&triangleVertColor[2]);
	for (int i = 0; i < 3; i++)
	{
		vertices[i * 9 + 3] = triangleVertColor[i].x;
		vertices[i * 9 + 4] = triangleVertColor[i].y;
		vertices[i * 9 + 5] = triangleVertColor[i].z;
	}
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesTriangle), indicesTriangle, GL_DYNAMIC_DRAW);
	glUseProgram(*shaderProgram);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
	ImGui::End();
}

void drawCub()
{
	
	unsigned int indicesCube[] = {
		0, 1, 2,
		1, 2, 3,
		4,5,6,
		5,6,7,
		8,9,10,
		9,10,11,
		12,13,14,
		13,14,15,
		16,17,18,
		17,18,19,
		20,21,22,
		21,22,23
	};
	float cube[] = {

		//上面0,1,2,3
		0.5f, 0.5f, 0.5f, 0.91f, 0.82f,0.92f,0.0f,1.0f,0.0f,
		-0.5f, 0.5f, 0.5f, 0.91f, 0.82f,0.92f,0.0f,1.0f,0.0f,
		0.5f, 0.5f, -0.5f, 0.91f, 0.82f,0.92f,0.0f,1.0f,0.0f,
		-0.5f, 0.5f, -0.5f, 0.91f, 0.82f,0.92f,0.0f,1.0f,0.0f,
		//前面4,5,6,7
		0.5f, 0.5f, 0.5f,  0.92f, 0.78f,0.89f,0.0f,0.0f,1.0f,
		0.5f, -0.5f, 0.5f,  0.92f, 0.78f,0.89f,0.0f,0.0f,1.0f,
		-0.5f, 0.5f,0.5f,  0.92f, 0.78f,0.89f,0.0f,0.0f,1.0f,
		-0.5f, -0.5f, 0.5f,  0.92f, 0.78f,0.89f,0.0f,0.0f,1.0f,
		//左面8,9,10,11
		-0.5f, 0.5f, 0.5f, 0.87f, 0.66f,0.70f,-1.0f,0.0f,0.0f,
		-0.5f, 0.5f, -0.5f,  0.87f, 0.66f,0.70f,-1.0f,0.0f,0.0f,
		-0.5f, -0.5f, 0.5f, 0.87f, 0.66f,0.70f,-1.0f,0.0f,0.0f,
		-0.5f, -0.5f, -0.5f,0.87f, 0.66f,0.70f,-1.0f,0.0f,0.0f,
		//右面12,13,14,15
		0.5f, 0.5f, 0.5f, 0.71f, 0.72f,0.94f,1.0f,0.0f,0.0f,
		0.5f, 0.5f, -0.5f,  0.71f, 0.72f,0.94f,1.0f,0.0f,0.0f,
		0.5f, -0.5f, 0.5f, 0.71f, 0.72f,0.94f,1.0f,0.0f,0.0f,
		0.5f, -0.5f, -0.5f, 0.71f, 0.72f,0.94f,1.0f,0.0f,0.0f,
		//后面16,17,18,19
		0.5f, 0.5f, -0.5f,  0.86f, 0.92f,0.96f,0.0f,0.0f,-1.0f,
		0.5f, -0.5f, -0.5f,  0.86f, 0.92f,0.96f,0.0f,0.0f,-1.0f,
		-0.5f, 0.5f,-0.5f,  0.86f, 0.92f,0.96f,0.0f,0.0f,-1.0f,
		-0.5f, -0.5f, -0.5f,  0.86f, 0.92f,0.96f,0.0f,0.0f,-1.0f,
		//下面20,21,22,23
		0.5f, -0.5f, 0.5f, 0.48f, 0.40f,0.93f,0.0f,-1.0f,0.0f,
		-0.5f, -0.5f, 0.5f,  0.48f, 0.40f,0.93f,0.0f,-1.0f,0.0f,
		0.5f, -0.5f, -0.5f,  0.48f, 0.40f,0.93f,0.0f,-1.0f,0.0f,
		-0.5f, -0.5f, -0.5f,  0.48f, 0.40f,0.93f,0.0f,-1.0f,0.0f};
	memcpy(vertices, cube, sizeof(cube));
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesCube), indicesCube, GL_DYNAMIC_DRAW);
	glUseProgram(*shaderProgram);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

}

void drawLin()
{

	memset(vertices, 0, sizeof(vertices));
	float line[] = {
		0, 0, 0.0f, 0.0f, 0.0f, 0.0f,0.0f, 0.0f, 1.0f,
		0, -3, 0.0f, 0.0f, 0.0f, 0.0f,0.0f, 0.0f, 1.0f };
	memcpy(vertices, line, sizeof(line));
	ImGui::Begin("Line Setting");
	ImGui::Text("Please design your own Line!");
	ImGui::ColorEdit3("Vertex 1 Color", (float*)&rectangleVertColor[0]);
	ImGui::ColorEdit3("Vertex 2 Color", (float*)&rectangleVertColor[1]);
	for (int i = 0; i < 2; i++)
	{
		vertices[i * 9 + 3] = rectangleVertColor[i].x;
		vertices[i * 9 + 4] = rectangleVertColor[i].y;
		vertices[i * 9 + 5] = rectangleVertColor[i].z;
	}
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glUseProgram(*shaderProgram);
	glDrawArrays(GL_LINE_STRIP, 0, 2);
	ImGui::End();
}

void drawBLine()
{
	memset(vertices, 0, sizeof(float) * 48);
	int x0 = 200, y0 = 200, x1 = 600, y1 = 450;
	int dx = x1 - x0;
	int dy = y1 - y0;
	int p = 2 * dy - dx;
	float m = dy / dx;
	int x = x0, y = y0;
	while (x <= x1)
	{
		//画点
		vertices[0] = float(x - display_w / 2) / (float)display_w, vertices[1] = (float)(y - display_h / 2) / (float)display_h;
		vertices[8] = 1.0f;
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
		glUseProgram(*shaderProgram);
		glDrawArrays(GL_POINTS, 0, 1);
		//计算下一个点
		x++;
		if (p <= 0) p += 2 * dy;
		else {
			y++;
			p += (2 * dy - 2 * dx);
		}
	}
}

void drawBCir()
{
	memset(vertices, 0, sizeof(float) * 80);
	int x0 = 200, y0 = 200, r = 100;
	int x = 0, y = r, d = 3 - 2 * r;
	while (x <= y) {
		//画点
		vertices[0] = float(x0 + x - display_w / 2) / (float)display_w, vertices[1] = (float)(y + y0 - display_h / 2) / (float)display_h; vertices[8] = 1.0f;
		vertices[9] = float(x0 - x - display_w / 2) / (float)display_w, vertices[10] = (float)(y + y0 - display_h / 2) / (float)display_h; vertices[17] = 1.0f;
		vertices[18] = float(x0 + x - display_w / 2) / (float)display_w, vertices[19] = (float)(y0 - y - display_h / 2) / (float)display_h; vertices[26] = 1.0f;
		vertices[27] = float(x0 - x - display_w / 2) / (float)display_w, vertices[28] = (float)(y0 - y - display_h / 2) / (float)display_h; vertices[35] = 1.0f;
		vertices[36] = float(x0 + y - display_w / 2) / (float)display_w, vertices[37] = (float)(y0 + x - display_h / 2) / (float)display_h; vertices[44] = 1.0f;
		vertices[45] = float(x0 + y - display_w / 2) / (float)display_w, vertices[46] = (float)(y0 - x - display_h / 2) / (float)display_h; vertices[53] = 1.0f;
		vertices[54] = float(x0 - y - display_w / 2) / (float)display_w, vertices[55] = (float)(y0 + x - display_h / 2) / (float)display_h; vertices[62] = 1.0f;
		vertices[63] = float(x0 - y - display_w / 2) / (float)display_w, vertices[64] = (float)(y0 - x - display_h / 2) / (float)display_h; vertices[71] = 1.0f;
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
		glUseProgram(*shaderProgram);
		glDrawArrays(GL_POINTS, 0, 8);
		//计算下一个点
		if (d < 0)
			d = d + 4 * x + 6;
		else {
			d = d + 4 * (x - y) + 10;
			y--;
		}
		x++;
	}
}

void drawSphere()
{
	memset(vertices, 0, sizeof(float) * 50);
	unsigned int indicesRectangle[] = {
		0,2,3, // 第一个三角形
		0,1,2  // 第二个三角形
	};
	
	float m = 16, n = 32;//球体细分总数
	int vertNum = m * n * 4;//顶点总数  
	float stepangY = glm::pi<float>() / m;//纵向角度每次增加的值  
	float stepangXZ = glm::pi<float>() * 2 / n;//横向角度每次增加的值  
	float angY = 0.0;//初始的纵向角度  
	float angXZ = 0.0;//初始的横向角度  
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			//构造一个顶点  
			float x1 = sin(angY)*cos(angXZ);
			float z1 = sin(angY)*sin(angXZ);
			float y1 = cos(angY);
			vertices[0] = vertices[6] = x1;
			vertices[1] = vertices[7] = y1;
			vertices[2] = vertices[8] = z1;
			vertices[3] = 1.0f;
			vertices[4] = 0.8f; 
			vertices[5] = 0.0f; 

			float x2 = sin(angY + stepangY)*cos(angXZ);
			float z2 = sin(angY + stepangY)*sin(angXZ);
			float y2 = cos(angY + stepangY);
			vertices[9] = vertices[15] = x2;
			vertices[10] = vertices[16] = y2;
			vertices[11] = vertices[17] = z2;
			vertices[12] = 1.0f;
			vertices[13] = 0.8f;
			vertices[14] = 0.0f;


			float x3 = sin(angY + stepangY)*cos(angXZ + stepangXZ);
			float z3 = sin(angY + stepangY)*sin(angXZ + stepangXZ);
			float y3 = cos(angY + stepangY);
			vertices[18] = vertices[24] = x3;
			vertices[19] = vertices[25] = y3;
			vertices[20] = vertices[26] = z3;
			vertices[21] = 1.0f;
			vertices[22] = 0.8f;
			vertices[23] = 0.0f;

			float x4 = sin(angY)*cos(angXZ + stepangXZ);
			float z4 = sin(angY)*sin(angXZ + stepangXZ);
			float y4 = cos(angY);
			vertices[27] = vertices[33] = x4;
			vertices[28] = vertices[34] = y4;
			vertices[29] = vertices[35] = z4;
			vertices[30] = 1.0f;
			vertices[31] = 0.8f;
			vertices[32] = 0.0f;

			angXZ += stepangXZ;
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesRectangle), indicesRectangle, GL_DYNAMIC_DRAW);
			glUseProgram(*shaderProgram);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		}
		angXZ = 0.0;//每次横向到达2PI角度则横向角度归0  
		angY += stepangY;
	}
}

void tranTran()
{
	model = glm::translate(model, glm::vec3(0.0f, sinf((float)(glfwGetTime()))*0.1, 0.0f));
	int modelLoc = glGetUniformLocation(*shaderProgram, "model");
	glUseProgram(*shaderProgram);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
}

void tranRota()
{
	model = glm::rotate(model, (float)(glfwGetTime()*0.01), glm::vec3(1.0, 0.0, 0.0));
	int modelLoc = glGetUniformLocation(*shaderProgram, "model");
	glUseProgram(*shaderProgram);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
}

void tranScal(float x,float y,float z)
{
	model = glm::scale(model, glm::vec3(x, y, z));
	int modelLoc = glGetUniformLocation(*shaderProgram, "model");
	glUseProgram(*shaderProgram);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
}

void tranRoll()
{
	model = glm::mat4(1.0f);		
	model = glm::translate(model, glm::vec3(sinf((float)(glfwGetTime())) * 3, 0.0f, 0.0f))*glm::rotate(model, -sinf((float)(glfwGetTime())) * 3, glm::vec3(0.0, 0.0, 1.0));
	int modelLoc = glGetUniformLocation(*shaderProgram, "model");
	glUseProgram(*shaderProgram);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
}

void tranSwin()
{
	model = glm::mat4(1.0f);
	model = glm::rotate(model, -abs((sinf((float)(glfwGetTime()))) * 3), glm::vec3(0.0, 0.0, 1.0))*glm::translate(model, glm::vec3(-2.0, 0.0f, 0.0f));
	int modelLoc = glGetUniformLocation(*shaderProgram, "model");
	glUseProgram(*shaderProgram);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
}

void changingView()
{
	float x = (float)(sin(glfwGetTime()));
	float z = (float)(cos(glfwGetTime()));
	cam.cameraPos = glm::vec3(x * 3, 0, z * 3);
	cam.cameraFront = glm::vec3(0,0,0)- cam.cameraPos;
	model = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), (float)display_w / (float)display_h, 0.1f, 100.0f);

	// retrieve the matrix uniform locations
	int modelLoc = glGetUniformLocation(*shaderProgram, "model");
	int proLoc = glGetUniformLocation(*shaderProgram, "projection");
	glUseProgram(*shaderProgram);

	// pass them to the shaders (3 different ways)
	glUniformMatrix4fv(proLoc, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

	drawCub();
}

void setDefaultPerspective()
{

	//坐标系统变换
	view = glm::lookAt(
		glm::vec3(3, 3, 3), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0) // Head is up (set to 0,-1,0 to look upside-down)		
	);
	model = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), (float)display_w / (float)display_h, 0.1f, 100.0f);

	// retrieve the matrix uniform locations
	int modelLoc = glGetUniformLocation(*shaderProgram, "model");
	int viewLoc = glGetUniformLocation(*shaderProgram, "view");
	int proLoc = glGetUniformLocation(*shaderProgram, "projection");
	glUseProgram(*shaderProgram);

	// pass them to the shaders (3 different ways)
	glUniformMatrix4fv(proLoc, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

}

void processInput(GLFWwindow *window, Camera&cam)
{
	float distance = 0.05f;
	// adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cam.moveForward(distance);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cam.moveBack(distance);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cam.moveLeft(distance);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cam.moveRight(distance);
		view = glm::lookAt(
			cam.cameraPos, // 
			cam.cameraPos + cam.cameraFront, // and looks at the origin
			cam.cameraUp 		
		);
		// retrieve the matrix uniform locations
		int viewLoc = glGetUniformLocation(*shaderProgram, "view");
		glUseProgram(*shaderProgram);
		// pass them to the shaders (3 different ways)
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	firstDown = state && !mouseLdown ? 1 : 0;
	mouseLdown = state;
	if (firstDown)
	{
		lastX = xpos;
		lastY = ypos;

	}
	else if (mouseLdown)
	{
		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.05;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		cam.yaw += xoffset;
		cam.pitch += yoffset;

		if (cam.pitch > 89.0f)
			cam.pitch = 89.0f;
		if (cam.pitch < -89.0f)
			cam.pitch = -89.0f;

		cam.rotate();
	}
	
}

void shaderCreate(const char*vertexSource, const char*fragmentSource, unsigned int *shaderProgram)
{
	//创建顶点着色器
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//把顶点着色器代码附到着色器上
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	//检查顶点着色器的编译成功与否
	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	//打印顶点着色器的编译失败信息
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	//编译片段着色器
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	//将着色器链接到该对象
	glAttachShader(*shaderProgram, vertexShader);
	glAttachShader(*shaderProgram, fragmentShader);
	glLinkProgram(*shaderProgram);


	//检查链接是否成功
	glGetProgramiv(*shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(*shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

//默认光源位置,光色
void setDefaultLight()
{
	int lightPosLoc = glGetUniformLocation(*shaderProgram, "lightPos");
	int lightColorLoc = glGetUniformLocation(*shaderProgram, "lightColor");
	glUseProgram(*shaderProgram);
	// pass them to the shaders (3 different ways)
	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightPosLoc, 0.0f, 0.0f, 1.5f);

}