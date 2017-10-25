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
enum PrimitivesCmd
{
	cube = 0001,triangle=0010,rectangle=0100,line = 01000,ball =010000
};

enum TransformCmd
{
	scale = 0001, translate = 0010, rotate = 0100,roll = 01000,swing = 010000
};

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aColor;\n"
"out vec4 vertColor;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection*view*model*vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"   vertColor = vec4(aColor,1.0); // 输出顶点颜色\n"
"}\0";


const char *fragmentShaderSource = "#version 330 core\n"
"in vec4 vertColor;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vertColor;\n"
"}\n\0";

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

//
void drawRec();
void drawTri();
void drawCub();
void drawBLine();
void drawBCir();
void drawLin();

void tranTran();
void tranRota();
void tranScal(float,float,float);
void tranRoll();
void tranSwin();

//绘制的点数组
float vertices[60] = { 0.0f };
//VBO VAO声明
unsigned int VBO,VAO,EBO;
//创建渲染程序
unsigned int shaderProgram;

int display_w, display_h;


glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

ImVec4 triangleVertColor[3] = {
	ImVec4(1.0f, 0.0f, 0.0f,1.0),
	ImVec4(0.0f, 1.0f, 0.0f,1.0),
	ImVec4(0.0f, 0.0f, 1.0f,1.0)
};

ImVec4 rectangleVertColor[4] = {
	ImVec4(1.0,0.0,0.0,1.0),
	ImVec4(1.0,0.0,0.0,1.0),
	ImVec4(1.0,1.0,1.0,1.0),
	ImVec4(1.0,1.0,1.0,1.0)
};



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
		ImVec4 clear_color = ImVec4(1.0f,1.0f,0.8f, 1.00f);

		glfwGetFramebufferSize(window, &display_w, &display_h);
	
		//着色器创建编译绑定
	
		//创建顶点着色器
		unsigned int vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);

		//把顶点着色器代码附到着色器上
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
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
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		
		shaderProgram = glCreateProgram();

		//将着色器链接到该对象
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);


		//检查链接是否成功
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	
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
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//设置顶点颜色属性
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(1);

		
		glEnable(GL_DEPTH_TEST);
		//解绑
		//glBindBuffer(GL_ARRAY_BUFFER, 0);

		//判断变量
		int primitivecmd = 0000;
		int transformcmd = 0000;
		BOOL drawCircleBresenhamB = FALSE;
		BOOL drawLineBresenhamB = FALSE;
		BOOL drawCross = FALSE;
		

		//坐标系统变换
		view = glm::lookAt(
			glm::vec3(3, 3, 3), // Camera is at (4,3,3), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0) // Head is up (set to 0,-1,0 to look upside-down)		
		);
		model = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (float)display_w / (float)display_h, 0.1f, 100.0f);

		// retrieve the matrix uniform locations
		int modelLoc = glGetUniformLocation(shaderProgram, "model");
		int viewLoc = glGetUniformLocation(shaderProgram, "view");
		int proLoc = glGetUniformLocation(shaderProgram, "projection");
		glUseProgram(shaderProgram);

		// pass them to the shaders (3 different ways)
		glUniformMatrix4fv(proLoc, 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);



	while (!glfwWindowShouldClose(window))
	{
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
			float s = abs(sinf((float)glfwGetTime()));
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
					primitivecmd = primitivecmd ^ cube;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Clear screen")) {
					primitivecmd = transformcmd = drawLineBresenhamB = drawCircleBresenhamB = 0;
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
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Combine")) {
				if (ImGui::MenuItem("Roll")) {
					transformcmd = roll;
				}
				if (ImGui::MenuItem("Swing")) {
					transformcmd = swing;
				}
				if (ImGui::MenuItem("Cross")) {
					drawCross = !drawCross;
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
		0.75, 0.25, 0.0f, 1.0f, 0.0f, 0.0f,
		0.75, -0.25, 0.0f, 0.0f, 1.0f, 0.0f,
		-0.75, 0.25, 0.0f, 0.0f, 0.0f, 1.0f,
		-0.75, -0.25, 0.0f, 0.0f, 0.0f, 1.0f };
	memcpy(vertices, rec, sizeof(rec));

	ImGui::Begin("Rectangle Setting");
	ImGui::Text("Please design your own rectangle!");
	ImGui::ColorEdit3("Vertex 1 Color", (float*)&rectangleVertColor[0]);
	ImGui::ColorEdit3("Vertex 2 Color", (float*)&rectangleVertColor[1]);
	ImGui::ColorEdit3("Vertex 3 Color", (float*)&rectangleVertColor[2]);
	ImGui::ColorEdit3("Vertex 4 Color", (float*)&rectangleVertColor[3]);
	for (int i = 0; i < 4; i++)
	{
		vertices[i*6 + 3] = rectangleVertColor[i].x;
		vertices[i*6 + 4] = rectangleVertColor[i].y;
		vertices[i*6 + 5] = rectangleVertColor[i].z;
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesRectangle), indicesRectangle, GL_DYNAMIC_DRAW);
	glUseProgram(shaderProgram);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	ImGui::End();
	
}

