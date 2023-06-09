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