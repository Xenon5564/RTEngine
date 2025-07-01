#include "CameraController.h"
#include <cmath>

#define M_PI 3.1415926535

CameraController::CameraController(Camera& camera, GLFWwindow* window)
    : cam(camera), win(window), lastX(0.0), lastY(0.0), firstMouse(true), yaw(-90.0f), pitch(0.0f) {
    // initialize yaw and pitch from camera forward vector
    yaw = atan2f(cam.forward[2], cam.forward[0]) * 180.0f / M_PI - 90.0f;
    pitch = asinf(cam.forward[1]) * 180.0f / M_PI;
}

void CameraController::updateVectors() {
    // calculate forward vector from yaw and pitch
    float yawRad = yaw * M_PI / 180.0f;
    float pitchRad = -pitch * M_PI / 180.0f;
    cam.forward[0] = cosf(pitchRad) * cosf(yawRad);
    cam.forward[1] = sinf(pitchRad);
    cam.forward[2] = cosf(pitchRad) * sinf(yawRad);

    // world up
    float up[3] = {0.0f, 1.0f, 0.0f};

    // right = normalize(cross(forward, up))
    cam.right[0] = cam.forward[1] * up[2] - cam.forward[2] * up[1];
    cam.right[1] = cam.forward[2] * up[0] - cam.forward[0] * up[2];
    cam.right[2] = cam.forward[0] * up[1] - cam.forward[1] * up[0];
    float rlen = sqrtf(cam.right[0]*cam.right[0] + cam.right[1]*cam.right[1] + cam.right[2]*cam.right[2]);
    cam.right[0] /= rlen; cam.right[1] /= rlen; cam.right[2] /= rlen;

    // up = cross(right, forward)
    cam.up[0] = cam.right[1] * cam.forward[2] - cam.right[2] * cam.forward[1];
    cam.up[1] = cam.right[2] * cam.forward[0] - cam.right[0] * cam.forward[2];
    cam.up[2] = cam.right[0] * cam.forward[1] - cam.right[1] * cam.forward[0];
}

void CameraController::update(float deltaTime) {
    const float speed = 2.5f * deltaTime;
    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) {
        cam.position[0] += cam.forward[0] * speed;
        cam.position[1] += cam.forward[1] * speed;
        cam.position[2] += cam.forward[2] * speed;
    }
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) {
        cam.position[0] -= cam.forward[0] * speed;
        cam.position[1] -= cam.forward[1] * speed;
        cam.position[2] -= cam.forward[2] * speed;
    }
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) {
        cam.position[0] -= cam.right[0] * speed;
        cam.position[1] -= cam.right[1] * speed;
        cam.position[2] -= cam.right[2] * speed;
    }
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) {
        cam.position[0] += cam.right[0] * speed;
        cam.position[1] += cam.right[1] * speed;
        cam.position[2] += cam.right[2] * speed;
    }
    if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cam.position[1] -= speed;
    }
    if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
        cam.position[1] += speed;
    }

    double x, y;
    glfwGetCursorPos(win, &x, &y);
    if (firstMouse) {
        lastX = x; lastY = y; firstMouse = false;
    }
    float xoffset = static_cast<float>(x - lastX);
    float yoffset = static_cast<float>(lastY - y); // reversed since y-coordinates go from bottom to top
    lastX = x; lastY = y;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateVectors();
}