void drawTri()
{
	unsigned int indicesTriangle[] = {
		0, 1, 2
	};
	float triangle[] = {
		0.5f,-0.5f,0.0f,0.0f,0.0f,0.0f,
		-0.5f,-0.5f,0.0f,0.0f,0.0f,0.0f,
		0,0.5f,0.0f,0.0f,0.0f,0.0f,
	};
	memcpy(vertices, triangle, sizeof(triangle));

	ImGui::Begin("Triangle Setting");
	ImGui::Text("Please design your own triangle!");
	ImGui::ColorEdit3("Vertex 1 Color", (float*)&triangleVertColor[0]);
	ImGui::ColorEdit3("Vertex 2 Color", (float*)&triangleVertColor[1]);
	ImGui::ColorEdit3("Vertex 3 Color", (float*)&triangleVertColor[2]);
	for (int i = 0; i < 3; i++)
	{
		vertices[i * 6 + 3] = triangleVertColor[i].x;
		vertices[i * 6 + 4] = triangleVertColor[i].y;
		vertices[i * 6 + 5] = triangleVertColor[i].z;
	}
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesTriangle), indicesTriangle, GL_DYNAMIC_DRAW);
	glUseProgram(shaderProgram);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
	ImGui::End();
}

void drawCub()
{
	unsigned int indicesCube[] = {
		0, 1, 2,
		1, 2, 3,
		0,1,4,
		4,1,5,
		4,5,6,
		5,6,7,
		2,3,6,
		3,6,7,
		0,2,4,
		4,2,6,
		1,3,5,
		3,5,7
	};
	float cube[] = {
		0.5f, 0.5f, 0.5f, 0.48f, 0.40f,0.93f,
		-0.5f, 0.5f, 0.5f,  0.48f, 0.40f,0.93f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.75f,0.79f,
		-0.5f, -0.5f, 0.5f, 1.0f, 0.75f,0.79f,
		0.5f, 0.5f, -0.5f,  0.48f, 0.40f,0.93f,
		-0.5f, 0.5f, -0.5f,  0.48f, 0.40f,0.93f,
		0.5f, -0.5f,-0.5f,  1.0f, 0.75f,0.79f,
		-0.5f, -0.5f, -0.5f,  1.0f, 0.75f,0.79f };
	memcpy(vertices, cube, sizeof(cube));
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesCube), indicesCube, GL_DYNAMIC_DRAW);
	glUseProgram(shaderProgram);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	//glBindVertexArray(VAO);
	//glDrawArrays(GL_TRIANGLES, 0, 36);

	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	//glUseProgram(shaderProgram);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
}

