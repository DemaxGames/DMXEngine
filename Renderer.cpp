#include "Renderer.h"
namespace core {
	class Renderer {
	private:
		int w, h;
		int renderMeshesCount;
		

		unsigned int shader;

		glm::mat4 proj;
		glm::mat4 view;
	public:
		Mesh* renderMeshes;
		GLFWwindow* window;
		int Init(int h, int w, const char* windowName) {
			renderMeshes = new Mesh[0];
			renderMeshesCount = 0;

			if(!glfwInit()) return -1;

			window = glfwCreateWindow(h, w, windowName, NULL, NULL);
			if (!window)
			{
				glfwTerminate();
				return -1;
			}
			glfwMakeContextCurrent(window);
			gladLoadGL();


			std::string vertex = readFile("E:\\DMXengineGIT\\DMXengine\\vs.shader");
			std::string fragment = readFile("E:\\DMXengineGIT\\DMXengine\\fs.shader");
			shader = CreateShader(vertex, fragment);
			glUseProgram(shader);

			glEnable(GL_DEPTH_TEST);

			view = glm::mat4(1.0f);
			view = glm::translate(view, glm::vec3(0.0f, 0.0f, -2.0f));
			//view = glm::rotate(view, glm::radians(-10.0f), glm::vec3(1.0f, 0.0f, 0.0f));

			proj = glm::mat4(1.0f);
			proj = glm::perspective(61.0f, 1080.0f / 720.0f, 0.01f, 100.0f);
		}

		int RenderFrame() {
			glfwGetWindowSize(window, &w, &h);
			WindowResize(w, h);
			glEnable(GL_DEPTH_TEST);
			glFrustum(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
			glClearColor((float)47 / 256, (float)109 / 256, (float)108 / 256, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			for(int i = 0; i < renderMeshesCount; i++){
				//std::cout << "working!!!\n";
				renderMeshes[i].Bind();

				glUniformMatrix4fv(glGetUniformLocation(shader, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(renderMeshes[i].model));
				glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(glGetUniformLocation(shader, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(proj));
				
				glDrawArrays(GL_TRIANGLES, 0, renderMeshes[i].bufferSize);
			}
			glfwSwapBuffers(window);
			glfwPollEvents();
			return 0;
		}

		int AddMesh(Mesh mesh) {
			Mesh* temp = renderMeshes;
			renderMeshesCount++;
			renderMeshes = new Mesh[renderMeshesCount];
			renderMeshes = temp;
			renderMeshes[renderMeshesCount-1] = mesh;
			return 0;
		}
		int UpdateMesh() {

		}
		int LoadMesh() {

		}
		int LoadMeshes() {

		}
		void WindowResize(int w, int h) {
			if (h == 0) h = 1;
			float aspect = (float)w / h;
			glViewport(0, 0, w, h);
		}
		unsigned int CompileShader(unsigned int shaderType, std::string& src) {
			unsigned id = 1;
			id = glCreateShader(shaderType);
			const char* raw = src.c_str();
			glShaderSource(id, 1, &raw, nullptr);
			glCompileShader(id);
			GLint isCompiled = 0;
			glGetShaderiv(id, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> errorLog(maxLength);
				glGetShaderInfoLog(id, maxLength, &maxLength, &errorLog[0]);
				if (shaderType == GL_VERTEX_SHADER) {
					std::cout << "in vertex shader\n";
				}
				else if (shaderType == GL_FRAGMENT_SHADER) {
					std::cout << "in fragment shader\n";
				}
				else {
					std::cout << "in unknown shader\n";
				}
				for (int i = 0; i < maxLength; i++) {
					std::cout << errorLog[i];
				}
				std::cout << '\n';
				// Provide the infolog in whatever manor you deem best.
				// Exit with failure.
				glDeleteShader(id); // Don't leak the shader.
			}
			return id;
		}
		unsigned int CreateShader(std::string& vertex, std::string& fragment) {
			unsigned vs = CompileShader(GL_VERTEX_SHADER, vertex);
			unsigned fs = CompileShader(GL_FRAGMENT_SHADER, fragment);

			unsigned program = glCreateProgram();
			glAttachShader(program, vs);
			glAttachShader(program, fs);
			glLinkProgram(program);
			glValidateProgram(program);

			glDeleteShader(vs);
			glDeleteShader(fs);

			return program;
		}
		std::string readFile(std::filesystem::path path) {
			std::ifstream in(path, std::ios::binary);
			const auto sz = std::filesystem::file_size(path);
			std::string result(sz, '\0');
			in.read(result.data(), sz);

			return result;
		}
	};
}