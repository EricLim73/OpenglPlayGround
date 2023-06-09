#include "render.h"

void startRenderMiniMap(windowTransform* wt, ColorValue* cl){
    glViewport(wt->pos_x, wt->pos_y, wt->width, wt->height);
    glEnable(GL_SCISSOR_TEST);
    glScissor(wt->pos_x, wt->pos_y, wt->width, wt->height);
    glClearColor(cl->R, cl->G, cl->B, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void endRenderMiniMap(GLFWwindow* window, windowTransform* wt){
    resetRenderArea(window, wt);
}

void setDefaultMVPShader(unsigned int* shader_id, 
                         const GLfloat *model, 
                         const GLfloat *ViewProj)
{
    int modelLoc = glGetUniformLocation(*shader_id, "model"); 
    int ViewProjLoc = glGetUniformLocation(*shader_id, "ViewProj");
    glUseProgram(*shader_id);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);   
    glUniformMatrix4fv(ViewProjLoc, 1, GL_FALSE, ViewProj);    
}   

// TODO: (EricLim73) Both "setLightParameter" and "setMaterialParameter"
//       is currently called individually. Haven't decide how to structure it.
void setMaterialParameter(unsigned int* shader_id, Material* mat){
    // material
	int loc = glGetUniformLocation(*shader_id, "material.ambient");
	glUniform4fv(loc, 1, glm::value_ptr(mat->ambient));
	loc = glGetUniformLocation(*shader_id, "material.diffuse");
	glUniform4fv(loc, 1, glm::value_ptr(mat->diffuse));
	loc = glGetUniformLocation(*shader_id, "material.specular");
	glUniform4fv(loc, 1, glm::value_ptr(mat->specular));
	loc = glGetUniformLocation(*shader_id, "material.shininess");
	glUniform1f(loc, mat->shininess);
}

void setLightParameter(unsigned int* shader_id, Light* light){
    // light
    int loc = glGetUniformLocation(*shader_id, "light.pos");
	glUniform4fv(loc, 1, glm::value_ptr(light->pos));
	loc = glGetUniformLocation(*shader_id, "light.ambient");
	glUniform4fv(loc, 1, glm::value_ptr(light->ambient));
	loc = glGetUniformLocation(*shader_id, "light.diffuse");
	glUniform4fv(loc, 1, glm::value_ptr(light->diffuse));
	loc = glGetUniformLocation(*shader_id, "light.specular");
	glUniform4fv(loc, 1, glm::value_ptr(light->specular));
	loc = glGetUniformLocation(*shader_id, "light.att");
	glUniform4fv(loc, 1, glm::value_ptr(light->att));
}

void setBlinnPhongParameter(unsigned int* shader_id,
                                const GLfloat* model,
                                const GLfloat* viewPos,
                                const GLfloat* ViewProj)
{
    int modelLoc = glGetUniformLocation(*shader_id, "model");
    int ViewPosLoc = glGetUniformLocation(*shader_id, "viewPos");
    int ViewProjLoc = glGetUniformLocation(*shader_id, "ViewProj");
    glUseProgram(*shader_id);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);   
    glUniformMatrix4fv(ViewPosLoc, 1, GL_FALSE, viewPos); 
    glUniformMatrix4fv(ViewProjLoc, 1, GL_FALSE, ViewProj);
}

void drawObj(render_obj* obj)
{
    glBindVertexArray(obj->vao);         
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, obj->texture);
    if (obj->ebo == -1)
        glDrawArrays(GL_TRIANGLES,0, obj->vert_count);
    else
        glDrawElements(GL_TRIANGLES, obj->vert_count, GL_UNSIGNED_INT, 0);
}
 
void runSpriteAnim(spriteFrameData* spriteFrameInfo)
{
    // TODO: (EricLim73) the initial frame time is not perfect 
    // (always depends on app start time and rednering time so...)
    // Needs fixing...?
    double timeStamp = glfwGetTime();
    double delta = timeStamp - spriteFrameInfo->timeOffset;
    if(delta >= 1.0f / spriteFrameInfo->frames_ps) {
        spriteFrameInfo->timeOffset = timeStamp;
        spriteFrameInfo->uv_x += 1.0f;
        if(spriteFrameInfo->uv_x >= spriteFrameInfo->nx_frames) {
            spriteFrameInfo->uv_x = 0.0f;
        }
    }
}