void drawLin()
{

	memset(vertices, 0, sizeof(vertices));
	float line[] = {
		0, 0, 0.0f, 0.0f, 0.0f, 0.0f,
		0, -3, 0.0f, 0.0f, 0.0f, 0.0f };
	memcpy(vertices, line, sizeof(line));
	ImGui::Begin("Line Setting");
	ImGui::Text("Please design your own Line!");
	ImGui::ColorEdit3("Vertex 1 Color", (float*)&rectangleVertColor[0]);
	ImGui::ColorEdit3("Vertex 2 Color", (float*)&rectangleVertColor[1]);
	for (int i = 0; i < 2; i++)
	{
		vertices[i * 6 + 3] = rectangleVertColor[i].x;
		vertices[i * 6 + 4] = rectangleVertColor[i].y;
		vertices[i * 6 + 5] = rectangleVertColor[i].z;
	}
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glUseProgram(shaderProgram);
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
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
		glUseProgram(shaderProgram);
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

void drawHam()
{
	drawCub();

}

void drawBCir()
{
	memset(vertices, 0, sizeof(float) * 48);
	int x0 = 200, y0 = 200, r = 100;
	int x = 0, y = r, d = 3 - 2 * r;
	while (x <= y) {
		//画点
		vertices[0] = float(x0 + x - display_w / 2) / (float)display_w, vertices[1] = (float)(y + y0 - display_h / 2) / (float)display_h;
		vertices[6] = float(x0 - x - display_w / 2) / (float)display_w, vertices[7] = (float)(y + y0 - display_h / 2) / (float)display_h;
		vertices[12] = float(x0 + x - display_w / 2) / (float)display_w, vertices[13] = (float)(y0 - y - display_h / 2) / (float)display_h;
		vertices[18] = float(x0 - x - display_w / 2) / (float)display_w, vertices[19] = (float)(y0 - y - display_h / 2) / (float)display_h;
		vertices[24] = float(x0 + y - display_w / 2) / (float)display_w, vertices[25] = (float)(y0 + x - display_h / 2) / (float)display_h;
		vertices[30] = float(x0 + y - display_w / 2) / (float)display_w, vertices[31] = (float)(y0 - x - display_h / 2) / (float)display_h;
		vertices[36] = float(x0 - y - display_w / 2) / (float)display_w, vertices[37] = (float)(y0 + x - display_h / 2) / (float)display_h;
		vertices[42] = float(x0 - y - display_w / 2) / (float)display_w, vertices[43] = (float)(y0 - x - display_h / 2) / (float)display_h;
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
		glUseProgram(shaderProgram);
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

void tranTran()
{
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, sinf((float)(glfwGetTime())), 0.0f));
	int modelLoc = glGetUniformLocation(shaderProgram, "model");
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
}

void tranRota()
{
	model = glm::mat4(1.0f);
	model = glm::rotate(model, (float)(glfwGetTime()), glm::vec3(1.0, 0.0, 0.0));
	int modelLoc = glGetUniformLocation(shaderProgram, "model");
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
}

void tranScal(float x,float y,float z)
{
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(x, y, z));
	int modelLoc = glGetUniformLocation(shaderProgram, "model");
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
}

void tranRoll()
{
	model = glm::mat4(1.0f);		
	model = glm::translate(model, glm::vec3(sinf((float)(glfwGetTime())) * 3, 0.0f, 0.0f))*glm::rotate(model, -sinf((float)(glfwGetTime())) * 3, glm::vec3(0.0, 0.0, 1.0));
	int modelLoc = glGetUniformLocation(shaderProgram, "model");
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
}

void tranSwin()
{
	model = glm::mat4(1.0f);
	model = glm::rotate(model, -abs((sinf((float)(glfwGetTime()))) * 3), glm::vec3(0.0, 0.0, 1.0))*glm::translate(model, glm::vec3(-2.0, 0.0f, 0.0f));
	int modelLoc = glGetUniformLocation(shaderProgram, "model");
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
}