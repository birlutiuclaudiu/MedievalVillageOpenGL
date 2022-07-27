#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types
#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"
#include <iostream>
#include "Structures.h"
#include <fstream>
using namespace std;
#define FILE_NAME "file.txt"
// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightRotation;

//shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;


// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 120.0f),
    glm::vec3(0.0f, 0.0f, -12.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;
GLfloat lightAngle = 0.0f;
GLfloat angle= 0.0f;
GLboolean pressedKeys[1024];

// models
gps::Model3D boat;
gps::Model3D teapot;
gps::Model3D nanosuit;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D collision[64];
gps::Model3D windWill1;
gps::Model3D windWill2;
gps::Model3D horseTail;
gps::Model3D sea;
gps::Model3D fireParticle;


// shaders
//gps::Shader myBasicShader;
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader fireShader;

//pentru umbre
GLuint shadowMapFBO;
GLuint depthMapTexture;
bool showDepthMap;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

//pentru Skybox
GLuint textureID;
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;



//pentru ceata
GLint isFogLoc;
int isFog=1;

//pentru lumina
Light dirLight;
Light pointLight;

//fisier pentru salvare pozitii camera
fstream myfile;
bool writeInFile = false;
bool readFromFile = false;


GLenum glCheckError_(const char *file, int line)
{
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

void updateViewMatrix(){
    view = myCamera.getViewMatrix();
    // compute normal matrix
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    fireShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(fireShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    myCustomShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));


}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    WindowDimensions windowDimensions{myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height};
    glfwGetFramebufferSize(myWindow.getWindow(), &windowDimensions.width, &windowDimensions.height);
    myWindow.setWindowDimensions(windowDimensions);
    myCustomShader.useShaderProgram();
    projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
   // updateViewMatrix();
}

void cleanup() {
    glDeleteTextures(1,& depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    myWindow.Delete();
    //cleanup code for your own data
    glfwTerminate();
}

////////////////////////////////MOUSE AND KEYBOARD/////////////////////////////////////////////////////////////////////
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
        isFog = -isFog;   //se schimba ceata

    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        readFromFile = !readFromFile;   //se schimba ceata false -> true
        if (readFromFile) {
            myfile.open(FILE_NAME, ios::in);
            if (!myfile) {
                cout << "No such file";
                readFromFile = !readFromFile;
            }
        } else if (myfile) {
            myfile.close();
        }
    }

    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        writeInFile = !writeInFile;   //se schimba ceata
        if(writeInFile){
            myfile.open(FILE_NAME, ios::out);
            if(!myfile)
                cout<<"Could not be created";
            else{
                char buffer[300];
                sprintf(buffer, "%f %f %f %f %f %f %f %f %f %f %f %f\n", myCamera.cameraPosition.x,myCamera.cameraPosition.y,myCamera.cameraPosition.z,
                        myCamera.cameraTarget.x, myCamera.cameraTarget.y, myCamera.cameraTarget.z,
                        myCamera.cameraUpDirection.x,myCamera.cameraUpDirection.y,myCamera.cameraUpDirection.z,
                        myCamera.cameraFrontDirection.x,myCamera.cameraFrontDirection.y,myCamera.cameraFrontDirection.z);
                myfile <<buffer;
            }
        }else if (myfile){
            myfile.close();
        }
    }
    if(pressedKeys[GLFW_KEY_1] ) {
        dirLight.intensity = (dirLight.intensity+0.1)>1.0f? 1.0f:(dirLight.intensity+0.1f);
    }
    if(pressedKeys[GLFW_KEY_2] ) {
        dirLight.intensity =  (dirLight.intensity-0.1f)<0.0f? 0.0f:(dirLight.intensity-0.1f);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void writeInFileCameraInfo(){
    char buffer[300];
    sprintf(buffer, "%f %f %f %f %f %f %f %f %f %f %f %f\n", myCamera.cameraPosition.x,myCamera.cameraPosition.y,myCamera.cameraPosition.z,
            myCamera.cameraTarget.x, myCamera.cameraTarget.y, myCamera.cameraTarget.z,
            myCamera.cameraUpDirection.x,myCamera.cameraUpDirection.y,myCamera.cameraUpDirection.z,
            myCamera.cameraFrontDirection.x,myCamera.cameraFrontDirection.y,myCamera.cameraFrontDirection.z);
    myfile <<buffer;
}
MouseInfo mouse;
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (mouse.firstMouse)
    {
        mouse.ant_x = xpos;
        mouse.ant_y = ypos;
        mouse.firstMouse = false;
    }
    float difx = xpos - mouse.ant_x;
    float dify = mouse.ant_y - ypos;
    mouse.ant_x = xpos;
    mouse.ant_y = ypos;

    difx *= mouse.sensitivity;
    dify *= mouse.sensitivity;
    mouse.yaw += difx;
    mouse.pitch += dify;
    if (mouse.pitch > 89.0f)
        mouse.pitch = 89.0f;
    if (mouse.pitch < -89.0f)
        mouse.pitch = -89.0f;
    myCamera.rotate(mouse.pitch, mouse.yaw);
    if (writeInFile){
       writeInFileCameraInfo();
    }
    updateViewMatrix();
}


