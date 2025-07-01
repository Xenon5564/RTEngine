#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct Camera {
    float position[3];
    float forward[3];
    float up[3];
    float right[3];
    float fov;
};

class CameraController {
public:
    CameraController(Camera& camera, GLFWwindow* window);
    void update(float deltaTime);

private:
    Camera& cam;
    GLFWwindow* win;
    double lastX;
    double lastY;
    bool firstMouse;
    float yaw;
    float pitch;

    void updateVectors();
};

#endif // CAMERA_CONTROLLER_H
