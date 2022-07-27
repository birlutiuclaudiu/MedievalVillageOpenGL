//
// Created by birlutiuclaudiu on 09.01.2022.
//

#ifndef BIRLUTIUPROJECT_STRUCTURES_H
#define BIRLUTIUPROJECT_STRUCTURES_H

#endif //BIRLUTIUPROJECT_STRUCTURES_H


struct Light{

    glm::vec3 lightDir;
    glm::vec3 lightColor;
    GLint lightDirLoc;
    GLint lightColorLoc;
    GLfloat intensity=1.0f;
};
struct MouseInfo{
    bool firstMouse = true;
    GLfloat yaw = -90.0f;
    GLfloat pitch = 0.0f;
    GLfloat ant_x , ant_y ;
    float sensitivity = 0.1f;
};

struct DeltaTime{
    GLfloat currentFrame = 0.0f;
    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;
    GLfloat speedCoef=80.0f;

};

struct FireParticle {
    gps::Model3D Model;
    glm::vec3 Position, Speed;
    glm::vec4 Color;
    float     Life;

    FireParticle()
            : Position(0.0f), Speed(0.0f), Color(1.0f), Life(0.0f) { }
};