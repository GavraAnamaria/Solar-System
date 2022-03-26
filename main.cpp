#define GLEW_STATIC

#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types
#include "Window.h"
#include "Shader.hpp"
#include "SkyBox.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include <iostream>

gps::SkyBox mySkyBox;
bool trans = false;
// window
gps::Window myWindow;
bool stins = false;
gps::Shader lightShader;
GLfloat al;
// matrices
glm::mat4 oldView;
glm::vec3 oldCamPos;
GLfloat u = 0.0f;

glm::mat4 model;
glm::mat4 modelE;
glm::mat4 modelM;
glm::mat4 view;
glm::mat4 lightRotation;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat3 normalMatrixE;
glm::mat3 normalMatrixM;

// light parameters
glm::vec3 viewPosEye;
glm::vec3 lightColor;
glm::vec3 lightColorP;
bool animatie = false;

unsigned int shadowMapFBO;
unsigned int depthMapTexture;
bool showDepthMap = false;

float fog = 0.0f;
GLfloat dir_rotatie = 0.08f;
GLfloat constant = 1.0f;
GLfloat linear = 0.005f;
GLfloat quadratic = 0.003f;
const GLfloat near_plane = 1.0f, far_plane = 2200.0f;
int  SHADOW_WIDTH =20000, SHADOW_HEIGHT = 30000;

//glm::vec3 lightDir = glm::vec3(0.0f, 5.0f, 2000.0);
glm::vec3 lightDir = glm::vec3(0.0f, 10.0f, 2000.0); //-20.0f, 30.0f, 1.0f);
glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, -4884.0f);

glm::vec3 lightPos;
// shader uniform locations
GLint alLoc;
GLint viewPosEyeLoc;
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightDirPLoc;
GLint lightColorLoc;
GLint lightColorPLoc;
GLint lPosLoc;
GLint lPos1Loc;
GLint ctLoc;
GLint linLoc;
GLint qLoc;
GLint fogLoc;
gps::Model3D lightCube;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.7f;

GLboolean pressedKeys[1024];

glm::vec3 coordP;

// models
gps::Model3D ss;
gps::Model3D moon;
gps::Model3D earth;
GLfloat angle;
GLfloat lightAngle;

// shaders
gps::Shader depthMapShader;
gps::Shader myBasicShader;
gps::Shader SBShader;

int retina_width, retina_height;
float delta = 0;
float movementSpeed = 0.002; 
void updateDelta(double elapsedSeconds) {
    delta = delta + movementSpeed * elapsedSeconds;
}
double lastTimeStamp = glfwGetTime();

GLenum glCheckError_(const char* file, int line){
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    //if (key == GLFW_KEY_O && action == GLFW_PRESS)
      //  showDepthMap = !showDepthMap;

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

bool firstMouse = true;
float lastX = 400, lastY = 250;
float yaw = -90.0f;
float pitch = 0.0f;
float sensitivity = 0.1f;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!animatie){
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
     }
}

