#include "render.h"

void setDefaultMVPShader(unsigned int* shader_id, 
                         Camera* cam,
                         glm::mat4* model)
{
    int modelLoc = glGetUniformLocation(*shader_id, "model"); 
    int ViewProjLoc = glGetUniformLocation(*shader_id, "ViewProj");
    glUseProgram(*shader_id);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(*model));    
    glUniformMatrix4fv(ViewProjLoc, 1, GL_FALSE, glm::value_ptr(cam->ViewProj));   
}   

// TODO: general idea of the function NOT USED YET
void setLightShaderParameter(unsigned int* shader_id, 
                             Camera* cam, Material* m_material, 
                             Light* light, glm::mat4* model)
{
    int modelLoc = glGetUniformLocation(*shader_id, "model"); 
    int ViewProjLoc = glGetUniformLocation(*shader_id, "ViewProj");
    
    int lightPosLoc = glGetUniformLocation(*shader_id, "light.position");
    int lightDirectionLoc = glGetUniformLocation(*shader_id, "light.direction");
    
    int lightAmbientLoc = glGetUniformLocation(*shader_id, "light.ambient");
    int lightDiffuseLoc = glGetUniformLocation(*shader_id, "light.diffuse");
    int lightSpecularLoc = glGetUniformLocation(*shader_id, "light.specular");
    
    int lightAttenuationLoc = glGetUniformLocation(*shader_id, "light.attenuation");
    int lightCutoffLoc = glGetUniformLocation(*shader_id, "light.cutoff");
    
    int lightTypeLoc = glGetUniformLocation(*shader_id, "light.lightType");
    
    int materialShininessLoc = glGetUniformLocation(*shader_id, "material.shininess");
    int viewPosLoc = glGetUniformLocation(*shader_id, "viewPos");

    glUseProgram(*shader_id);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(*model));
    glUniformMatrix4fv(ViewProjLoc, 1, GL_FALSE, glm::value_ptr(cam->ViewProj));
    
    glUniform1i(lightTypeLoc, (int)light->type);
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(light->position));
    glUniform3fv(lightDirectionLoc, 1, glm::value_ptr(light->direction));

    glUniform3fv(lightAmbientLoc, 1, glm::value_ptr(light->ambient));
	glUniform3fv(lightDiffuseLoc, 1, glm::value_ptr(light->diffuse));
	glUniform3fv(lightSpecularLoc, 1, glm::value_ptr(light->specular));
	
    glm::vec3 attCoef = GetAttenuationCoeff(light->affectRange);
    glUniform3fv(lightAttenuationLoc, 1, glm::value_ptr(attCoef));
    glm::vec2 cutoffInCosine = glm::vec2(cosf(glm::radians(light->cutoff[0])),
                                         cosf(glm::radians(light->cutoff[0] + 
                                                           light->cutoff[0])));
    glUniform2fv(lightCutoffLoc, 1, glm::value_ptr(cutoffInCosine));
    
    glUniform1f(materialShininessLoc, m_material->shininess);
    // NOTE: "timeStamp" for emmision animation effect
    glUniform1f(glGetUniformLocation(*shader_id, "timeStamp"), (float)glfwGetTime());
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(cam->cameraPos));

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

void drawObj(renderPrimitive* obj)
{
    glBindVertexArray(obj->vao);         
    bindTextures(obj);
    if (obj->ebo == -1)
        glDrawArrays(GL_TRIANGLES,0, obj->vert_count);
    else
        glDrawElements(GL_TRIANGLES, obj->vert_count, GL_UNSIGNED_INT, 0);
}

void setSpriteUniform(renderPrimitive* obj, 
                      spriteFrameData* spriteFrameInfo, 
                      float x_dir, float y_dir)
{
    glUseProgram(obj->shader_id);
    glUniform1f(glGetUniformLocation(obj->shader_id, "x_dir"), x_dir);
    glUniform1f(glGetUniformLocation(obj->shader_id, "y_dir"), y_dir);
    glUniform1f(glGetUniformLocation(obj->shader_id, "uv_x"), spriteFrameInfo->uv_x);
    glUniform1f(glGetUniformLocation(obj->shader_id, "uv_y"), spriteFrameInfo->uv_y);
    glUniform1f(glGetUniformLocation(obj->shader_id, "nx_frames"), spriteFrameInfo->nx_frames);
    glUniform1f(glGetUniformLocation(obj->shader_id, "ny_frames"), spriteFrameInfo->ny_frames);
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
    cam->focus = false;
    
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
    cam->focus = false;
    setCamera(cam, ProjectionMode::PERSPECTIVE, &pos, &front, &up, 0.1f, 100.0f, 45.0f);
}

