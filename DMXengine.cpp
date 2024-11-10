// DMXengine.cpp: определяет точку входа для приложения.
//

#include "DMXengine.h"

using namespace std;

int main()
{
	core::Renderer renderer;
	renderer.Init(1080, 720, "DMXEngine");
	core::Mesh chunk;
	float* somedata = new float[3]{ (float)(0b0000000000000000111111010000000), (float)0b0000000000000001000000010000010, (float)0b00000000000000001000001010000000 };
	chunk.LoadVBOData(somedata, 3);
	renderer.AddMesh(chunk);

	renderer.RenderFrame();
    while (!glfwWindowShouldClose(renderer.window))
    {
		renderer.renderMeshes[0].model = glm::rotate(renderer.renderMeshes[0].model, 0.002f, glm::vec3(0.0f, 1.0f, 0.0f));
		std::cout << "working!\n";
		renderer.RenderFrame();
    }
	return 0;
}