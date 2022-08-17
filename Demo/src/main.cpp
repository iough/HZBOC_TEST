#include <glad/glad.h>
#include <glfw3.h>
#include "shader.h"
#include "camera.h"
#include "modelLoader.h"
#include <ProjectConfig.h>

void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modes);
namespace {

	GLFWwindow* window;
	const uint32_t SCR_WIDTH = 800;
	const uint32_t SCR_HEIGHT = 640;

	Camera camera(glm::vec3(0.f, 0.f, 200.f));
	float projNear = 0.1f;
	float projFar = 5000.f;
	float projFeild = projFar - projNear;

	float deltaTime = 0.f;
	float lastTime = 0.f;
	float currentTime = 0.f;

	GLuint fOfflineFBO;
	GLuint tOfflineCA0;
	GLuint tOfflineCA1;
	GLuint tOfflineDS;
	GLuint numLevels;
	uint32_t currentWidth;
	uint32_t currentHeight;

	GLuint bVPMatrixUBO;
	struct VPmatrix {
		glm::mat4 viewMat;
		glm::mat4 projMat;
		glm::mat4 vpMat;
	}vpMatrix;

	ModelBatch batchInstance;

	size_t FrameNum;
	float frameTimer;
	float FPS;
	std::string title;

}
void PrepareTransformMatrix() {
	glGenBuffers(1, &bVPMatrixUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, bVPMatrixUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
void PrepareFrameBuffer() {
	glGenFramebuffers(1, &fOfflineFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, fOfflineFBO);

	glGenTextures(1, &tOfflineCA0);
	glBindTexture(GL_TEXTURE_2D, tOfflineCA0);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tOfflineCA0, 0);

	numLevels = 1 + (GLuint)floorf(log2f(fmaxf(SCR_HEIGHT, SCR_HEIGHT)));
	glGenTextures(1, &tOfflineCA1);
	glBindTexture(GL_TEXTURE_2D, tOfflineCA1);
	glTexStorage2D(GL_TEXTURE_2D, numLevels, GL_R32F, SCR_WIDTH, SCR_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, tOfflineCA1, 0);

	glGenTextures(1, &tOfflineDS);
	glBindTexture(GL_TEXTURE_2D, tOfflineDS);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, SCR_WIDTH, SCR_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tOfflineDS, 0);

	GLenum CAs[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};
	glDrawBuffers(2, CAs);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER::Framebuffer is not complete!\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
int main() {
	{
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

		window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Learn OpenGL", nullptr, nullptr);
		if (window == nullptr) {
			std::cout << "Failed to create GLFW window.\n";
			glfwTerminate();
			return -1;
		}

		glfwMakeContextCurrent(window);
		glfwSetKeyCallback(window, key_callback);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cout << "Failed to initialize GLAD.\n";
			return -1;
		}
	}
	
	Model car("resource/Objects/Car.obj");
	int mtlNum = 5;
	for (int i = 0; i < mtlNum; ++i) {
		batchInstance.LoadMaterials((rand()%10)/10.0f, (rand() % 10) / 10.0f, (rand() % 10) / 10.0f, 0.0f);
	}
	int xInst = 10;
	int yInst = 2;
	int zInst = 10;
	for (auto x = 0; x < xInst; ++x) {
		for (auto y = 0; y < yInst; ++y) {
			for (auto z = 0; z < zInst; ++z) {
				batchInstance.LoadModel(&car, rand() % mtlNum, 0.5f, glm::vec3(100.f * x, 100.f * y, -200.f * z));
			}
		}
	}
	batchInstance.SetUp();

	Shader offlineRenderShader("resource/Shaders/OfflineRender.vert", "resource/Shaders/OfflineRender.frag");
	Shader presentShader("resource/Shaders/Present.vert", "resource/Shaders/Present.frag");
	Shader DepthReduce("resource/Shaders/DepthReduce.comp");
	Shader HizCulling("resource/Shaders/HizCulling.comp");

	PrepareTransformMatrix();
	PrepareFrameBuffer();

	glBindTextureUnit(0, tOfflineCA0);
	glBindTextureUnit(1, tOfflineCA1);
	glBindTextureUnit(2, tOfflineDS);

	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window)) {
		currentTime = static_cast<float>(glfwGetTime());
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		processInput(window);
		
		{	
			glBindFramebuffer(GL_FRAMEBUFFER, fOfflineFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, bVPMatrixUBO);
			vpMatrix.viewMat = camera.GetViewMatrix();
			vpMatrix.projMat = glm::perspective(camera.Zoom, float(SCR_WIDTH) / float(SCR_HEIGHT), projNear, projFar);
			vpMatrix.vpMat = vpMatrix.projMat * vpMatrix.viewMat;		
			glBufferSubData(GL_UNIFORM_BUFFER, 0, 3 * sizeof(glm::mat4), &vpMatrix);
			glUniform3fv(0, 1, &camera.Position[0]);
			offlineRenderShader.use();
			batchInstance.DrawVisible();
		}
		{
			{
				DepthReduce.use();
				currentWidth = SCR_WIDTH;
				currentHeight = SCR_HEIGHT;
				for (GLuint i = 1; i < numLevels; ++i) {
					glBindImageTexture(0, tOfflineCA1, i - 1, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
					glBindImageTexture(1, tOfflineCA1, i, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
					currentWidth = currentWidth < 1 ? 1 : currentWidth / 2;
					currentHeight = currentHeight < 1 ? 1 : currentHeight / 2;
					glDispatchCompute((currentWidth + 15) / 16, (currentHeight + 15) / 16, 1);
					glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
				}
			}
			{
				HizCulling.use();
				glUniform3fv(0, 1, &camera.Position[0]);
				batchInstance.HizCulling(vpMatrix.vpMat, camera.Position, projFeild);
			}
			{
				offlineRenderShader.use();
				glUniform3fv(0, 1, &camera.Position[0]);
				batchInstance.DrawFixed();
			}
		}
		

		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(1.0, 1.0, 1.0, 0.0);
			presentShader.use();
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

		++FrameNum;
		frameTimer += deltaTime;
		if (FrameNum >= 30) {
			FPS = FrameNum / frameTimer;
			FrameNum = 0;
			frameTimer = 0;
			title = "Learn OpenGL (FPS: " + std::to_string(FPS) + ")";
			glfwSetWindowTitle(window, title.c_str());
		}

	}

	glfwTerminate();
	return 0;
}
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		camera.ProcessKeyboard(TURN_UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		camera.ProcessKeyboard(TURN_DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		camera.ProcessKeyboard(TURN_RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		camera.ProcessKeyboard(TURN_LEFT, deltaTime);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modes){
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}