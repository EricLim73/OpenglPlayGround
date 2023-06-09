#define SPHERE_SUBDIVISION_LEVEL 4
struct Vert{
    glm::vec4 pos;
    glm::vec4 norm;
    glm::vec4 color;
    glm::vec2 texCoord;
};

// TODO: seperate texture part
//       add indices count
struct render_obj{
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int shader_id;
    unsigned int vert_count;

    unsigned int texture;
    int tex_width;
    int tex_height;
    int nrChannels;
};

struct spriteFrameData{
    double timeOffset;          // for storing timing for animation
    float uv_x;
    float uv_y;
    unsigned short nx_frames;   // 0~65,535
    unsigned short ny_frames;   // 0~65,535
    unsigned short frames_ps;
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


struct Light {
	glm::vec4 pos;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	glm::vec4 att;
};

Light light = {
	{ -3.0f, 3.0f, 3.0f, 1.0f },
	{ 0.2f, 0.2f, 0.2f, 1.0f },
	{ 4.0f, 4.0f, 4.0f, 1.0f },
	{ 7.0f, 7.0f, 7.0f, 1.0f },
	{ 1.0f, 0.2f, 0.2f, 1.0f },
};

struct Material {
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float shininess;
};

Material material = {
	{ 0.24725f, 0.1995f, 0.0745f, 1.0f },
	{ 0.75164f, 0.60648f, 0.22648f, 1.0f },
	{ 0.628281, 0.555802f, 0.366065f, 1.0f },
	0.4f*32.0f
};

unsigned int CreateShaderProgram(const char* vertShaderPath, 
                                 const char* fragShaderPath);

void setCamera( Camera* cam, ProjectionMode mode, 
                glm::vec3* pos, glm::vec3* front, glm::vec3* up, 
                float near, float far, float fov );
void CameraMovement(GLFWwindow* window, Camera* cam, float deltaTime);
void CameraRotation(GLFWwindow* window, Camera* cam, Mouse* mouse, float xpos, float ypos); 
void CameraZoom(Camera* cam, float zoom);

void setDefaultMVPShader(unsigned int* shader_id, 
                         const GLfloat *model, const GLfloat *ViewProj);

// TODO: Because it uses scissor to acheive the effect, when done first
//      objects that drawCalled later that will cover up the minimap...
//      Need to find another way of doing this
void startRenderMiniMap(windowTransform* wt, ColorValue* cl);
void endRenderMiniMap(GLFWwindow* window);

void createSpriteAnim(render_obj* obj);
void drawSpriteAnim(render_obj* obj, 
                    spriteFrameData* spriteFrameInfo, 
                    float x_dir, float y_dir);
void runSpriteAnim(spriteFrameData* spriteFrameInfo);

void drawObj(render_obj* obj);
void createTriangle(render_obj* obj);
void createSquare(render_obj* obj); 
void createCube(render_obj* obj); 
void setTexture(render_obj* obj, 
                int textureType, int wrap_s, int wrap_t, 
                int minFileter, int magFilter, 
                const char* texturePath);


// NOTE: inline functions (mostly helper functions)
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

// NOTE: reset viewport & scissor for current window size
inline void resetRenderArea(GLFWwindow* window, windowTransform* wt){
    glScissor(wt->pos_x, wt->pos_y, wt->width, wt->height);
    glDisable(GL_SCISSOR_TEST);
    glViewport(wt->pos_x, wt->pos_y, wt->width, wt->height);
}

inline void setMaterial(Material* mat,
                        glm::vec4 ambient, 
                        glm::vec4 diffuse, 
                        glm::vec4 specular, 
                        float shininess)
{
    mat->ambient = ambient;
    mat->diffuse = diffuse;
    mat->specular = specular;
    mat->shininess = shininess;
}

inline void setLight(Light* light,
                     glm::vec4 pos, 
                     glm::vec4 ambient, 
                     glm::vec4 diffuse, 
                     glm::vec4 specular,
                     glm::vec4 att)
{
    light->pos = pos;
    light->ambient = ambient;
    light->diffuse = diffuse;
    light->specular = specular;
    light->att = att;
}