void drawSpriteAnim(render_obj* obj, spriteFrameData* spriteFrameInfo, float x_dir, float y_dir)
{
    glUseProgram(obj->shader_id);
    glBindTexture(GL_TEXTURE_2D, obj->texture);
    glUniform1f(glGetUniformLocation(obj->shader_id, "x_dir"), x_dir);
    glUniform1f(glGetUniformLocation(obj->shader_id, "y_dir"), y_dir);
    glUniform1f(glGetUniformLocation(obj->shader_id, "uv_x"), spriteFrameInfo->uv_x);
    glUniform1f(glGetUniformLocation(obj->shader_id, "uv_y"), spriteFrameInfo->uv_y);
    glUniform1f(glGetUniformLocation(obj->shader_id, "nx_frames"), spriteFrameInfo->nx_frames);
    glUniform1f(glGetUniformLocation(obj->shader_id, "ny_frames"), spriteFrameInfo->ny_frames);
    glBindVertexArray(obj->vao);
    glDrawElements(GL_TRIANGLES, obj->vert_count, GL_UNSIGNED_INT, 0);
}

void setCamera( Camera* cam, ProjectionMode mode, 
                glm::vec3* pos, glm::vec3* front, glm::vec3* up, 
                float near, float far, float fov )
{
    cam->cameraPos   = *pos;
    cam->cameraFront = *front;
    cam->cameraUp    = *up;
    cam->fov = fov;
    cam->mode = mode;
    cam->yaw = -90.0f;
    cam->pitch = 0.0f;
    cam->row = 0.0f;
    cam->speed = 2.5f;
    cam->sensitivity = 0.1f;
    
    cam->view = glm::lookAt(cam->cameraPos, 
                            cam->cameraPos + cam->cameraFront, 
                            cam->cameraUp); 
    updateProj(cam, near, far);
    cam->ViewProj = cam->proj * cam->view;
}

void CameraStatReset(Camera* cam){
    glm::vec3 pos = glm::vec3(0.0f, 0.0f,  3.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f,  0.0f);
    setCamera(cam, ProjectionMode::PERSPECTIVE, &pos, &front, &up, 0.1f, 100.0f, 45.0f);
}

