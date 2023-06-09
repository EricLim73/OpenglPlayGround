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


/*  NOTE: Error solution report -> gets deleted after commit
---------------------------------------------------------------------------------------------------
    [Type]: GL_DEBUG_TYPE_ERROR
    [Source]: GL_DEBUG_SOURCE_API
    [ID]: 1282
    [Severity]: GL_DEBUG_SEVERITY_HIGH
    [Message]: GL_INVALID_OPERATION error generated. Object is not a program or shader object.

        NOTE: [Problem Solved] 
              glGetShaderiv() -> glGetProgramiv() inside CreateShaderProgram() was the answer.
              I was just using the wrong function
---------------------------------------------------------------------------------------------------
    [Type]: GL_DEBUG_TYPE_PERFORMANCE
    [Source]: GL_DEBUG_SOURCE_API
    [ID]: 131218
    [Severity]: GL_DEBUG_SEVERITY_MEDIUM
    [Message]: Program/shader state performance warning: Vertex shader in program 5 
               is being recompiled based on GL state.
                                &&&&
    [Type]: GL_DEBUG_TYPE_PERFORMANCE
    [Source]: GL_DEBUG_SOURCE_API
    [ID]: 131218
    [Severity]: GL_DEBUG_SEVERITY_MEDIUM
    [Message]: Program/shader state performance warning: Vertex shader in program 5 
               is being recompiled based on GL state, and was not found in the disk cache
        
        NOTE: [Problem Solved]   
              Hard time finding where it happened. First i thought it was 
              glUseProgram() & glUniform1i() inside "setSingleTexture()" and
              "cubemapTexture()" bc when removing functions that use them
              the last one calling them will pop up this error (its shader id to be precise).
              When i googled through bunch of shits i found this post on reddit 
              (https://www.reddit.com/r/opengl/comments/ld18gp/opengl_debugger_complain_vertex_shader_is/)       

              Two solutions. The first one is...
              (quote)
              You have two options to solve this: 
              first is to track down which state change is triggering this and avoid it. 
              The second is to figure out all combinations of shaders and states you need, 
              and draw one triangle using every combination. The second option is what 
              (some) commercial engines do (especially on mobile, where state based recompiles 
              are more common). It's also how Vulkan works.

              The second on (which worked for me) is...
              (quote)
              According to https://www.khronos.org/opengl/wiki/Shader_Compilation#Program_setup
              Before Linking:
                A number of parameters can be set up that will affect the linkingprocess. 
                This generally involves interfaces with the program. Theseinclude:
                    ...
                    Vertex shader input attribute locations
                    ...
              So when I compiled and linked my shaders, my VAO was unbound. 
              Therefore the shaders input attribute locations were not enabled. 
              By just enabling location with "glEnableVertexAttribArray(location)" 
              before linking the warning disappeared.
             
             SO most of my shader uses 0,1,2 for [vertex pos, norm, texCoord(*this one is the culprit imo)],
             i added glEnableVertexAttribArray(0), glEnableVertexAttribArray(1), glEnableVertexAttribArray(2)
             before linking shader inside "CreateShaderProgram()" and it fixed the problem
*/
  

// TODO: (EricLim73) Fix lighting for god sakes
// TODO: (EricLim73) Refactor render flow to more flexible format
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

void message_callback(GLenum src,GLenum type,GLuint id,GLenum severity,
                      GLsizei length, GLchar const* msg, void const* user_param);

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
    if (enableVsync)
        glfwSwapInterval(1); // Enable vsync

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

//---------------OpenGL option setup
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

    glEnable(GL_DEBUG_OUTPUT);
    // print out immediately (default == buffered)
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    // disable notification message
    glDebugMessageControl(GL_DONT_CARE, 
                          GL_DONT_CARE, 
                          GL_DEBUG_SEVERITY_NOTIFICATION, 
                          0, nullptr, GL_FALSE);
    glDebugMessageCallback(message_callback, nullptr);

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
    ColorValue DefaultCL = {0.1f, 0.1f, 0.3f};

    windowTransform RenderArea = {};
    RenderArea.aspectRatio = (float)windowWidth/(float)windowHeight;


//---------------Shader setup 
    unsigned int simpleShader = CreateShaderProgram("./shaders/simple.vs", 
                                                    "./shaders/simple.fs");
    unsigned int simpleTexShader = CreateShaderProgram("./shaders/simpleTex.vs", 
                                                       "./shaders/simpleTex.fs");

    unsigned int spriteAnime = CreateShaderProgram("./shaders/spriteAnim.vs", 
                                                   "./shaders/spriteAnim.fs");    

    unsigned int cubemapTex = CreateShaderProgram("./shaders/cubemapTex.vs", 
                                                  "./shaders/cubemapTex.fs");

    unsigned int lightingShader = CreateShaderProgram("./shaders/lighting.vs", 
                                                      "./shaders/lighting.fs");
