#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //TODO - Update the rest of camera parameters
        this->cameraFrontDirection = glm::normalize(cameraPosition - cameraTarget); //cameraDirection
        this->cameraRightDirection = glm::normalize(glm::cross(cameraUp,cameraFrontDirection));
        this->cameraUpDirection = glm::normalize(glm::cross(cameraFrontDirection,cameraRightDirection));

    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition+cameraFrontDirection, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed, Model3D collision[]) {
        //TODO
        glm::vec3 newPos;
        switch (direction) {
            case MOVE_FORWARD:
                newPos = cameraPosition+ (cameraFrontDirection * speed);
                break;

            case MOVE_BACKWARD:
                newPos = cameraPosition +(-cameraFrontDirection * speed);
                break;

            case MOVE_RIGHT:
                 newPos = cameraPosition+glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
                break;

            case MOVE_LEFT:
                newPos = cameraPosition-glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
                break;
        }
        //PENTRU COLIZIUNI
        GLfloat min_x, max_x, min_y, max_y, min_z, max_z;
        for(int c=0; c<64; c++) {
            for (int i = 0; i < collision[c].meshes.size(); i++) {
                min_x = max_x = collision[c].meshes[i].vertices[0].Position.x;
                min_y = max_y = collision[c].meshes[i].vertices[0].Position.y;
                min_z = max_z = collision[c].meshes[i].vertices[0].Position.z;
                for (int j = 0; j < collision[c].meshes[i].vertices.size(); j++) {
                    if (collision[c].meshes[i].vertices[j].Position.x < min_x)
                        min_x = collision[c].meshes[i].vertices[j].Position.x;
                    if (collision[c].meshes[i].vertices[j].Position.x > max_x)
                        max_x = collision[c].meshes[i].vertices[j].Position.x;
                    if (collision[c].meshes[i].vertices[j].Position.y < min_y)
                        min_y = collision[c].meshes[i].vertices[j].Position.y;
                    if (collision[c].meshes[i].vertices[j].Position.y > max_y)
                        max_y = collision[c].meshes[i].vertices[j].Position.y;
                    if (collision[c].meshes[i].vertices[j].Position.z < min_z)
                        min_z = collision[c].meshes[i].vertices[j].Position.z;
                    if (collision[c].meshes[i].vertices[j].Position.z > max_z)
                        max_z = collision[c].meshes[i].vertices[j].Position.z;
                }
                if (newPos.x > min_x and newPos.x < max_x and newPos.y > min_y and newPos.y < max_y and
                    newPos.z > min_z and newPos.z < max_z)
                    return;

            }
        }
        cameraPosition = newPos;
        if (cameraPosition.y <6.0f)
            cameraPosition.y = 6.0f;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection = glm::normalize(direction);
        //cameraFrontDirection=glm::normalize(glm::cross(cameraUpDirection,cameraFrontDirection));
    }
}