int mod = 0;
void processMovement() {
    if(!animatie){
    if (pressedKeys[GLFW_KEY_Q]) {
        yaw = yaw + 0.4;
        myCamera.rotate(0.0f, yaw);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    else if (pressedKeys[GLFW_KEY_E]) {
        yaw = yaw - 0.4;
        myCamera.rotate(0.0f, yaw);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    else if (pressedKeys[GLFW_KEY_M]) {
        animatie = true;
        oldView = myCamera.getViewMatrix();
        oldCamPos = myCamera.getPosition();
        u = 0;
        glm::vec3 direction;
        direction.x = cos(glm::radians(90.0f)) * cos(glm::radians(-89.0f));
        direction.y = sin(glm::radians(-89.0f));
        direction.z = sin(glm::radians(90.0f)) * cos(glm::radians(-89.0f));
        myCamera.setVal(glm::vec3(0, 70, -250), glm::normalize(direction), glm::vec3(1, 0, 0));
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    else if (pressedKeys[GLFW_KEY_K]) {
        mod = (mod + 1) % 3;
        if (mod == 0)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else if (mod == 1)
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        Sleep(300);
    }
    else if (pressedKeys[GLFW_KEY_N]) {
        if (fog == 0.0f)
            fog = 0.0025f;
        else if (fog == 0.0025f)
            fog = 0.0045f;
        else if (fog == 0.0045f)
            fog = 0.006f;
        else
            fog = 0.0f;
        glUniform1f(fogLoc, fog);
        Sleep(300);
    }

    else if (pressedKeys[GLFW_KEY_O]) {
        trans = !trans;
        Sleep(300);
    }
    
    else if (pressedKeys[GLFW_KEY_UP]) {
        cameraSpeed =cameraSpeed+0.1f;
    }
    else if (pressedKeys[GLFW_KEY_DOWN]) {
        cameraSpeed=cameraSpeed - 0.1f;
    }
    else if (pressedKeys[GLFW_KEY_RIGHT]) {
         dir_rotatie += 0.1f;
    }
    else if (pressedKeys[GLFW_KEY_LEFT]) {
        dir_rotatie -= 0.1f;
    }
    else if (pressedKeys[GLFW_KEY_P]) {
        if (lightColorP == glm::vec3(0.0f, 0.0f, 0.0f)) {
            lightColorP = glm::vec3(0.3f, 0.3f, 0.3f);
        }
        else if (lightColorP == glm::vec3(0.3f, 0.3f, 0.3f)) {
            lightColorP = glm::vec3(0.6f, 0.6f, 0.6f);
        }
        else if (lightColorP == glm::vec3(0.6f, 0.6f, 0.6f)) {
            lightColorP = glm::vec3(1.0f, 1.0f, 1.0f);
        }
        else {
            lightColorP = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        Sleep(202);
            myBasicShader.useShaderProgram();
           glUniform3fv(lightColorPLoc, 1, glm::value_ptr(lightColorP));
        }

    else if (pressedKeys[GLFW_KEY_L]) {
        dir_rotatie = -dir_rotatie;
        Sleep(300);
       }
    else if (pressedKeys[GLFW_KEY_R] || pressedKeys[GLFW_KEY_F] || pressedKeys[GLFW_KEY_W] || pressedKeys[GLFW_KEY_S] || pressedKeys[GLFW_KEY_A] || pressedKeys[GLFW_KEY_D]) {
        if (pressedKeys[GLFW_KEY_R])
            myCamera.move(gps::MOVE_UP, cameraSpeed);

        if (pressedKeys[GLFW_KEY_F])
            myCamera.move(gps::MOVE_DOWN, cameraSpeed);

        if (pressedKeys[GLFW_KEY_W])
            myCamera.move(gps::MOVE_FORWARD, cameraSpeed);

        if (pressedKeys[GLFW_KEY_S])
            myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);

        if (pressedKeys[GLFW_KEY_A])
            myCamera.move(gps::MOVE_LEFT, cameraSpeed);

        if (pressedKeys[GLFW_KEY_D])
            myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    }
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
    glfwGetWindowSize(myWindow.getWindow(), &retina_width, &retina_height);
}

void setWindowCallbacks() {
   // glfwSetWindowSizeCallback(myWindow.getWindow());
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glAlphaFunc(GL_GREATER, 0.1f);
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    ss.LoadModel("models/moon/untitled.obj");
    earth.LoadModel("models/e/untitled.obj");
    moon.LoadModel("models/moon2/untitled.obj");
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    myBasicShader.useShaderProgram();
    SBShader.loadShader(
        "shaders/skyboxShader.vert",
        "shaders/skyboxShader.frag");
    SBShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    depthMapShader.loadShader(
        "shaders/depth.vert",
        "shaders/depth.frag");
    depthMapShader.useShaderProgram();
}

void initSB() {
    std::vector<const GLchar*> faces;
   /* faces.push_back("models/skybox/s_rt.tga");
    faces.push_back("models/skybox/s_lf.tga");
    faces.push_back("models/skybox/s_up.tga");
    faces.push_back("models/skybox/s_dn.tga");
    faces.push_back("models/skybox/s_bk.tga");
    faces.push_back("models/skybox/s_ft.tga");*/
    faces.push_back("models/skybox/s1_rt.tga");
    faces.push_back("models/skybox/s1_lf.tga");
    faces.push_back("models/skybox/s1_up.tga");
    faces.push_back("models/skybox/s1_dn.tga");
    faces.push_back("models/skybox/s1_bk.tga");
    faces.push_back("models/skybox/s1_ft.tga");
    mySkyBox.Load(faces);
    SBShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(SBShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 999.0f);
    glUniformMatrix4fv(glGetUniformLocation(SBShader.shaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));
}

glm::mat4 computeLightSpaceTrMatrix() {
    glm::mat4 lightView = glm::lookAt(lightDir, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightProjection = glm::ortho(-3000.0f, 3000.0f, -3000.0f, 3000.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    modelE = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
   //odelLocE = glGetUniformLocation(myBasicShader.shaderProgram, "modelE");
    modelM = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
   // modelLocM = glGetUniformLocation(myBasicShader.shaderProgram, "modelM");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    normalMatrixE = glm::mat3(glm::inverseTranspose(view * modelE));
   //ormalMatrixLocE = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrixE");
    normalMatrixM = glm::mat3(glm::inverseTranspose(view * modelE));
   // normalMatrixLocM = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrixE");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 500.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    //set light color
    viewPosEye = myCamera.getPosition();
    viewPosEyeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "viewPosEye");
    glUniform3fv(viewPosEyeLoc, 1, glm::value_ptr(viewPosEye));

    qLoc = glGetUniformLocation(myBasicShader.shaderProgram, "quadratic");
    glUniform1f(qLoc,quadratic);
    linLoc = glGetUniformLocation(myBasicShader.shaderProgram, "linear");
    glUniform1f(linLoc, linear);
    ctLoc = glGetUniformLocation(myBasicShader.shaderProgram, "constant");
    glUniform1f(ctLoc, constant);
    alLoc = glGetUniformLocation(myBasicShader.shaderProgram, "al");
    glUniform1f(alLoc, al);
    lightPos = glm::vec3(0.0f, 0.0f, -230.0f);
    lPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos");
    glUniform3fv(lPosLoc, 1, glm::value_ptr(lightPos));

    fogLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    glUniform1f(fogLoc, fog);

    lightColorP = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorPLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColorP");
    glUniform3fv(lightColorPLoc, 1, glm::value_ptr(lightColorP));

    glm::mat4 lightSpaceTrMatrix = computeLightSpaceTrMatrix();
    GLuint lightSpaceMatrixLocation = glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix");
    glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceTrMatrix));
    
    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

}

void renderSS(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    if(!depthPass)
       glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    ss.Draw(shader);
}

//glm::mat4 model2 = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
//glm::vec3 coordP1 = coordP;
void renderEarth(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();
    modelE = glm::translate(modelE, -coordP);
    modelE = glm::rotate(modelE, glm::radians(0.6f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelE = glm::translate(modelE, coordP);
    glm::mat4 model3 = model * modelE;
    if (!depthPass) {
        view = myCamera.getViewMatrix();
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model3));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));
    earth.Draw(shader); 
}

void renderMoon(gps::Shader shader,bool depthPass) {
   
    shader.useShaderProgram();
    modelM = glm::translate(modelM, -coordP);
    modelM = glm::rotate(modelM, glm::radians(0.4f), glm::vec3(1.0f, 0.0f, 0.0f));
    modelM = glm::rotate(modelM, glm::radians(0.4f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelM = glm::rotate(modelM, glm::radians(0.4f), glm::vec3(0.0f, 0.0f, 0.1f));
    modelM = glm::translate(modelM, coordP);
    glm::mat4 model3 = model * modelM;
    if (!depthPass) {
        view = myCamera.getViewMatrix();
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model3));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));
    if (trans) {
        al = 0.3;
        glUniform1f(alLoc, al);
    }
    else {
        al = 1.0;
        glUniform1f(alLoc, al);
    }
    moon.Draw(shader);
    al = 1.0;
    glUniform1f(alLoc, al);
}

void renderObj(gps::Shader shader, bool depthPass) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (animatie) {
        if (u < 400) {
            u = u + 1;
            myCamera.move(gps::MOVE_UP, 0.7);
        }
        else {
            animatie = false;
            view = oldView;
            myCamera.setVal(oldCamPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        }
    }
    // compute normal matrix for teapot
    if (!depthPass) {
        view = myCamera.getViewMatrix();
        shader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    mySkyBox.Draw(SBShader, view, projection);
    model = glm::translate(model, glm::vec3(0, 0, -225));
    model = glm::rotate(model, glm::radians(dir_rotatie), glm::vec3(0.0f, 1.0f, 0 + .0f));
    model = glm::translate(model, glm::vec3(0, 0, 225));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    renderSS(shader, depthPass);

    renderEarth(shader, depthPass);
    renderMoon(shader, depthPass);
    double currentTimeStamp = glfwGetTime();
    updateDelta(currentTimeStamp - lastTimeStamp);
    lastTimeStamp = currentTimeStamp;

}

void initFBO() {
    GLuint shadowMapFBO;
    //generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);
    GLuint depthMapTexture;
    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);//attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderScene() {

    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderObj(depthMapShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // render depth map on screen - toggled with the M key
    if (showDepthMap) {
        glViewport(0, 0, retina_width, retina_height);

        glClear(GL_COLOR_BUFFER_BIT);

        depthMapShader.useShaderProgram();

        //bind the depth map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
     //   glUniform1i(glGetUniformLocation(depthMapShader.shaderProgram, "shadowMap"), 0);

        glDisable(GL_DEPTH_TEST);
        //mainModel.Draw(depthMapShader);
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_FRAMEBUFFER_SRGB);
    }
    else {
        // final scene rendering pass (with shadows)

        glViewport(0, 0, retina_width, retina_height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myBasicShader.useShaderProgram();

        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

        //bind the shadow map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(computeLightSpaceTrMatrix()));

        renderObj(myBasicShader, false);
    }
}

void cleanup() {
    myWindow.Delete();
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    //close GL context and any other GLFW resources
    glfwTerminate();
    //cleanup code for your own data
}

int main(int argc, const char* argv[]) {
    coordP =  glm::vec3(45.0f, 0, 130.0f);
    printf("%f, %f, %f\n", coordP[0], coordP[1], coordP[2]);
    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initSB();
    initUniforms();
    setWindowCallbacks();
    glCheckError();
    // application loop
 //   model = glm::translate(model, -myCamera.getPosition());
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}