#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <GLM/geometric.hpp>
#include <GLM/gtx/quaternion.hpp>

#include "common.cpp"
#include "render.cpp"
  
// TODO: (EricLim73) see if there is any usecase for this -> maybe when dealing with more
//       event per objects within context
// TODO: (EricLim73) THIS IS NOT TYPICALL "Context" THAT IS USED AROUND GAME ENGINE
//      I DIDN'T HAVE ANY OTHER OPTION TO NAME THIS STRUCT. THIS HOLDS POINTERS
//      TO THINGS THAT I NEED ACCESS FROM GLFW_CALLBACK FUNCTIONS. I DON'T NO
//      ANY OTHER WAYS OF HANDLING THE PROBLEM.
struct Context{
    Camera camera;
    Mouse mouse;
};

void framebuffer_sizeCallback(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, 
                 int key, int scancode, int action, int mods);
void mousePosCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

void readKeyboard(GLFWwindow *window, float *x_direction, float *y_direction);

int main(int argc, char** argv){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "OPJ", NULL, NULL);
    if (!window){
        std::cout << "Failed to create GLFW Window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

//---------------Callback Func
    glfwSetFramebufferSizeCallback(window, framebuffer_sizeCallback);
    glfwSetKeyCallback(window, keyCallback); 
    glfwSetCursorPosCallback(window, mousePosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);

//---------------Shader setup
    unsigned int simpleShader = -1;
    simpleShader = CreateShaderProgram("./shaders/simple.vs", 
                                       "./shaders/simple.fs");

    unsigned int simpleTexShader = -1;
    simpleTexShader = CreateShaderProgram("./shaders/simpleTex.vs",
                                           "./shaders/simpleTex.fs");
    unsigned int shaderProgram_id1 = -1;
    shaderProgram_id1 = CreateShaderProgram("./shaders/spriteAnim.vs", 
                                            "./shaders/spriteAnim.fs");
    unsigned int blinnPhongShader = -1;
    blinnPhongShader = CreateShaderProgram("./shaders/blinn-phong.vs",
                                       "./shaders/blinn-phong.fs");
    unsigned int shaderSphereCube = -1;
    shaderSphereCube = CreateShaderProgram("./shaders/cubemapTex.vs",
                                       "./shaders/cubemapTex.fs");
                                       
//---------------Object setup (mesh + some of them texture)
    render_obj simpleLight = {};
    simpleLight.shader_id = simpleShader;
    createCube(&simpleLight);

    render_obj objects[5] = {};
    objects[0].shader_id = simpleTexShader;
    createCube(&objects[0]);
    setTexture( &objects[0], 
                GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT,
                GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR,
                "resources/textures/awesomeface.png" );
 
    objects[1].shader_id = simpleTexShader;
    createTriangle(&objects[1]);
    setTexture( &objects[1], 
                GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT,
                GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR,
                "resources/textures/wall.jpg" );

    objects[2].shader_id = shaderProgram_id1;
    createSpriteAnim(&objects[2]);
    setTexture(&objects[2],
                GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT,
                GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST,
                "resources/textures/Dancer_walk-Sheet.png");

    objects[3].shader_id = blinnPhongShader;
    generateSphere(&objects[3], SPHERE_SUBDIVISION_LEVEL);
    Material mat = {};
    setMaterial(&mat,	
                glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f),
	            glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f),
	            glm::vec4(0.628281, 0.555802f, 0.366065f, 1.0f),
	            0.4f*32.0f);
    Light light = {};
    setLight(&light,
            glm::vec4(-3.0f, 3.0f, 3.0f, 1.0f),
	        glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
	        glm::vec4(4.0f, 4.0f, 4.0f, 1.0f),
	        glm::vec4(7.0f, 7.0f, 7.0f, 1.0f),
	        glm::vec4(1.0f, 0.2f, 0.2f, 1.0f));


    objects[4].shader_id = shaderSphereCube;
    generateSphere(&objects[4], SPHERE_SUBDIVISION_LEVEL);
    setCubeMapTexture(&objects[4], 
                      GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
                      GL_LINEAR, GL_LINEAR,
                      "resources/cubemap/earth/earth-map-0.png",
                      "resources/cubemap/earth/earth-map-1.png",
                      "resources/cubemap/earth/earth-map-2.png",
                      "resources/cubemap/earth/earth-map-3.png",
                      "resources/cubemap/earth/earth-map-4.png",
                      "resources/cubemap/earth/earth-map-5.png");
    Material mat1 = {};
    setMaterial(&mat1,	
                glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
	            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
	            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
	            32.0f);

