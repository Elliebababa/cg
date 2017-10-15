
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
const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aColor;\n"
"out vec4 vertColor;\n"
"uniform mat4 projection;\n"

"void main()\n"
"{\n"
"   gl_Position = projection * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
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

int main()
{
	//创建窗口
		glfwSetErrorCallback(error_callback);
		if (!glfwInit())
			return 1;
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		GLFWwindow* window = glfwCreateWindow(800, 495, "ELLIE'S PROJECT", NULL, NULL);
		glfwMakeContextCurrent(window);
		glfwSwapInterval(1); // Enable vsync
		gl3wInit();
		ImGui_ImplGlfwGL3_Init(window, true);
		ImVec4 clear_color = ImVec4(1.0f,1.0f,0.8f, 1.00f);


		int display_w, display_h;
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

		//创建渲染程序
		unsigned int shaderProgram;
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
	
	
		//绘制的点数组
		float vertices[60] = {0.0f};

		//VBO VAO声明
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		unsigned int EBO;
		glGenBuffers(1, &EBO);

		//设置变换，将坐标转化为标准化坐标
		glm::mat4 projection;
		projection = glm::ortho(0.0f, float(display_w), 0.0f, float(display_h));
		int pro = glGetUniformLocation(shaderProgram, "projection");
		glUseProgram(shaderProgram);
		glUniformMatrix4fv(pro,1, FALSE, &(projection[0][0]));

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

		//解绑
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	//判断变量
		BOOL drawRectangleB = FALSE;
		BOOL drawTriangleB = FALSE; 
		BOOL drawLineB = FALSE;
		BOOL drawCircleBresenhamB = FALSE;
		BOOL drawLineBresenhamB = FALSE;
		BOOL drawEdgeWalkingB = FALSE;
		BOOL drawEdgeEquationB = FALSE;

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

	while (!glfwWindowShouldClose(window))
	{
		
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		if (drawRectangleB)
		{
			unsigned int indicesRectangle[] = {
				0,1,2, // 第一个三角形
				1,2, 3  // 第二个三角形
			};
			float rec[] = {
				display_w / 4, display_h *3/ 4, 0.0f, 1.0f, 0.0f, 0.0f,
				display_w * 3 / 4, display_h*3 / 4, 0.0f, 0.0f, 1.0f, 0.0f,
				display_w / 4, display_h / 4, 0.0f, 0.0f, 0.0f, 1.0f,
				display_w *3 / 4, display_h / 4, 0.0f, 0.0f, 0.0f, 1.0f };
			memcpy(vertices, rec, sizeof(rec));
			ImGui::Begin("Rectangle Setting");
			ImGui::Text("Please design your own rectangle!");
			ImGui::ColorEdit3("Vertex 1 Color", (float*)&rectangleVertColor[0]);
			ImGui::ColorEdit3("Vertex 2 Color", (float*)&rectangleVertColor[1]);
			ImGui::ColorEdit3("Vertex 3 Color", (float*)&rectangleVertColor[2]);
			ImGui::ColorEdit3("Vertex 4 Color", (float*)&rectangleVertColor[3]);
			for (int i = 0; i < 4; i++)
			{
				vertices[i * 6 + 3] = rectangleVertColor[i].x;
				vertices[i * 6 + 4] = rectangleVertColor[i].y;
				vertices[i * 6 + 5] = rectangleVertColor[i].z;
			}
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesRectangle), indicesRectangle, GL_DYNAMIC_DRAW);
			glUseProgram(shaderProgram);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			ImGui::End();
		}

		if (drawTriangleB)
		{
			unsigned int indicesTriangle[] = {
				0, 1, 2
			};
			float triangle[] = {
				display_w/4, display_h / 4, 0.0f, 1.0f, 0.0f, 0.0f,
				display_w *3 / 4, display_h / 4, 0.0f, 0.0f, 1.0f, 0.0f,
				display_w / 2, display_h * 3 / 4, 0.0f, 0.0f, 0.0f, 1.0f };
			memcpy(vertices, triangle, sizeof(triangle));
			
			ImGui::Begin("Triangle Setting");
			ImGui::Text("Please design your own triangle!");
			ImGui::ColorEdit3("Vertex 1 Color",(float*)&triangleVertColor[0]);
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

		if (drawLineB)
		{
			float line[] = {
				display_w * 3 / 4, display_h * 3 / 4, 1.0f, 0.0f, 0.0f, 0.0f,
				display_w / 4, display_h / 4, 1.0f, 0.0f, 0.0f, 0.0f };
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
		if (drawLineBresenhamB)
		{
			memset(vertices, 0, sizeof(float) * 48);
			int x0 = 200, y0 = 200, x1 = 600, y1 = 450;
			int dx = x1 - x0;
			int dy = y1 - y0;
			int p = 2 * dy - dx;
			float m = dy / dx;
			int x = x0, y = y0;
			while(x<=x1)
			{
				//画点
				vertices[0] = x, vertices[1] = y;
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
				glUseProgram(shaderProgram);
				glDrawArrays(GL_POINTS, 0, 1);
				//计算下一个点
				x++;
				if (p <= 0) p += 2 * dy;
				else {
					y++;
					p += (2*dy - 2*dx);
				}
			}
		}
		if (drawCircleBresenhamB)
		{
			memset(vertices, 0, sizeof(float) * 48);
			int x0 = 200, y0 = 200, r = 100;
			int x = 0,y = r, d = 3 - 2*r;
			while (x <= y) {
				//画点
				vertices[0] = x0 + x, vertices[1] = y + y0;
				vertices[6] = x0 - x, vertices[7] = y + y0;
				vertices[12] = x0 + x, vertices[13] = y0 - y;
				vertices[18] = x0 - x, vertices[19] = y0 - y;
				vertices[24] = x0 + y, vertices[25] = y0 + x;
			    vertices[30] = x0 + y, vertices[31] = y0 - x;
				vertices[36] = x0 - y, vertices[37] = y0 + x;
				vertices[42] = x0 - y, vertices[43] = y0 - x;
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
		if (drawEdgeEquationB)
		{
			memset(vertices, 0, sizeof(float) * 6);
			int x0 = 400, y0 = 300, x1 = 100,y1= 200,x2 = 700,y2 = 200;
			int xmin = (x0 < x1) ? x0 : x1;
			xmin = (xmin < x2) ? xmin : x2;

			while (x <= y) {
				//画点
				vertices[0] = x0 + x, vertices[1] = y + y0;
				vertices[6] = x0 - x, vertices[7] = y + y0;
				vertices[12] = x0 + x, vertices[13] = y0 - y;
				vertices[18] = x0 - x, vertices[19] = y0 - y;
				vertices[24] = x0 + y, vertices[25] = y0 + x;
				vertices[30] = x0 + y, vertices[31] = y0 - x;
				vertices[36] = x0 - y, vertices[37] = y0 + x;
				vertices[42] = x0 - y, vertices[43] = y0 - x;
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

		// Rendering
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Add Primitives")) {
				if (ImGui::MenuItem("Rectangle")) {
					drawRectangleB = !drawRectangleB;
				}
				if (ImGui::MenuItem("Triangle")) {
					drawTriangleB = !drawTriangleB;
				}
				if (ImGui::MenuItem("Line")) {
					drawLineB = !drawLineB;
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
				if (ImGui::BeginMenu("Triangle Rasterization")) {
					if (ImGui::MenuItem("Edge Equation")) {
						drawEdgeEquationB = !drawEdgeEquationB;
					}
					ImGui::EndMenu();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Clear screen")) {
					drawTriangleB = 0;
					drawRectangleB = 0;
					drawLineB = 0;
					drawLineBresenhamB = 0;
					drawCircleBresenhamB = 0;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Transform")) {
				if (ImGui::MenuItem("Rotate")) {}
				if (ImGui::BeginMenu("Scale")) {
					if (ImGui::MenuItem("Up")) {}
					if (ImGui::MenuItem("Down")) {}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		ImGui::Render();
		glfwSwapBuffers(window);

	}

	// Cleanup
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	return 0;
}