//---------------Object setup (mesh + some of them texture)   
    Light directionalLight = {}; 
    setUpDirectionLight(&directionalLight,
                    glm::vec3(-0.2f, -1.0f, -0.3f),
                    glm::vec3(0.2f, 0.2f, 0.2f), 
                    glm::vec3(0.5f, 0.5f, 0.5f),
                    glm::vec3(1.0f, 1.0f, 1.0f));
    
    Light pointLight = {}; 
    setUpPointLight(&pointLight,
                    glm::vec3(2.0f, 2.0f, 2.0f),
                    glm::vec3(0.1f, 0.1f, 0.1f), 
                    glm::vec3(0.5f, 0.5f, 0.5f),
                    glm::vec3(1.0f, 1.0f, 1.0f),
                    128.0f);
        
    Light spotLight = {}; 
    setUpSpotLight(&spotLight,
                   cam->cameraPos,
                   cam->cameraFront,
                   glm::vec3(0.2f, 0.2f, 0.2f),
                   glm::vec3(0.5f, 0.5f, 0.5f),
                   glm::vec3(1.0f, 1.0f, 1.0f),
                   glm::vec2(10.0f, 2.0f),
                   32.0f);

    renderPrimitive plane = {};
    plane.shader_id = simpleTexShader;
    createSquare(&plane);
    const char* groundTex[] = {"resources/textures/awesomeface.png", "resources/textures/wall.jpg"};
    const char* groundTexUN[] = {"Tex1", "Tex2"};
    createSquare(&plane);
    setTextures(&plane, ArrayCount(groundTex),
                GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT,
                GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST,
                groundTex, groundTexUN);

    renderPrimitive globe = {};
    globe.shader_id = cubemapTex;
    globe.material.shininess = 7.0f;
    createSphere(&globe, SPHERE_SUBDIVISION_LEVEL);
    setCubeMapTexture(&globe, &globe.material.textures[0], "material.diffuse",
                      GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
                      GL_LINEAR, GL_LINEAR,
                      "resources/cubemap/earth/earth-map-0.png",
                      "resources/cubemap/earth/earth-map-1.png",
                      "resources/cubemap/earth/earth-map-2.png",
                      "resources/cubemap/earth/earth-map-3.png",
                      "resources/cubemap/earth/earth-map-4.png",
                      "resources/cubemap/earth/earth-map-5.png");

    renderPrimitive testLight = {};
    createCube(&testLight);
    testLight.shader_id = simpleShader;

    renderPrimitive materialContainer = {};
    materialContainer.shader_id = lightingShader;
    createCube(&materialContainer);
    const char* textures[] ={"resources/textures/container2.png", 
                             "resources/textures/container2_specular.png", 
                             "resources/textures/matrix.jpg"};
    const char* uniformNames[] = {"material.diffuse", 
                                  "material.specular", 
                                  "material.emission"};
    setMaterials(&materialContainer, ArrayCount(textures),
                GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT, 
                GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR,
                textures, uniformNames);    

    renderPrimitive spriteMan = {};
    spriteMan.shader_id = spriteAnime;
    const char* sprites[] = {"resources/textures/Dancer_walk-Sheet.png"};
    const char* spriteUniformName[] = {"Tex"};
    createSpriteAnim(&spriteMan);
    setTextures(&spriteMan, 1,
                GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT,
                GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST,
                sprites, spriteUniformName);

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
        
        if (AspectRatioLocked){
            ColorValue AspectWindowCol = {};
            setupColorValue(&AspectWindowCol, 0.0f, 0.0f, 0.0f);
            SetupAspectRatioLock(window, &RenderArea, AspectWindowCol);
        }
    //---------------ACTAUL RENDER CALLS            
            Light* lightCaster = &spotLight;   
            
            glUseProgram(testLight.shader_id);
            glm::mat4 modelLight = glm::translate(glm::mat4(1.0), lightCaster->position) * 
                                   glm::scale(glm::mat4(1.0), glm::vec3(0.1f)); 
            setDefaultMVPShader(&testLight.shader_id, cam, &modelLight);
            if (lightCaster->type == LightType::POINT_LIGHT)
                drawObj(&testLight);

            if (lightCaster->type == LightType::SPOT_LIGHT){
                lightCaster->position = cam->cameraPos;
                lightCaster->direction = cam->cameraFront;
            }

            glm::mat4 model = glm::mat4(1.0f);
            for (int i = 0; i < 10; ++i)
            {
                model = glm::rotate(glm::mat4(1.0f), 
                                   (float)i*glm::radians(15.0f), 
                                   glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::translate(model, glm::vec3(-2.0f+(float)i, 
                                                        0.0f-(float)i+1, 
                                                        0.0f+(float)i+2));
                setLightShaderParameter(&materialContainer.shader_id, 
                                        cam, &materialContainer.material, 
                                        lightCaster, &model);
                drawObj(&materialContainer);
            }

            glm::mat4 globeModel = glm::rotate(glm::mat4(1.0f),  
                                               (float)glfwGetTime() * glm::radians(15.0f),
                                               glm::vec3(0.0f, 1.0f, 0.0f));
            globeModel = glm::translate(globeModel, glm::vec3(5.0f, 0.0f, 0.0f));
            globeModel = glm::rotate(glm::mat4(1.0f), 
                                    (float)glfwGetTime() * glm::radians(30.0f), 
                                    glm::vec3(0.0f, 1.0f, 0.0f));
            setLightShaderParameter(&globe.shader_id, cam, &globe.material, lightCaster, &globeModel);            
            drawObj(&globe);  

            runSpriteAnim(&frame);
            setSpriteUniform(&spriteMan, &frame, x_dir, y_dir);
            drawObj(&spriteMan);  

            glm::mat4 groundMat = glm::mat4(1.0f);
            groundMat = glm::translate(groundMat, glm::vec3(2.0f, 1.0f, 0.0f));
            setDefaultMVPShader(&plane.shader_id, cam, &groundMat);
            drawObj(&plane);  


    //---------------Swapbuffer & event polling
        glfwSwapBuffers(window);
        glfwPollEvents();
        deltaTime = (float)glfwGetTime() - lastFrame;
        lastFrame = (float)glfwGetTime(); 
    }

    cleanupAllocatedTexture(&globe);
    cleanupAllocatedTexture(&materialContainer);
    cleanupAllocatedTexture(&testLight);
    cleanupAllocatedTexture(&spriteMan);
    cleanupAllocatedTexture(&plane);


    glfwTerminate();
    return 0;
} 