//---------------Basic setup for variables used during rendering  
    float deltaTime = 0.0f;	// Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

    float x_dir = 0.0f, y_dir = 0.0f;
    spriteFrameData frame = {};
    frame.uv_x = 0.0f;
    frame.uv_y = 0.0f;
    frame.nx_frames = 8;
    frame.ny_frames = 1;
    frame.frames_ps = 8;

//---------------Context setup   
    Context context = {};    
    Camera* cam = &context.camera;
    glm::vec3 pos = glm::vec3(0.0f, 0.0f,  3.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f,  0.0f);
    setCamera(cam, ProjectionMode::PERSPECTIVE, &pos, &front, &up, 0.1f, 100.0f, 45.0f);
    Mouse* mouse = &context.mouse; 
    glfwSetCursorPos(window, (double)windowWidth/2, (double)windowHeight/2);
    mouse->pos = glm::vec2((float)(windowWidth / 2), (float)(windowHeight / 2));
    
    // TODO: (EricLim73) change this to incoporate a whole Context for future
    // dont know if thats the best way
    //~ SetFunction
    glfwSetWindowUserPointer(window, &context);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glViewport(0, 0, windowWidth, windowHeight);
    ColorValue DefaultCL = {0.0f, 0.0f, 0.0f};
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

    windowTransform RenderArea = {};
    RenderArea.aspectRatio = (float)windowWidth/(float)windowHeight;

    // NOTE:  (EricLim73) setting up miniMap config
    windowTransform miniMapWT = {};
    ColorValue miniMapColor = {};
    setupColorValue(&miniMapColor, 0.5f, 0.5f, 0.2f);

//---------------renderLoop (GameLoop)
    while(!glfwWindowShouldClose(window)){ 
        // TODO: (EricLim73) find elegant way to shove these two inside callback function
        readKeyboard(window, &x_dir, &y_dir); 
        CameraMovement(window, cam, deltaTime); 
        
    //---------------Basic render call setup
        resetRenderArea(window, &RenderArea);
        glClearColor(DefaultCL.R, DefaultCL.G, DefaultCL.B, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);   // state changing func

        // update current renderArea -> NOTE:  (EricLim73) can be done in callback function if i try
        RenderArea.pos_x = RenderArea.pos_y = 0;
        glfwGetWindowSize(window, &RenderArea.width, &RenderArea.height);

        // setting minimap to match current renderArea
        setupWindowTransfrom(&miniMapWT, 
                            (int)(windowWidth * 0.70f), (int)(windowHeight * 0.05f),
                            (int)(windowWidth * 0.25f), (int)(windowHeight * 0.25f));
        
        if (AspectRatioLocked){
            ColorValue AspectWindowCol = {};
            setupColorValue(&AspectWindowCol, 0.3f, 0.4f, 0.3f);
            SetupAspectRatioLock(window, &RenderArea, AspectWindowCol);
            setupWindowTransfrom(&miniMapWT, 
                        RenderArea.pos_x + (int)(RenderArea.width * 0.70f), 
                        RenderArea.pos_y + (int)(RenderArea.height * 0.05f),
                        (int)(RenderArea.width * 0.25f), (int)(RenderArea.height * 0.25f));
        }
    //---------------ACTAUL RENDER CALLS        

        /* 
        */
        glm::mat4 lightTrans = glm::mat4(1.0f);
        lightTrans = glm::translate(lightTrans, glm::vec3( -3.0f, 3.0f, 3.0f));
        lightTrans = glm::scale(lightTrans, glm::vec3(0.2f, 0.2f, 0.2f));
        setDefaultMVPShader(&simpleLight.shader_id, 
                            glm::value_ptr(lightTrans), 
                            glm::value_ptr(cam->ViewProj));
        drawObj(&simpleLight);

        runSpriteAnim(&frame);
        drawSpriteAnim(&objects[2], &frame, x_dir, y_dir);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(32.0f), glm::vec3(1.0f, 1.0f, 0.0f));
        setDefaultMVPShader(&objects[0].shader_id,  
                            glm::value_ptr(model), 
                            glm::value_ptr(cam->ViewProj));
        drawObj(&objects[0]);
     
        model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * glm::radians(42.0f), glm::vec3(1.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(1.0f, 0.0f, -1.0f));
        setDefaultMVPShader(&objects[1].shader_id, 
                            glm::value_ptr(model), 
                            glm::value_ptr(cam->ViewProj));
        drawObj(&objects[1]);

        glm::mat4 SphereModel = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 0.0f));
        SphereModel = glm::rotate(SphereModel, (float)glfwGetTime() * glm::radians(32.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        SphereModel = glm::scale(SphereModel, glm::vec3(0.2f, 0.5f, 0.2f));
        setBlinnPhongParameter(&objects[3].shader_id, 
                               glm::value_ptr(SphereModel), 
                               glm::value_ptr(glm::vec4(cam->cameraPos, 1.0f)), 
                               glm::value_ptr(cam->ViewProj));
        setMaterialParameter(&objects[3].shader_id, &mat);  
        setLightParameter(&objects[3].shader_id, &light);  
        drawObj(&objects[3]);
        
        SphereModel = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * glm::radians(32.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        setBlinnPhongParameter(&objects[4].shader_id, 
                               glm::value_ptr(SphereModel), 
                               glm::value_ptr(glm::vec4(cam->cameraPos, 1.0f)), 
                               glm::value_ptr(cam->ViewProj));
        setMaterialParameter(&objects[4].shader_id, &mat1);  
        setLightParameter(&objects[4].shader_id, &light);  
        drawObj(&objects[4]);
            //startRenderMiniMap(&miniMapWT, &miniMapColor);
            //endRenderMiniMap(window, &RenderArea);

    //---------------Swapbuffer & event polling
        glfwSwapBuffers(window);
        glfwPollEvents();
        deltaTime = (float)glfwGetTime() - lastFrame;
        lastFrame = (float)glfwGetTime(); 
    }

    for (int idx = 0;
        idx < ArrayCount(objects);
        ++idx)
    {
        glDeleteVertexArrays(1, &objects[idx].vao);
        glDeleteBuffers(1, &objects[idx].vbo);
        glDeleteBuffers(1, &objects[idx].ebo);
    }
    glfwTerminate();
    return 0;
} 

//---------------MAIN CALLBACKS & UTILITY FUNCTION
void framebuffer_sizeCallback(GLFWwindow* window, int width, int height)
{
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
}

void keyCallback(GLFWwindow* window, 
                 int key, int scancode, 
                 int action, int mods)
{
    Context* context = (Context*)glfwGetWindowUserPointer(window);
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_W && 
        action == GLFW_PRESS && 
        mods == GLFW_MOD_CONTROL)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
    }
    if (key == GLFW_KEY_W && 
        action == GLFW_RELEASE &&
        mods == GLFW_MOD_CONTROL)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
    }   

    if (key == GLFW_KEY_R &&
        action == GLFW_PRESS &&
        mods == GLFW_MOD_SHIFT)
    {
        CameraStatReset(&context->camera);
    }
}