void CameraRotation(GLFWwindow* window, Camera* cam, Mouse* mouse, float xpos, float ypos)
{
    float xoffset = (float)((float)xpos - mouse->pos.x);
    float yoffset = (float)(mouse->pos.y - (float)ypos);

    xoffset *= cam->sensitivity;
    yoffset *= cam->sensitivity;

    cam->yaw += xoffset;
    cam->pitch += yoffset;
    
    if (cam->pitch > 89.0f)
        cam->pitch = 89.0f;
    if (cam->pitch < -89.0f)
        cam->pitch = -89.0f;
    if (cam->yaw < 0.0f)
        cam->yaw += 360;
    if (cam->yaw > 360)
        cam->yaw -= 360;

    glm::vec3 direction;
    direction.x = cos(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
    direction.z = sin(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
    direction.y = sin(glm::radians(cam->pitch));
    cam->cameraFront = glm::normalize(direction);
    
    cam->view = glm::lookAt(cam->cameraPos, 
                            cam->cameraPos + cam->cameraFront, 
                            cam->cameraUp);
    cam->ViewProj = cam->proj * cam->view;
}

void CameraMovement(GLFWwindow* window, Camera* cam, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        cam->cameraPos += (cam->speed * deltaTime) * cam->cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        cam->cameraPos -= (cam->speed * deltaTime) * cam->cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        cam->cameraPos += (cam->speed * deltaTime) * glm::normalize(glm::cross(cam->cameraFront, 
                                                            cam->cameraUp));
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        cam->cameraPos -= (cam->speed * deltaTime) * glm::normalize(glm::cross(cam->cameraFront, 
                                                            cam->cameraUp));    
    }    
    
    cam->view = glm::lookAt(cam->cameraPos, 
                            cam->cameraPos + cam->cameraFront, 
                            cam->cameraUp);
    cam->ViewProj = cam->proj * cam->view;
}

void CameraZoom(Camera* cam, float zoom){
    cam->fov -= zoom;
    if (cam->fov < 20.0f)
        cam->fov = 20.0f;
    if (cam->fov > 65.0f)
        cam->fov = 65.0f;
    updateProj(cam, 0.1f, 50.0f);
    cam->ViewProj = cam->proj * cam->view;
}

void subTriangles(int level, unsigned long long VertSize,
                  Vert* vertSphere, int* VertIndex, 
                  const glm::vec3 a, const glm::vec3 b, const glm::vec3 c) 
{
	if (*VertIndex >= VertSize) {
		printf("vertex buffer overflow...\n");
		return;
	}
	if (level <= 0) { 
        //glm::vec2 texA, texB, texC;
        //texCoord calculation
		//if (a.z < 0.0F && b.z < 0.0F && c.z < 0.0F) {
		//	texA = glm::vec2((atan2f(-a.x, -a.z) + (float)M_PI) / (float)M_PI / 2.0F,
        //                          0.5f - asinf(a.y) / (float)M_PI / 1.0F);
		//	texB = glm::vec2((atan2f(-b.x, -b.z) + (float)M_PI) / (float)M_PI / 2.0F,
        //                          0.5f - asinf(b.y) / (float)M_PI / 1.0F);
		//	texC = glm::vec2((atan2f(-c.x, -c.z) + (float)M_PI) / (float)M_PI / 2.0F,
        //                          0.5f - asinf(c.y) / (float)M_PI / 1.0F);
		//} else {
		//	texA = glm::vec2(atan2f(a.x, a.z) / (float)M_PI / 2.0F,
        //                          0.5f - asinf(a.y) / (float)M_PI / 1.0F);
		//	texB = glm::vec2(atan2f(b.x, b.z) / (float)M_PI / 2.0F,
        //                          0.5f - asinf(b.y) / (float)M_PI / 1.0F);
        //    texC = glm::vec2(atan2f(c.x, c.z) / (float)M_PI / 2.0F,
        //                          0.5f - asinf(c.y) / (float)M_PI / 1.0F);
		//}
        glm::vec4 color( 1.0f, 1.0f, 1.0f, 1.0f );
		color.r = ((rand() % 1000) / 1000.0F) * 0.25f + 0.75f;
		color.g = ((rand() % 1000) / 1000.0F) * 0.25f + 0.75f;
		color.b = ((rand() % 1000) / 1000.0F) * 0.25f + 0.75f;
        
        vertSphere[*VertIndex].pos = glm::vec4(a.x, a.y, a.z, 1.0F); 
        vertSphere[*VertIndex].norm = glm::vec4(a.x, a.y, a.z, 1.0F); 
        vertSphere[*VertIndex].color = color; 
        vertSphere[*VertIndex].texCoord = a;

        vertSphere[*VertIndex + 1].pos = glm::vec4(b.x, b.y, b.z, 1.0F); 
        vertSphere[*VertIndex + 1].norm = glm::vec4(b.x, b.y, b.z, 1.0F); 
        vertSphere[*VertIndex + 1].color = color;
        vertSphere[*VertIndex + 1].texCoord = b; 

        vertSphere[*VertIndex + 2].pos = glm::vec4(c.x, c.y, c.z, 1.0F); 
        vertSphere[*VertIndex + 2].norm = glm::vec4(c.x, c.y, c.z, 1.0F); 
        vertSphere[*VertIndex + 2].color = color;
        vertSphere[*VertIndex + 2].texCoord = c;   

        *VertIndex += 3; 
	} else {
		glm::vec3 ab = normalize(a + b);
		glm::vec3 bc = normalize(b + c);
		glm::vec3 ca = normalize(c + a);
		// call subTriangles
		subTriangles(level - 1, VertSize, vertSphere, VertIndex, a, ab, ca);
		subTriangles(level - 1, VertSize, vertSphere, VertIndex, b, bc, ab);
		subTriangles(level - 1, VertSize, vertSphere, VertIndex, c, ca, bc);
		subTriangles(level - 1, VertSize, vertSphere, VertIndex, ab, bc, ca);
	}
}

void generateSphere(render_obj* obj,int level) {
    static glm::vec3 v[6] = {
		{ +1, 0, 0 },
		{ 0, +1, 0 },
		{ -1, 0, 0 },
		{ 0, -1, 0 },
		{ 0, 0, +1 },
		{ 0, 0, -1 },
	};
    // pos, normal, color, texCoord
    //glm::vec4 vertSphere[10240 * 4];
    Vert vertSphere[10240];
    int VertIndex = 0; 
    auto VertSize = ArrayCount(vertSphere);
	subTriangles(level, VertSize, vertSphere, &VertIndex, v[0], v[1], v[4]);
	subTriangles(level, VertSize, vertSphere, &VertIndex, v[1], v[2], v[4]);
	subTriangles(level, VertSize, vertSphere, &VertIndex, v[2], v[3], v[4]);
	subTriangles(level, VertSize, vertSphere, &VertIndex, v[3], v[0], v[4]);
	subTriangles(level, VertSize, vertSphere, &VertIndex, v[1], v[0], v[5]);
	subTriangles(level, VertSize, vertSphere, &VertIndex, v[2], v[1], v[5]);
	subTriangles(level, VertSize, vertSphere, &VertIndex, v[3], v[2], v[5]);
	subTriangles(level, VertSize, vertSphere, &VertIndex, v[0], v[3], v[5]);
 
    obj->vao  = -1;
    obj->vbo  = -1;
    obj->ebo  = -1;
    obj->vert_count = VertIndex;
    
    glGenVertexArrays(1, &obj->vao);
    glBindVertexArray(obj->vao);
    
    glGenBuffers(1, &obj->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
    glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(vertSphere), 
                 vertSphere, 
                 GL_STATIC_DRAW);
     
    
    // Attrib_location, element_count, do_Normalize?, element_size(stride), offset
    // "offset" needs typeCasting to void* of GLvoid* -> ex) (GLvoid*)(2*sizeof(float))
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)(offsetof(Vert, norm)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)(offsetof(Vert,color)));
    glEnableVertexAttribArray(2);  
    
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)(offsetof(Vert,texCoord)));
    glEnableVertexAttribArray(3);  
}

