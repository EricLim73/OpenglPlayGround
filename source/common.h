
#include <iostream>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h> 
#define _USE_MATH_DEFINES
#include <math.h>

// NOTE(EricLim): DO RIGOROUS BRACKETS when defining macro
// Remind that all this does is swapping out the code that
// called this macro to what is defined bellow.
#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

#define SPEED 0.005f

int windowWidth = 960;
int windowHeight = 720;
bool AspectRatioLocked = true;

struct Mouse{
    glm::vec2 pos;          // constant update through cursorPos callback event
    glm::vec2 dragStart;    // only update when mouse_btn_1 is pressed (testing for now)

    union{
        bool buttonPressed[5];
        struct{
            bool button_1;
            bool button_2;
            bool button_3;
            bool button_4;
            bool button_5;
        };
    };
};

struct windowTransform{
    int pos_x;
    int pos_y;
    int width;
    int height;
    //NOTE: This should stay the way it first gets inited by main()
    float aspectRatio;
};

struct ColorValue{
    float R;
    float G;
    float B;
};

void SetupAspectRatioLock(GLFWwindow* window, 
						  windowTransform* RenderArea,
                          ColorValue AspectR_col);

inline void setupColorValue(ColorValue* cl, 
                            float r, float g, float b)
{
    cl->R = r;
    cl->G = g;
    cl->B = b;
}

inline void setupWindowTransfrom(windowTransform* wt,
                                 int posx, int posy,
                                 int width, int height)
{
    wt->pos_x  = posx;
    wt->pos_y = posy;
    wt->width = width;
    wt->height = height;
}