DeltaTime cameraDelta;
//variabile
void processMovement() {
    cameraDelta.currentFrame = (float)glfwGetTime();
    cameraDelta.deltaTime = cameraDelta.currentFrame - cameraDelta.lastFrame;
    cameraDelta.lastFrame = cameraDelta.currentFrame;
    cameraSpeed =cameraDelta.speedCoef * (cameraDelta.deltaTime);
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed, collision);
		//update view matrix
        updateViewMatrix();
        if (writeInFile){
            writeInFileCameraInfo();
        }
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed, collision);
        //update view matrix
        updateViewMatrix();
        if (writeInFile){
            writeInFileCameraInfo();
        }
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed, collision);
        //update view matrix
        updateViewMatrix();
        if (writeInFile){
            writeInFileCameraInfo();
        }
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed, collision);
        //update view matrix
        updateViewMatrix();
        if (writeInFile){
            writeInFileCameraInfo();
        }
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        // for teapot
        angle -= 0.1f* cameraSpeed;
    }

    if (pressedKeys[GLFW_KEY_E]) {
        // for teapot
        angle += 0.1f*cameraSpeed;
    }
    if (pressedKeys[GLFW_KEY_L]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (pressedKeys[GLFW_KEY_F]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (pressedKeys[GLFW_KEY_P]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    if (pressedKeys[GLFW_KEY_O]) {
        glShadeModel( GL_SMOOTH);
    }
    if (pressedKeys[GLFW_KEY_I]) {
        glShadeModel( GL_FLAT);
    }
    if (pressedKeys[GLFW_KEY_Z]) {
        cleanup();
        exit(0);
    }
    if (pressedKeys[GLFW_KEY_T]) {
        lightAngle+=0.3f*cameraSpeed;
        updateViewMatrix();
    }
    if (pressedKeys[GLFW_KEY_Y]) {
        lightAngle-=0.3f*cameraSpeed;
        updateViewMatrix();
    }

}
//////////////////////////////////////////FOR PARTICLES////////////////////////////////////////////////////////////////

/////////////////////////////////////////INIT/////////////////////////////////////////////////////////////////////////
void initOpenGLWindow() {
    myWindow.Create(1800, 900, "OpenGL Birlutiu Claudiu");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    boat.LoadModel("models/boat/boat.obj");
    teapot.LoadModel("models/teapot/teapot20segUT.obj");
    nanosuit.LoadModel("models/nanosuit/nanosuit.obj");
    ground.LoadModel("models/scenaOGL/scenaF1.obj");
    lightCube.LoadModel("models/cube/cube.obj");
    screenQuad.LoadModel("models/quad/quad.obj");
    windWill1.LoadModel("models/Windwill/elice.obj");
    windWill2.LoadModel("models/Elice2/windWill2.obj");
    horseTail.LoadModel("models/CoadaCal/horseTail.obj");

    for(int i=0; i<64; i++){
        char buffer[100];
        sprintf(buffer,"models/colision/collision%d.obj", i+1);
        collision[i].LoadModel(buffer);
    }
    sea.LoadModel("models/Sea/sea.obj");
    fireParticle.LoadModel("models/Fire/fire.obj");

}

void initShaders() {

    myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
    myCustomShader.useShaderProgram();
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    lightShader.useShaderProgram();
    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    screenQuadShader.useShaderProgram();
    depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
    depthMapShader.useShaderProgram();

    //crearea shader-ului cu geometry shader /////////////////////
    fireShader.loadShader("shaders/particle.vert", "shaders/particle.frag");
    std::string f = fireShader.readShaderFile("shaders/particle.geom");
    const GLchar* geShaderString = f.c_str();
    GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, &geShaderString, NULL);
    glCompileShader(geometryShader);
    glAttachShader(fireShader.shaderProgram, geometryShader);
    glLinkProgram(fireShader.shaderProgram);
    fireShader.useShaderProgram();

}


void initUniforms() {
    myCustomShader.useShaderProgram();
    // create model matrix for teapot
    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
//////////////////////////////////////////////////POINT LIGHT/////////////////////////////////////////////////////////////////////////////////
    //set the light direction (direction towards the light)
    pointLight.lightDir = glm::vec3(-141.0f, 18.0f, 159.0f);
    pointLight.lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightDir");
    glUniform3fv(pointLight.lightDirLoc, 1,  glm::value_ptr(pointLight.lightDir));

    //set light color
    pointLight.lightColor = glm::vec3(0.4f, 0.0f, 0.0f); //red color
    pointLight.lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor");
    glUniform3fv(pointLight.lightColorLoc, 1, glm::value_ptr(pointLight.lightColor));
/////////////////////////////////////////////DIRECTIONAL LIGHT///////////////////////////////////////////////////////////////////////////////////
    dirLight.lightDir = glm::vec3(-100.0f, 90.0f, -159.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    dirLight.lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "directionalLightDir");
    glUniform3fv( dirLight.lightDirLoc, 1,  glm::value_ptr( glm::mat3(lightRotation) * dirLight.lightDir));

    //set light color
    dirLight.lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    dirLight.lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "directionalLightColor");
    glUniform3fv(dirLight.lightColorLoc, 1, glm::value_ptr(dirLight.lightColor));


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //pentru ceata
    isFogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "isFog");
    glUniform1i(isFogLoc, isFog);

    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    fireShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(fireShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(fireShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(fireShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform1i(glGetUniformLocation(fireShader.shaderProgram, "nbOfParticles"), rand()%100);
}


glm::mat4 computeLightSpaceTrMatrix() {
    //TODO - Return the light-space transformation matrix
    glm::vec4 viewCenter = lightRotation * glm::vec4(dirLight.lightDir,0.0f);
    glm::mat4 lightView = glm::lookAt(glm::vec3(viewCenter), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 800.0f;
    glm::mat4 lightProjection = glm::ortho(-150.0f, 150.0f, -150.0f, 150.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection *lightView;
    return lightSpaceTrMatrix;
}
/////////////////////////////////////////////////RENDER////////////////////////////////////////////////////////////////
DeltaTime animationDelta={0.0f, 0.0f, 10.0f};
GLfloat gradeElice = 0.0f;
GLfloat gradeElice2 = 0.0f;
GLfloat incrementElice  = 5.0f;
GLfloat gradeTail = 0.0f;
GLfloat incrementTail = 2.0f;
GLfloat speedAnimation;
GLfloat seaLevel=0.0f;
GLfloat incrementLevelSea =0.01;
GLfloat boatMove = 0.01;
GLfloat boatIncrement = 0.1;
void drawObjects(gps::Shader shader, bool depthPass) {
    //////////////////////////////////setare viteze cu delta time//////////////////////////////////////////////////
    animationDelta.currentFrame = (float)glfwGetTime();
    animationDelta.deltaTime = animationDelta.currentFrame - animationDelta.lastFrame;
    animationDelta.lastFrame = animationDelta.currentFrame;
    speedAnimation =animationDelta.speedCoef * (animationDelta.deltaTime);
    shader.useShaderProgram();
    ///////////////////////////////////////////////transmitere informatii: ceata, intensitate lumina/////////////////
    if (!depthPass) {
        glUniform1i(glGetUniformLocation(shader.shaderProgram, "isFog"),isFog);
        glm::vec3 colorDir =glm::vec3(dirLight.intensity * dirLight.lightColor.x, dirLight.intensity * dirLight.lightColor.y,dirLight.intensity * dirLight.lightColor.z);
                                      glUniform3fv(dirLight.lightColorLoc, 1, glm::value_ptr(colorDir));
    }

   //---------------------------SCENA--------------------------------
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    ground.Draw(shader);
    ////////////////////////ceaininc/////////////////////
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-38.0f, 8.0f, 120.0f));
    model = glm::scale(model, glm::vec3(5.0f));
    model = glm::rotate(model,angle,glm::vec3(0.0f,1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    teapot.Draw(shader);

    //---------------------------Animatie mare-------------
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.4f, 0.0f));
    seaLevel += incrementLevelSea*speedAnimation;
    if(abs(seaLevel) > 0.4f){
        incrementLevelSea = - incrementLevelSea;
        seaLevel = (seaLevel<-0.4f)? -0.4f : 0.4f;
    }
    model = glm::translate(model, glm::vec3(0.0f, seaLevel, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    sea.Draw(shader);

    //---------------------------Animatie barca-------------
    boatMove+=boatIncrement*speedAnimation;
    model = glm::translate(model, glm::vec3(-20.0f, 0.0f, boatMove));
    if(boatMove >100.0f){
        boatMove = 100.0f;
        boatIncrement = - boatIncrement;
    }else if (boatMove <0.0f){
        boatMove = 0.0f;
        boatIncrement = -boatIncrement;
    }
    if(boatIncrement<0.0f){
        model = glm::translate(model, glm::vec3(-10.284f, 3.5573f, -124.39));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(10.284f, -3.5573f, 124.39));
    }
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    boat.Draw(shader);
    //---------------------------animatie elice moara 1------------------

    model = glm::mat4(1.0f);
    gradeElice = (gradeElice + incrementElice*speedAnimation > 360.0f)? 0.0f:gradeElice + incrementElice*speedAnimation;
    model = glm::translate(model, glm::vec3(54.167f, 34.237f, 205.25f));
    model = glm::rotate(model, glm::radians(gradeElice), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-54.167f, -34.237f, -205.25f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    windWill1.Draw(shader);
    //---------------------------animatie elice moara 2------------------

    model = glm::mat4(1.0f);
    gradeElice2 = (gradeElice2 + incrementElice*speedAnimation > 360.0f)? 0.0f:gradeElice2 + incrementElice*speedAnimation;
    model = glm::translate(model, glm::vec3(156.0f, 41.05f, -27.983));
    model = glm::rotate(model, glm::radians(gradeElice2), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-156.0f, -41.05f, 27.983));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    windWill2.Draw(shader);

    //---------------------------animatie coada cal 2------------------
    model = glm::mat4(1.0f);
    gradeTail = gradeTail + incrementTail*speedAnimation;
    if(abs(gradeTail) >60.0f){
        incrementTail = -incrementTail;
        gradeTail = gradeTail<-60.0f? -60.0f:60.0f;
    }
    model = glm::translate(model, glm::vec3(109.08f, 9.2502f, 207.06));
    model = glm::rotate(model, glm::radians(gradeTail), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-109.08f, -9.2502f, -207.06));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    horseTail.Draw(shader);



    //-----------------Desenare skybox-------------------
    mySkyBox.Draw(skyboxShader, view, projection, dirLight.intensity);

}

void animationCamera(){
    string buff;
    if(myfile.eof())
        myfile.seekg(0, ios::beg);
    if(!getline(myfile,buff)) {
        myfile.close();
        myfile.open(FILE_NAME, ios::in);
        myfile.seekg(0, ios::beg);
        getline(myfile,buff);
    }

    char buffer[300];
    int i=0;
    for( i=0; i<buff.length(); i++){
        buffer[i]=buff[i];
    }
    buffer[i]='\n';
    float px,py,pz, tx,ty,tz,dx,dy,dz,fx,fy,fz;
    sscanf(buffer, "%f %f %f %f %f %f %f %f %f %f %f %f",&px,&py,&pz,&tx,&ty,&tz,&dx,&dy,&dz,&fx,&fy,&fz);
    myCamera.cameraPosition.x=px;        myCamera.cameraPosition.y=py;       myCamera.cameraPosition.z=pz;
    myCamera.cameraTarget.x=tx;          myCamera.cameraTarget.y=ty;         myCamera.cameraTarget.z=tz;
    myCamera.cameraUpDirection.x=dx;     myCamera.cameraUpDirection.y=dy;    myCamera.cameraUpDirection.z=dz;
    myCamera.cameraFrontDirection.x=fx;  myCamera.cameraFrontDirection.y=fy; myCamera.cameraFrontDirection.z=fz;

}
void renderScene() {

    if(readFromFile){
        animationCamera();
    }
    // depth maps creation pass
    //TODO - Send the light-space transformation matrix to the depth map creation shader and
    //		 render the scene in the depth map
    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),1,GL_FALSE,glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    drawObjects(depthMapShader,1);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // render depth map on screen - toggled with the M key

    if (showDepthMap) {
        glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();

        //bind the depth map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    }
    else {

        // final scene rendering pass (with shadows)
        glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myCustomShader.useShaderProgram();

        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        ///////////////////////POINT LIGHT/////////////////////////////////////////
        glUniform3fv(pointLight.lightDirLoc, 1, glm::value_ptr( pointLight.lightDir));
        ///////////////////////DIR LIGHT//////////////////////////////////////////
        lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniform3fv( dirLight.lightDirLoc, 1,  glm::value_ptr( glm::mat3(lightRotation) * dirLight.lightDir));
        /////////////////////////bind the shadow map/////////////////////////////////////////////////
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
        glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
                           1,
                           GL_FALSE,
                           glm::value_ptr(computeLightSpaceTrMatrix()));
        drawObjects(myCustomShader, false);

        //draw the white cube around the light
        lightShader.useShaderProgram();
        model = lightRotation;
        model = glm::translate(model, 1.0f * dirLight.lightDir);
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        lightCube.Draw(lightShader);
        //animatie foc
        fireShader.useShaderProgram();
        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(fireShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
        int nbParticle = rand()%200;
        glUniform1i(glGetUniformLocation(fireShader.shaderProgram, "nbParticles"), nbParticle);
        glUniformMatrix4fv(glGetUniformLocation(fireShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        fireParticle.Draw(fireShader);
    }

}


void initFBO() {
    //TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
    glGenFramebuffers(1, &shadowMapFBO);
    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initSkyBox(){
    faces.push_back("textures/skybox/right.tga");
    faces.push_back("textures/skybox/left.tga");
    faces.push_back("textures/skybox/top.tga");
    faces.push_back("textures/skybox/bottom.tga");
    faces.push_back("textures/skybox/back.tga");
    faces.push_back("textures/skybox/front.tga");
    mySkyBox.Load(faces);
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
    view= myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    projection= glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width/ (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

int main(int argc, const char * argv[]) {
    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    setWindowCallbacks();
    initFBO();
    initSkyBox();

    glCheckError();
    // application loop
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