void createTriangle(render_obj* obj)
{
    // vertices will be processed in NDC ranging[-1~1]
    // and that will be transferred into Screen-Space Coordinates
    // pos,color,texCoord
    float vertices[] = {
        // positions               // colors               // texture coords
        0.0f,  0.5f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top right
       -0.5f, -0.5f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,   // bottom left
        0.5f, -0.5f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f   // bottom right
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3   // first triangle
    };
    
    obj->vao  = -1;
    obj->vbo  = -1;
    obj->ebo  = -1;
    obj->vert_count = 3;
    
    glGenVertexArrays(1, &obj->vao);
    glBindVertexArray(obj->vao);
    
    glGenBuffers(1, &obj->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
    glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(vertices), 
                 vertices, 
                 GL_STATIC_DRAW);
     
    
    // Attrib_location, element_count, do_Normalize?, element_size(stride), offset
    // "offset" needs typeCasting to void* of GLvoid* -> ex) (GLvoid*)(2*sizeof(float))
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10*sizeof(float), (void*)(4*sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);  
}

void createSquare(render_obj* obj)
{
    // vertices will be processed in NDC ranging[-1~1]
    // and that will be transferred into Screen-Space Coordinates
    // pos,color,texCoord
    
    float vertices[] = {
        // positions                 // colors             // texture coords
         0.5f,  0.5f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f,   // top right
        -0.5f,  0.5f, 0.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f,  0.0f, 1.0f,    // top left 
        -0.5f, -0.5f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 0.0f,   // bottom left
         0.5f, -0.5f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f   // bottom right
    };
    
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    
    obj->vao  = -1;
    obj->vbo  = -1;
    obj->ebo  = -1;
    obj->vert_count = 6;
    
    glGenVertexArrays(1, &obj->vao);
    glBindVertexArray(obj->vao);
    
    glGenBuffers(1, &obj->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
    glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(vertices), 
                 vertices, 
                 GL_STATIC_DRAW);
    
    glGenBuffers(1, &obj->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 sizeof(indices), 
                 indices, 
                 GL_STATIC_DRAW);
    
    // Attrib_location, element_count, do_Normalize?, element_size(stride), offset
    // "offset" needs typeCasting to void* of GLvoid* -> ex) (GLvoid*)(2*sizeof(float))
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10*sizeof(float), (void*)(4*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(2);  
}

void createCube(render_obj* obj)
{
    // vertices will be processed in NDC ranging[-1~1]
    // and that will be transferred into Screen-Space Coordinates
    //   4pos,                     4normal,                  2texCoord    
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 
         0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 
         0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, 1.0f, 
        -0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, 1.0f, 
        -0.5f, -0.5f,  0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 
         0.5f, -0.5f,  0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, 0.0f, 
         0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, 1.0f, 
        -0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 1.0f, 
        -0.5f,  0.5f,  0.5f, 1.0f,  -1.0f,  0.0f,  0.0f,1.0f,  1.0f, 0.0f, 
        -0.5f,  0.5f, -0.5f, 1.0f,  -1.0f,  0.0f,  0.0f,1.0f,  1.0f, 1.0f, 
        -0.5f, -0.5f, -0.5f, 1.0f,  -1.0f,  0.0f,  0.0f,1.0f,  0.0f, 1.0f, 
        -0.5f, -0.5f,  0.5f, 1.0f,  -1.0f,  0.0f,  0.0f,1.0f,  0.0f, 0.0f, 
         0.5f,  0.5f,  0.5f, 1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 1.0f, 
         0.5f, -0.5f, -0.5f, 1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, 1.0f, 
         0.5f, -0.5f,  0.5f, 1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, 0.0f, 
        -0.5f, -0.5f,  0.5f, 1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 
        -0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f
    };
    
    unsigned int indices[] = {
         0,  2,  1,  2,  0,  3,
         4,  5,  6,  6,  7,  4,
         8,  9, 10, 10, 11,  8,
        12, 14, 13, 14, 12, 15,
        16, 17, 18, 18, 19, 16,
        20, 22, 21, 22, 20, 23,
    };
    

    obj->vao  = -1;
    obj->vbo  = -1;
    obj->ebo  = -1;
    obj->vert_count = 36;
    
    glGenVertexArrays(1, &obj->vao);
    glBindVertexArray(obj->vao);
    
    glGenBuffers(1, &obj->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
    glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(vertices), 
                 vertices, 
                 GL_STATIC_DRAW);
    
    glGenBuffers(1, &obj->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 sizeof(indices), 
                 indices, 
                 GL_STATIC_DRAW);
    
    // Attrib_location, element_count, do_Normalize?, element_size(stride), offset
    // "offset" needs typeCasting to void* of GLvoid* -> ex) (GLvoid*)(2*sizeof(float))
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 10*sizeof(float), (void*)(4*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 10*sizeof(float), (void*)(8*sizeof(float)));
    glEnableVertexAttribArray(3);  
}

