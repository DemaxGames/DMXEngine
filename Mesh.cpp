#include "Mesh.h"

namespace core {
	class Mesh {
	protected:
		float* VBOData;
		GLuint VBO;
	public:
		glm::mat4 model = glm::mat4(1.0f);

		int bufferSize = 3;

		int Bind() {
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(float), VBOData, GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);
			return 0;
		}
		int LoadVBOData(float* data, int dataSize) {
			VBOData = data;
			bufferSize = dataSize;
			return 0;
		}
	};
	class Chunk : public Mesh{
	public:
		
	};
}