void mouseButtonCallback(GLFWwindow* window, 
                         int button, int action, int mods)
{
    Context* context = (Context*)glfwGetWindowUserPointer(window);
    Mouse* mouse = &context->mouse;
    double x, y;
    switch (action)
    {
        case GLFW_PRESS:{
            mouse->buttonPressed[button] = true;
            glfwGetCursorPos(window, &x, &y);
            mouse->dragStart = glm::vec2((float)x, (float)y);
            break;
        }
        case GLFW_RELEASE:{
            mouse->buttonPressed[button] = false;
            glfwGetCursorPos(window, &x, &y);
            glm::vec2 dragCur = glm::vec2((float)x, (float)y);
            break;
        }
        default:{
            break;
        }
    }
}

void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
    Context* context = (Context*)glfwGetWindowUserPointer(window);
    Camera* cam = &context->camera;
    Mouse* mouse = &context->mouse;
    CameraRotation(window, cam, mouse, (float)xpos, (float)ypos);
    mouse->pos = glm::vec2((float)xpos, (float)ypos);
}

void mouseScrollCallback(GLFWwindow* window, 
                         double xoffset, double yoffset)
{
    Context* context = (Context*)glfwGetWindowUserPointer(window);
    Camera* cam = &context->camera;
    CameraZoom(cam, (float)yoffset);
}

// just for now, will clear this up later
void readKeyboard(GLFWwindow *window, 
                  float *x_direction, float *y_direction)
{
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        *y_direction += SPEED;
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        *y_direction -= SPEED;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        *x_direction -= SPEED;
    }
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        *x_direction += SPEED;
    }
    // TODO: (EricLim73) make clamp() macro
    if (*x_direction > 0.9f)
        *x_direction = 0.9f;
    if (*y_direction > 0.9f)
        *y_direction = 0.9f;
    if (*x_direction <= -0.9f)
        *x_direction = -0.9f;
    if (*y_direction <= -0.9f)
        *y_direction = -0.9f;
}