//---------------MAIN CALLBACKS & UTILITY FUNCTION
// TODO: potentially problematic when the values defined gets updated and so on
//       but its good for now i guess...? at least i practiced to avoid branching for 
//       something that does not need branching so thats a win for me
void message_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
                      GLsizei length, GLchar const* msg, void const* user_param)
{
    // NOTE: (Ericlim73) All the "GL_DEBUG_SOURCE" that i can find inside "glad.h"
    const char* sourceValue[] = {"GL_DEBUG_SOURCE_API", 
                                 "GL_DEBUG_SOURCE_WINDOW_SYSTEM", 
                                 "GL_DEBUG_SOURCE_SHADER_COMPILER", 
                                 "GL_DEBUG_SOURCE_THIRD_PARTY", 
                                 "GL_DEBUG_SOURCE_APPLICATION", 
                                 "GL_DEBUG_SOURCE_OTHER"
                                };
    // NOTE: (Ericlim73) All the "GL_DEBUG_TYPE" that i can find inside "glad.h"
    const char* typeValue[] = {"GL_DEBUG_TYPE_ERROR", 
                               "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR", 
                               "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR", 
                               "GL_DEBUG_TYPE_PORTABILITY", 
                               "GL_DEBUG_TYPE_PERFORMANCE", 
                               "GL_DEBUG_TYPE_OTHER",     // 0x8251
                               "GL_DEBUG_TYPE_MARKER",    // 0x8268
                               "GL_DEBUG_TYPE_PUSH_GROUP",
                               "GL_DEBUG_TYPE_POP_GROUP"
                                };
    // NOTE: (Ericlim73) All the "GL_DEBUG_SEVERITY" that i can find inside "glad.h"
    const char* severityValue[] = {"GL_DEBUG_SEVERITY_HIGH",        // 0x9146
                                   "GL_DEBUG_SEVERITY_MEDIUM", 
                                   "GL_DEBUG_SEVERITY_LOW", 
                                   "GL_DEBUG_SEVERITY_NOTIFICATION" // 0x826B
                                   }; 

    unsigned int srcOffset = GL_DEBUG_SOURCE_API;
    unsigned int typeOffset = GL_DEBUG_TYPE_ERROR;
    unsigned int servOffset = GL_DEBUG_SEVERITY_HIGH;
    int sourceIndex = src  - srcOffset;
    int typeIndex   = type - typeOffset;
    int servIndex   = severity - servOffset;
    // if type goes over "OTHER" recalculate
    if (typeIndex > 5) {typeIndex = 5 + type - GL_DEBUG_TYPE_MARKER;}
    // if severity goes below "HIGH" recalculate
    if (servIndex < 0) {servIndex = 3;}

    printf("[Type]: %s\n[Source]: %s\n[ID]: %d\n[Severity]: %s\n",
            typeValue[typeIndex], sourceValue[sourceIndex], id, severityValue[servIndex]);
    printf("[Message]: %s\n\n", msg);
    fflush(stdout);
}

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
    if (key == GLFW_KEY_Y && 
        action == GLFW_PRESS && 
        mods == GLFW_MOD_CONTROL)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
    }
    if (key == GLFW_KEY_P && 
        action == GLFW_PRESS &&
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
            if (button == 0){
                context->camera.focus = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            mouse->buttonPressed[button] = true;
            glfwGetCursorPos(window, &x, &y);
            mouse->dragStart = glm::vec2((float)x, (float)y);
            break;
        }
        case GLFW_RELEASE:{
            mouse->buttonPressed[button] = false;
            if (button == 0){
                context->camera.focus = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
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