#include "common.h"

const char* loadFile( const char* filename ) 
{
	FILE* fp = NULL;
    fopen_s(&fp, filename, "r" );
	if (fp == NULL) {
		fprintf(stderr, "Error: cannot open \"%s\"\n", filename);
		return NULL;
	}
	// get file size to allocate a buffer
	fseek(fp, 0, SEEK_END);
	size_t len = ftell(fp);
	rewind(fp);
	char* buf = (char*)malloc(sizeof(char) * (len + 4));
	// read in the whole contents: (ASSUMPTION: small file size)
	size_t size = fread(buf, sizeof(char), len, fp);
	fclose(fp);
	buf[size] = '\0';
	// done
	return (const char*)buf;
}

void SetupAspectRatioLock(GLFWwindow* window, 
						  windowTransform* RenderArea,
                          ColorValue AspectR_col)
{
	int winW = 0;
    int winH = 0;
    glfwGetWindowSize(window, &winW, &winH);
    if (winW < winH * RenderArea->aspectRatio){
        RenderArea->width = windowWidth;
        // new_w * h/w
        RenderArea->height = (int)(winW/RenderArea->aspectRatio); 
        RenderArea->pos_x = 0;
        RenderArea->pos_y = (int)(winH - RenderArea->height) / 2;
    }
    else{
        // new_h * w/h
        RenderArea->width = (int)(winH*RenderArea->aspectRatio);
        RenderArea->height = winH; 
        RenderArea->pos_x = (int)(winW - RenderArea->width) / 2;
        RenderArea->pos_y = 0;
    }

    glViewport(RenderArea->pos_x, RenderArea->pos_y, RenderArea->width, RenderArea->height);
    glEnable(GL_SCISSOR_TEST);
    glScissor(RenderArea->pos_x, RenderArea->pos_y, RenderArea->width, RenderArea->height);
    glClearColor(AspectR_col.R, AspectR_col.G, AspectR_col.B, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glScissor(0, 0, winW, winH);
    glDisable(GL_SCISSOR_TEST);
}

glm::vec3 GetAttenuationCoeff(float distance) {
    const auto linear_coeff = glm::vec4(
      8.4523112e-05, 4.4712582e+00, -1.8516388e+00, 3.3955811e+01
    );
    const auto quad_coeff = glm::vec4(
      -7.6103583e-04, 9.0120201e+00, -1.1618500e+01, 1.0000464e+02
    );  

    float kc = 1.0f;
    float d = 1.0f / distance;
    auto dvec = glm::vec4(1.0f, d, d*d, d*d*d);
    float kl = glm::dot(linear_coeff, dvec);
    float kq = glm::dot(quad_coeff, dvec);

    return glm::vec3(kc, max(kl, 0.0f), max(kq*kq, 0.0f));
}