 #define SPHERE_SUBDIVISION_LEVEL 4

struct Vert{
    glm::vec4 pos;
    glm::vec4 norm;
    // NOTE: (EricLim73) based on my almost non-existing braincell
    //       i think i heard that when passing info to shaders(gpu)
    //       it has specific alignment rules that it follows and 
    //       in that rule "vec3" is treated as "vec4" 
    glm::vec3 texCoord; 
};

enum class ProjectionMode : unsigned short{
    PERSPECTIVE,
    ORTHOGONAL,
    END
};

struct Camera{
    glm::mat4 view;
    glm::mat4 proj;  
    glm::mat4 ViewProj;
      
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;

    float fov;
    float yaw;
    float pitch;
    float row;
    float speed;
    float sensitivity;

    ProjectionMode mode;
};

struct TextureData{  
    unsigned int texture;
    int tex_width;
    int tex_height;
    int nrChannels; 
};
// light parameter
struct Light2 {
    glm::vec3 position { glm::vec3(0.0f, 0.0f, 1.0f) };
    glm::vec3 ambient { glm::vec3(0.1f, 0.1f, 0.1f) };
    glm::vec3 diffuse { glm::vec3(0.5f, 0.5f, 0.5f) };
    glm::vec3 specular { glm::vec3(1.0f, 1.0f, 1.0f) };
};
Light2 m_light;

// material parameter
struct Material2 {
    union{
        TextureData textures[3];
        struct{
            TextureData diffuse;
            TextureData specular;
            TextureData emission;
        };
    };
    float shininess { 7.0f };
    bool isValid;
    // NOTE: (EricLim73) In no way this should go over 32
}; 

// NOTE: (Ericlim73) "renderPrimitive" will work as base data bank for supported primitive shapes
//                    Each render object will reference this primitive info to have shape. 
//                    Not 100% mesh but sort of. 
//                    This primitive shape already acts as a model of sort.
//                    Thought about SoA and wanted to try it for fun. BUT i can't think of any possibilities
//                    for using SIMD on rendering (plus binding textures makes it little hard for my smooth brain)
//                    so i will use SoA for game obejct looping and stick with AoS for this one
struct renderPrimitive{
    TextureData* texData;
    Material2 material;
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int shader_id;
    unsigned int vert_count;
    unsigned int textureCount;
    bool textureAllocated;
};

struct spriteFrameData{
    double timeOffset;          // for storing timing for animation
    float uv_x;
    float uv_y;
    unsigned short nx_frames;   // 0~65,535
    unsigned short ny_frames;   // 0~65,535
    unsigned short frames_ps;
};

//~ Functions

// Shader Creation //
unsigned int CreateShaderProgram(const char* vertShaderPath, 
                                 const char* fragShaderPath);

// Camera //
void setCamera( Camera* cam, ProjectionMode mode, 
                glm::vec3* pos, glm::vec3* front, glm::vec3* up, 
                float near, float far, float fov );
void CameraMovement(GLFWwindow* window, Camera* cam, float deltaTime);
void CameraRotation(GLFWwindow* window, Camera* cam, Mouse* mouse, float xpos, float ypos); 
void CameraZoom(Camera* cam, float zoom);

// Shader Uniform Setting Function //
void setDefaultMVPShader(unsigned int* shader_id, 
                         Camera* cam,
                         glm::mat4* model);
// Sprite Render Functions //
void createSpriteAnim(renderPrimitive* obj);


// Render Function //
void drawObj(renderPrimitive* obj);

// Generate Mesh //
void createTriangle(renderPrimitive* obj);
void createSquare(renderPrimitive* obj); 
void createCube(renderPrimitive* obj); 
void generateSphere(renderPrimitive* obj, int level);

// TEXTURE //
void setTextures(renderPrimitive* obj, unsigned int textureCount, 
                 int textureType, int wrap_s, int wrap_t, 
                 int minFileter, int magFilter,
                 const char** texturePath, const char** uniformName);
void bindTextures(renderPrimitive* obj);

void setCubeMapTexture(renderPrimitive* obj, TextureData* targetTexture, 
                       const char* uniformName,
                       int wrap_s, int wrap_t,int wrap_r, 
                       int minFileter, int magFilter,
                       const char* positiveX, const char* negativeX,
                       const char* positiveY, const char* negativeY,
                       const char* positiveZ, const char* negativeZ);



// Inline Functions // 
inline void updateProj(Camera* cam, float near, float far){
    if (cam->mode == ProjectionMode::PERSPECTIVE)
    {
        cam->proj = glm::perspective(glm::radians(cam->fov), (float)(windowWidth / windowHeight), near, far);
    }
    if (cam->mode == ProjectionMode::ORTHOGONAL)
    {
        cam->proj = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight, near, far);
    }
}

// NOTE:  (EricLim73) reset viewport & scissor for current window size
inline void resetRenderArea(GLFWwindow* window, windowTransform* wt){
    glScissor(wt->pos_x, wt->pos_y, wt->width, wt->height);
    glDisable(GL_SCISSOR_TEST);
    glViewport(wt->pos_x, wt->pos_y, wt->width, wt->height);
}

inline void cleanupAllocatedTexture(renderPrimitive* obj){
    if (obj->texData){
        free(obj->texData);
    }
}