void CameraRotation(GLFWwindow* window, Camera* cam, Mouse* mouse, float xpos, float ypos)
{
    if (!cam->focus)
        return;
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
    if (!cam->focus)
        return;
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
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
        cam->cameraPos += (cam->speed * deltaTime) * glm::normalize(cam->cameraUp);    
    }  
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
        cam->cameraPos -= (cam->speed * deltaTime) * glm::normalize(cam->cameraUp);    
    }

    cam->view = glm::lookAt(cam->cameraPos, 
                            cam->cameraPos + cam->cameraFront, 
                            cam->cameraUp);
    cam->ViewProj = cam->proj * cam->view;
}

void CameraZoom(Camera* cam, float zoom){
    if (!cam->focus)
        return;
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
    /*
    if (a.z < 0.0F && b.z < 0.0F && c.z < 0.0F) {
        texCoordSphere[numVertSphere].s = (atan2f(-a.x, -a.z) + (float)M_PI) / (float)M_PI / 2.0F;
        texCoordSphere[numVertSphere].t = 0.5f - asinf(a.y) / (float)M_PI / 1.0F;
        texCoordSphere[numVertSphere+1].s = (atan2f(-b.x, -b.z) + (float)M_PI) / (float)M_PI / 2.0F;
        texCoordSphere[numVertSphere+1].t = 0.5f - asinf(b.y) / (float)M_PI / 1.0F;
        texCoordSphere[numVertSphere+2].s = (atan2f(-c.x, -c.z) + (float)M_PI) / (float)M_PI / 2.0F;
        texCoordSphere[numVertSphere+2].t = 0.5f - asinf(c.y) / (float)M_PI / 1.0F;
    } else {
        texCoordSphere[numVertSphere].s = atan2f(a.x, a.z) / (float)M_PI / 2.0F;
        texCoordSphere[numVertSphere].t = 0.5f - asinf(a.y) / (float)M_PI / 1.0F;
        texCoordSphere[numVertSphere+1].s = atan2f(b.x, b.z) / (float)M_PI / 2.0F;
        texCoordSphere[numVertSphere+1].t = 0.5f - asinf(b.y) / (float)M_PI / 1.0F;
        texCoordSphere[numVertSphere+2].s = atan2f(c.x, c.z) / (float)M_PI / 2.0F;
        texCoordSphere[numVertSphere+2].t = 0.5f - asinf(c.y) / (float)M_PI / 1.0F; 
    }
    */

    if (level <= 0) { 
        vertSphere[*VertIndex].pos = glm::vec4(a.x, a.y, a.z, 1.0F); 
        vertSphere[*VertIndex].norm = glm::vec4(a.x, a.y, a.z, 1.0F);  
        vertSphere[*VertIndex].texCoord = glm::vec4(a.x, a.y, a.z, 1.0F);
        vertSphere[*VertIndex + 1].pos = glm::vec4(b.x, b.y, b.z, 1.0F); 
        vertSphere[*VertIndex + 1].norm = glm::vec4(b.x, b.y, b.z, 1.0F);  
        vertSphere[*VertIndex + 1].texCoord = glm::vec4(b.x, b.y, b.z, 1.0F); 
        vertSphere[*VertIndex + 2].pos = glm::vec4(c.x, c.y, c.z, 1.0F); 
        vertSphere[*VertIndex + 2].norm = glm::vec4(c.x, c.y, c.z, 1.0F);  
        vertSphere[*VertIndex + 2].texCoord = glm::vec4(c.x, c.y, c.z, 1.0F);   
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

void createSphere(renderPrimitive* obj,int level) {
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
    
    glCreateVertexArrays(1, &obj->vao);
    glCreateBuffers(1, &obj->vbo);
    
    // equivalent to "glBufferData"
    glNamedBufferData(obj->vbo, sizeof(vertSphere), vertSphere, GL_STATIC_DRAW); 
    
    // setting vbo data structure to vao
    // parameter that is specified as "binding index"
    // probably means the index representing vbo that we are trying to connect
    glEnableVertexArrayAttrib(obj->vao, 0);
    // the 2 line below acts as one set of work
    glVertexArrayAttribBinding(obj->vao, 0, 0);
    glVertexArrayAttribFormat(obj->vao, 0, 4, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(obj->vao, 1);
    glVertexArrayAttribBinding(obj->vao, 1, 0);
    glVertexArrayAttribFormat(obj->vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(Vert, norm));

    glEnableVertexArrayAttrib(obj->vao, 2);
    glVertexArrayAttribBinding(obj->vao, 2, 0);
    glVertexArrayAttribFormat(obj->vao, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vert, texCoord));
    
    // setting vbo and ebo to created vao
    glVertexArrayVertexBuffer(obj->vao, 0, obj->vbo, 0, sizeof(Vert));
}


void createTriangle(renderPrimitive* obj)
{
    // vertices will be processed in NDC ranging[-1~1]
    // and that will be transferred into Screen-Space Coordinates
    // pos,color,texCoord
    Vert vertices[] = {
        // positions                        // colors|normal                 // texture coords
        glm::vec4( 0.0f,  0.5f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f),  // top right
        glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),  // bottom left
        glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f)   // bottom right
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3   // first triangle
    };
    
    obj->vao  = -1;
    obj->vbo  = -1;
    obj->ebo  = -1;
    obj->vert_count = ArrayCount(indices);
    
    glCreateVertexArrays(1, &obj->vao);
    glCreateBuffers(1, &obj->vbo);
    glCreateBuffers(1, &obj->ebo);
    
    // equivalent to "glBufferData"
    glNamedBufferData(obj->vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glNamedBufferData(obj->ebo, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // setting vbo data structure to vao
    // parameter that is specified as "binding index"
    // probably means the index representing vbo that we are trying to connect
    glEnableVertexArrayAttrib(obj->vao, 0);
    // the 2 line below acts as one set of work
    glVertexArrayAttribBinding(obj->vao, 0, 0);
    glVertexArrayAttribFormat(obj->vao, 0, 4, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(obj->vao, 1);
    glVertexArrayAttribBinding(obj->vao, 1, 0);
    glVertexArrayAttribFormat(obj->vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(Vert, norm));

    glEnableVertexArrayAttrib(obj->vao, 2);
    glVertexArrayAttribBinding(obj->vao, 2, 0);
    glVertexArrayAttribFormat(obj->vao, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vert, texCoord));
    
    // setting vbo and ebo to created vao
    glVertexArrayVertexBuffer(obj->vao, 0, obj->vbo, 0, sizeof(Vert));
    glVertexArrayElementBuffer(obj->vao, obj->ebo);
}

void createSquare(renderPrimitive* obj)
{
    // vertices will be processed in NDC ranging[-1~1]
    // and that will be transferred into Screen-Space Coordinates
    // pos,color,texCoord
    Vert vertices[] = {
        glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f),   // top right
        glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f),    // top left 
        glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),   // bottom left
        glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f)  // bottom right
    };
    
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    obj->vao  = -1;
    obj->vbo  = -1;
    obj->ebo  = -1;
    obj->vert_count = ArrayCount(indices);
    
    glCreateVertexArrays(1, &obj->vao);
    glCreateBuffers(1, &obj->vbo);
    glCreateBuffers(1, &obj->ebo);
    
    // equivalent to "glBufferData"
    glNamedBufferData(obj->vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glNamedBufferData(obj->ebo, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // setting vbo data structure to vao
    // parameter that is specified as "binding index"
    // probably means the index representing vbo that we are trying to connect
    glEnableVertexArrayAttrib(obj->vao, 0);
    // the 2 line below acts as one set of work
    glVertexArrayAttribBinding(obj->vao, 0, 0);
    glVertexArrayAttribFormat(obj->vao, 0, 4, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(obj->vao, 1);
    glVertexArrayAttribBinding(obj->vao, 1, 0);
    glVertexArrayAttribFormat(obj->vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(Vert, norm));

    glEnableVertexArrayAttrib(obj->vao, 2);
    glVertexArrayAttribBinding(obj->vao, 2, 0);
    glVertexArrayAttribFormat(obj->vao, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vert, texCoord));
    
    // setting vbo and ebo to created vao
    glVertexArrayVertexBuffer(obj->vao, 0, obj->vbo, 0, sizeof(Vert));
    glVertexArrayElementBuffer(obj->vao, obj->ebo);

}

void createCube(renderPrimitive* obj)
{
    Vert vertices[] = {
        glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f),  glm::vec4(0.0f,  0.0f, -1.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec4( 0.5f, -0.5f, -0.5f, 1.0f),  glm::vec4(0.0f,  0.0f, -1.0f, 1.0f),  glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec4( 0.5f,  0.5f, -0.5f, 1.0f),  glm::vec4(0.0f,  0.0f, -1.0f, 1.0f),  glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec4(-0.5f,  0.5f, -0.5f, 1.0f),  glm::vec4(0.0f,  0.0f, -1.0f, 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f),
        
        glm::vec4(-0.5f, -0.5f,  0.5f, 1.0f),  glm::vec4(0.0f,  0.0f,  1.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec4( 0.5f, -0.5f,  0.5f, 1.0f),  glm::vec4(0.0f,  0.0f,  1.0f, 1.0f),  glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec4( 0.5f,  0.5f,  0.5f, 1.0f),  glm::vec4(0.0f,  0.0f,  1.0f, 1.0f),  glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec4(-0.5f,  0.5f,  0.5f, 1.0f),  glm::vec4(0.0f,  0.0f,  1.0f, 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f),
        
        glm::vec4(-0.5f,  0.5f,  0.5f, 1.0f),  glm::vec4(-1.0f,  0.0f,  0.0f,1.0f),  glm::vec3( 1.0f, 0.0f, 0.0f),
        glm::vec4(-0.5f,  0.5f, -0.5f, 1.0f),  glm::vec4(-1.0f,  0.0f,  0.0f,1.0f),  glm::vec3( 1.0f, 1.0f, 0.0f),
        glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f),  glm::vec4(-1.0f,  0.0f,  0.0f,1.0f),  glm::vec3( 0.0f, 1.0f, 0.0f),
        glm::vec4(-0.5f, -0.5f,  0.5f, 1.0f),  glm::vec4(-1.0f,  0.0f,  0.0f,1.0f),  glm::vec3( 0.0f, 0.0f, 0.0f),
        
        glm::vec4( 0.5f,  0.5f,  0.5f, 1.0f),  glm::vec4(1.0f,  0.0f,  0.0f, 1.0f),  glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec4( 0.5f,  0.5f, -0.5f, 1.0f),  glm::vec4(1.0f,  0.0f,  0.0f, 1.0f),  glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec4( 0.5f, -0.5f, -0.5f, 1.0f),  glm::vec4(1.0f,  0.0f,  0.0f, 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec4( 0.5f, -0.5f,  0.5f, 1.0f),  glm::vec4(1.0f,  0.0f,  0.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 0.0f),
        
        glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f),  glm::vec4(0.0f, -1.0f,  0.0f, 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec4( 0.5f, -0.5f, -0.5f, 1.0f),  glm::vec4(0.0f, -1.0f,  0.0f, 1.0f),  glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec4( 0.5f, -0.5f,  0.5f, 1.0f),  glm::vec4(0.0f, -1.0f,  0.0f, 1.0f),  glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec4(-0.5f, -0.5f,  0.5f, 1.0f),  glm::vec4(0.0f, -1.0f,  0.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 0.0f),
        
        glm::vec4(-0.5f,  0.5f, -0.5f, 1.0f),  glm::vec4(0.0f,  1.0f,  0.0f, 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec4( 0.5f,  0.5f, -0.5f, 1.0f),  glm::vec4(0.0f,  1.0f,  0.0f, 1.0f),  glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec4( 0.5f,  0.5f,  0.5f, 1.0f),  glm::vec4(0.0f,  1.0f,  0.0f, 1.0f),  glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec4(-0.5f,  0.5f,  0.5f, 1.0f),  glm::vec4(0.0f,  1.0f,  0.0f, 1.0f),  glm::vec3(0.0f, 0.0f, 0.0f)
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
    obj->vert_count = ArrayCount(indices);
    
    glCreateVertexArrays(1, &obj->vao);
    glCreateBuffers(1, &obj->vbo);
    glCreateBuffers(1, &obj->ebo);
    
    // equivalent to "glBufferData"
    glNamedBufferData(obj->vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glNamedBufferData(obj->ebo, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // setting vbo data structure to vao
    // parameter that is specified as "binding index"
    // probably means the index representing vbo that we are trying to connect
    glEnableVertexArrayAttrib(obj->vao, 0);
    // the 2 line below acts as one set of work
    glVertexArrayAttribBinding(obj->vao, 0, 0);
    glVertexArrayAttribFormat(obj->vao, 0, 4, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(obj->vao, 1);
    glVertexArrayAttribBinding(obj->vao, 1, 0);
    glVertexArrayAttribFormat(obj->vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(Vert, norm));

    glEnableVertexArrayAttrib(obj->vao, 2);
    glVertexArrayAttribBinding(obj->vao, 2, 0);
    glVertexArrayAttribFormat(obj->vao, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vert, texCoord));
    
    // setting vbo and ebo to created vao
    glVertexArrayVertexBuffer(obj->vao, 0, obj->vbo, 0, sizeof(Vert));
    glVertexArrayElementBuffer(obj->vao, obj->ebo);
}

void createSpriteAnim(renderPrimitive* obj)
{
   // vertices & indices
    float vertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    obj->vao  = -1;
    obj->vbo  = -1;
    obj->ebo  = -1;
    obj->vert_count = ArrayCount(indices);
    
    glCreateVertexArrays(1, &obj->vao);
    glCreateBuffers(1, &obj->vbo);
    glCreateBuffers(1, &obj->ebo);
    
    // equivalent to "glBufferData"
    glNamedBufferData(obj->vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glNamedBufferData(obj->ebo, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // setting vbo data structure to vao
    // parameter that is specified as "binding index"
    // probably means the index representing vbo that we are trying to connect
    glEnableVertexArrayAttrib(obj->vao, 0);
    // the 2 line below acts as one set of work
    glVertexArrayAttribBinding(obj->vao, 0, 0);
    glVertexArrayAttribFormat(obj->vao, 0, 2, GL_FLOAT, GL_FALSE, 0);

    // setting vbo and ebo to created vao
    glVertexArrayVertexBuffer(obj->vao, 0, obj->vbo, 0, 2*sizeof(float));
    glVertexArrayElementBuffer(obj->vao, obj->ebo);
}

void bindTextures(renderPrimitive* obj){
    if (obj->textureCount > 31)
        return;
    TextureData* texture = obj->texData;
    unsigned int loopCount = obj->textureCount;

    int textureUnit = 0;
    if (obj->material.isValid){
        texture = obj->material.textures;
    }
    
    for (unsigned int i = 0; i < loopCount; ++i){
        glBindTextureUnit(textureUnit, texture[i].texture);
        textureUnit+=1;
    }
}

void setSingleTexture(renderPrimitive* obj, TextureData* textureTarget, 
                      int textureType, int wrap_s, int wrap_t, 
                      int minFileter, int magFilter, 
                      const char* texturePath,
                      const char* uniformName, unsigned short textureUnit)
{
    if (!obj->material.isValid && !obj->textureAllocated){
        obj->textureAllocated = true;
        textureTarget = (TextureData*)malloc(sizeof(TextureData));
    }  

    glCreateTextures(GL_TEXTURE_2D, 1, &textureTarget->texture);

    glTextureParameteri(textureTarget->texture, 
                        GL_TEXTURE_WRAP_S, wrap_s);
    glTextureParameteri(textureTarget->texture, 
                        GL_TEXTURE_WRAP_T, wrap_t);
    glTextureParameteri(textureTarget->texture, 
                        GL_TEXTURE_MIN_FILTER, minFileter);
    glTextureParameteri(textureTarget->texture, 
                        GL_TEXTURE_MAG_FILTER, magFilter);

    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(texturePath, 
                                    &textureTarget->tex_width, 
                                    &textureTarget->tex_height, 
                                    &textureTarget->nrChannels, 0);
    if (data){
        // for now we only store the original imageFormat and render that
        // exact thing. no fancy tricks here.
        int imageType = -1;
        switch (textureTarget->nrChannels){
            default: break;
            case 1: imageType = GL_RED; break;
            case 2: imageType = GL_RG;  break;
            case 3: imageType = GL_RGB; break;
            case 4: imageType = GL_RGBA; break;
        }
        glTextureStorage2D(textureTarget->texture, 1, GL_RGBA8, 
                           textureTarget->tex_width, textureTarget->tex_height);

        glTextureSubImage2D(textureTarget->texture, 0,
                            0, 0, 
                            textureTarget->tex_width, 
                            textureTarget->tex_height, 
                            imageType, GL_UNSIGNED_BYTE, 
                            data);
        glGenerateTextureMipmap(textureTarget->texture);
    }
    else{
        std::cout << "failed to load image" << std::endl;
    }
    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(false);
    // TODO: This part is actually more toward shader setting function
    glUseProgram(obj->shader_id);
    glUniform1i(glGetUniformLocation(obj->shader_id, uniformName), textureUnit);
}

void setTextures(renderPrimitive* obj, unsigned int textureCount, 
                 int textureType, int wrap_s, int wrap_t, 
                 int minFileter, int magFilter,
                 const char** texturePath, const char** uniformName)
{
    // NOTE: (Ericlim73) For now renderPrimitive can only use
    //                   either texture or material
    if (obj->material.isValid)
        return;
    if (textureCount > 31)  // hard limit of opengl
        return;
    // instantiate textures -> TODO: FIND A WAY TO ELEGANTLY CLEAN THIS SHIT UP
    // TODO: (EricLim73) Learn to implement Arena Allocator QUICK!!!!!!
    if (!obj->textureAllocated)
        obj->textureAllocated = true;
    
    obj->textureCount = textureCount;
    obj->texData = (TextureData*)malloc(sizeof(TextureData) * obj->textureCount);
    for (unsigned int i = 0; i < obj->textureCount; ++i)
    {   
        obj->texData[i] = {};
        TextureData* texture = &obj->texData[i];
        setSingleTexture(obj, texture, 
                         textureType, wrap_s, wrap_t, 
                         minFileter, magFilter, 
                         texturePath[i], uniformName[i], i);
    }
}

void setMaterials(renderPrimitive* obj, unsigned int textureCount,
                  int textureType, int wrap_s, int wrap_t, 
                  int minFileter, int magFilter,
                  const char** texturePath, const char** uniformName)
{   
    // NOTE: (Ericlim73) For now renderPrimitive can only use
    //                   either texture or material
    if (obj->textureAllocated)
        return;
    obj->material.isValid = true;
    obj->textureCount = textureCount;
    for (unsigned int i = 0; i < textureCount; ++i)
    {   
        TextureData* texID = &obj->material.textures[i];
        setSingleTexture(obj, texID, 
                         textureType, wrap_s, wrap_t, 
                         minFileter, magFilter, 
                         texturePath[i], uniformName[i], i);
    }
}

// NOTE:  (EricLim73) This uses only the "diffuse" part of the material for now.
//                    I don't know if specular or other material cubemap texture for
//                    this....
void setCubeMapTexture(renderPrimitive* obj, TextureData* textureTarget, 
                       const char* uniformName,
                       int wrap_s, int wrap_t,int wrap_r, 
                       int minFileter, int magFilter,
                       const char* positiveX, const char* negativeX,
                       const char* positiveY, const char* negativeY,
                       const char* positiveZ, const char* negativeZ)
{
    // TODO: (Ericlim73) make it so that we can take multiple image
    //        and combine them as single texture
    if (obj->textureAllocated)
        return;
    obj->material.isValid = true;
    obj->textureCount = 1;

    // TODO: (EricLim73) Take only the dest path, loop it,
    // and concat the "-X" part for 6 times. I think it doesn't
    // make any performance boost but at least that looks clean (this is a bad idea)
    unsigned char *cubePX = stbi_load(positiveX, 
                                    &textureTarget->tex_width, &textureTarget->tex_height, 
                                    &textureTarget->nrChannels, 0);
    unsigned char *cubeNX = stbi_load(negativeX, 
                                    &textureTarget->tex_width, &textureTarget->tex_height, 
                                    &textureTarget->nrChannels, 0);
    unsigned char *cubePY = stbi_load(positiveY, 
                                    &textureTarget->tex_width, &textureTarget->tex_height, 
                                    &textureTarget->nrChannels, 0);
    unsigned char *cubeNY = stbi_load(negativeY, 
                                    &textureTarget->tex_width, &textureTarget->tex_height, 
                                    &textureTarget->nrChannels, 0);
    unsigned char *cubePZ = stbi_load(positiveZ, 
                                    &textureTarget->tex_width, &textureTarget->tex_height, 
                                    &textureTarget->nrChannels, 0);
    unsigned char *cubeNZ = stbi_load(negativeZ, 
                                    &textureTarget->tex_width, &textureTarget->tex_height, 
                                    &textureTarget->nrChannels, 0);

    int imageType = -1;
    switch (textureTarget->nrChannels){
        default: break;
        case 1: imageType = GL_RED; break;
        case 2: imageType = GL_RG;  break;
        case 3: imageType = GL_RGB; break;
        case 4: imageType = GL_RGBA; break;
    }
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureTarget->texture);

    glTextureParameteri(textureTarget->texture, 
                        GL_TEXTURE_MIN_FILTER, minFileter);
    glTextureParameteri(textureTarget->texture, 
                        GL_TEXTURE_MAG_FILTER, magFilter);
    glTextureParameteri(textureTarget->texture, 
                        GL_TEXTURE_WRAP_S, wrap_s);
    glTextureParameteri(textureTarget->texture, 
                        GL_TEXTURE_WRAP_T, wrap_t);
    glTextureParameteri(textureTarget->texture, 
                        GL_TEXTURE_WRAP_R, wrap_r);

    glTextureStorage2D(textureTarget->texture, 
                       1, GL_RGBA8, 
                       textureTarget->tex_width, 
                       textureTarget->tex_height);

	glTextureSubImage3D(textureTarget->texture, 0, 
                        0, 0, 0, 
                        textureTarget->tex_width, 
                        textureTarget->tex_height, 
                        1, imageType, GL_UNSIGNED_BYTE,
                        cubePX);
	glTextureSubImage3D(textureTarget->texture, 0, 
                        0, 0, 1, 
                        textureTarget->tex_width, 
                        textureTarget->tex_height, 
                        1, imageType, GL_UNSIGNED_BYTE, 
                        cubeNX);
	glTextureSubImage3D(textureTarget->texture, 0, 
                        0, 0, 2, 
                        textureTarget->tex_width, 
                        textureTarget->tex_height, 
                        1, imageType, GL_UNSIGNED_BYTE,
                        cubePY);
	glTextureSubImage3D(textureTarget->texture, 0, 
                        0, 0, 3, 
                        textureTarget->tex_width, 
                        textureTarget->tex_height, 
                        1, imageType, GL_UNSIGNED_BYTE,
                        cubeNY);
	glTextureSubImage3D(textureTarget->texture, 0, 
                        0, 0, 4, 
                        textureTarget->tex_width, 
                        textureTarget->tex_height, 
                        1, imageType, GL_UNSIGNED_BYTE,
                        cubePZ);
	glTextureSubImage3D(textureTarget->texture, 0, 
                        0, 0, 5, 
                        textureTarget->tex_width, 
                        textureTarget->tex_height, 
                        1, imageType, GL_UNSIGNED_BYTE,
                        cubeNZ);
	glGenerateTextureMipmap(textureTarget->texture);

    stbi_image_free(cubePX);
    stbi_image_free(cubeNX);
    stbi_image_free(cubePY);
    stbi_image_free(cubeNY);
    stbi_image_free(cubePZ);
    stbi_image_free(cubeNZ);

    glUseProgram(obj->shader_id);
    glUniform1i(glGetUniformLocation(obj->shader_id, uniformName), 0);
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
 