void createSpriteAnim(render_obj* obj)
{
    // vertices & indices
    float vertices[] = {
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    obj->vao  = -1;
    obj->vbo  = -1;
    obj->ebo  = -1;
    obj->vert_count = 6;

    glGenVertexArrays(1, &obj->vao);
    glBindVertexArray(obj->vao);
    
    glGenBuffers(1, &obj->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
    glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(vertices), 
                 vertices, 
                 GL_STATIC_DRAW);
    
    glGenBuffers(1, &obj->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 sizeof(indices), 
                 indices, 
                 GL_STATIC_DRAW);
    
    // Attrib_location, element_count, do_Normalize?, element_size(stride), offset
    // "offset" needs typeCasting to void* of GLvoid* -> ex) (GLvoid*)(2*sizeof(float))
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(0);
}

void setTexture(render_obj* obj, 
                int textureType, int wrap_s, int wrap_t, 
                int minFileter, int magFilter, 
                const char* texturePath)
{
    glGenTextures(1, &obj->texture);
    glBindTexture(GL_TEXTURE_2D, obj->texture);
    // opengl 4.5 and after can change "textureType" to "obj->texture"
    glTexParameteri(textureType, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(textureType, GL_TEXTURE_WRAP_T, wrap_t);
    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, minFileter);
    glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, magFilter);
    
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(texturePath, 
                                    &obj->tex_width, &obj->tex_height, 
                                    &obj->nrChannels, 0);
    if (data){
        // for now we only store the original imageFormat and render that
        // exact thing. no fancy tricks here.
        int imageType = -1;
        switch (obj->nrChannels){
            default: break;
            case 1: imageType = GL_RED; break;
            case 2: imageType = GL_RG;  break;
            case 3: imageType = GL_RGB; break;
            case 4: imageType = GL_RGBA; break;
        }

        // the first RGB is for how are we gonna save it inside GPU
        // the second RGB is for what we are lookin at inside CPU
        // first 0 is telling us the mipmap-level -> 0 being original
        // second 0 is pixel size for outline border
        glTexImage2D(textureType, 0, imageType, 
                    obj->tex_width, obj->tex_height, 
                    0, imageType, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(textureType);
    }
    else{
        std::cout << "failed to load image" << std::endl;
    }
    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(false);
    glUseProgram(obj->shader_id);
    glUniform1i(glGetUniformLocation(obj->shader_id, "Tex"), 0);
}

// NOTE:  (EricLim73) its a CUBE map, so it only expects 6 filepath
// ok this looks dumb but its easy and i understand what it does 
// so thats a ok for me
void setCubeMapTexture(render_obj* obj, 
                        int wrap_s, int wrap_t,int wrap_r, 
                        int minFileter, int magFilter,
                        const char* positiveX, const char* negativeX,
                        const char* positiveY, const char* negativeY,
                        const char* positiveZ, const char* negativeZ)
{
    // TODO: (EricLim73) Take only the dest path, loop it,
    // and concat the "-X" part for 6 times. I think it doesn't
    // make any performance boost but at least that looks clean (this is a bad idea)

    unsigned char *cubePX = stbi_load(positiveX, 
                                    &obj->tex_width, &obj->tex_height, 
                                    &obj->nrChannels, 0);
    unsigned char *cubeNX = stbi_load(negativeX, 
                                    &obj->tex_width, &obj->tex_height, 
                                    &obj->nrChannels, 0);
    unsigned char *cubePY = stbi_load(positiveY, 
                                    &obj->tex_width, &obj->tex_height, 
                                    &obj->nrChannels, 0);
    unsigned char *cubeNY = stbi_load(negativeY, 
                                    &obj->tex_width, &obj->tex_height, 
                                    &obj->nrChannels, 0);
    unsigned char *cubePZ = stbi_load(positiveZ, 
                                    &obj->tex_width, &obj->tex_height, 
                                    &obj->nrChannels, 0);
    unsigned char *cubeNZ = stbi_load(negativeZ, 
                                    &obj->tex_width, &obj->tex_height, 
                                    &obj->nrChannels, 0);

    int imageType = -1;
    switch (obj->nrChannels){
        default: break;
        case 1: imageType = GL_RED; break;
        case 2: imageType = GL_RG;  break;
        case 3: imageType = GL_RGB; break;
        case 4: imageType = GL_RGBA; break;
    }

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glGenTextures(1, &obj->texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, obj->texture);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFileter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap_t);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap_r);

    // new way of doing the same thing from 4.5 and up
    // SubImage3D(tex, level, 
    //            xoffset, yoffset, zoffset, 
    //            width, height, depth, 
    //            format, dataType, pixels)
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, imageType,
      obj->tex_width, obj->tex_height, 0, imageType, GL_UNSIGNED_BYTE, cubePX);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, imageType,
      obj->tex_width, obj->tex_height, 0, imageType, GL_UNSIGNED_BYTE, cubeNX);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, imageType,
      obj->tex_width, obj->tex_height, 0, imageType, GL_UNSIGNED_BYTE, cubePY);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, imageType,
      obj->tex_width, obj->tex_height, 0, imageType, GL_UNSIGNED_BYTE, cubeNY);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z , 0, imageType,
      obj->tex_width, obj->tex_height, 0, imageType, GL_UNSIGNED_BYTE, cubePZ);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, imageType,
      obj->tex_width, obj->tex_height, 0, imageType, GL_UNSIGNED_BYTE, cubeNZ);


    glGenerateTextureMipmap(obj->texture);

    stbi_image_free(cubePX);
    stbi_image_free(cubeNX);
    stbi_image_free(cubePY);
    stbi_image_free(cubeNY);
    stbi_image_free(cubePZ);
    stbi_image_free(cubeNZ);
    glUseProgram(obj->shader_id);
    glUniform1i(glGetUniformLocation(obj->shader_id, "TexCube"), 0);
}

unsigned int CreateShaderProgram(const char* vertShaderPath, 
                                 const char* fragShaderPath)
{
    const char* vertSource = loadFile(vertShaderPath);
    unsigned int vertShader;
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertSource, NULL);
    glCompileShader(vertShader);    
    
    const char* fragSource = loadFile(fragShaderPath);
    unsigned int fragShader;
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSource, NULL);
    glCompileShader(fragShader);    
    
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
        std::cout << "[Error::SHADER::VERTEX::COMPILATION_FAILED]: " << infoLog << std::endl;
    }
    
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        std::cout << "[Error::SHADER::FRAGMENT::COMPILATION_FAILED]: " << infoLog << std::endl;
    }
    
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);
    
    glGetShaderiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "[Error::SHADER::PROGRAM::LINK_FAILED]: " << infoLog << std::endl;
    }
    glDetachShader(shaderProgram, vertShader);
    glDetachShader(shaderProgram,fragShader);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    
    return shaderProgram;
}
 