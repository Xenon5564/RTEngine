#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "CameraController.h"

const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = 800;

std::string loadFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to load: " << path << std::endl;
        exit(EXIT_FAILURE);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GLuint loadShader(const char* path, GLenum type) {
    std::string srcStr = loadFile(path);
    const char* src = srcStr.c_str();
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compilation failed:\n" << log << std::endl;
        exit(EXIT_FAILURE);
    }
    return shader;
}

GLuint createProgram(const char* vsPath, const char* fsPath) {
    GLuint vs = loadShader(vsPath, GL_VERTEX_SHADER);
    GLuint fs = loadShader(fsPath, GL_FRAGMENT_SHADER);
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

GLuint createComputeProgram(const char* path) {
    GLuint cs = loadShader(path, GL_COMPUTE_SHADER);
    GLuint program = glCreateProgram();
    glAttachShader(program, cs);
    glLinkProgram(program);
    glDeleteShader(cs);
    return program;
}

int main() {
    // Init GLFW and window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Compute Shader UV Demo", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Load shaders
    GLuint computeProgram = createComputeProgram("Shaders/RayTrace.comp");
    GLuint screenProgram = createProgram("Shaders/fullscreen.vert", "Shaders/fullscreen.frag");

    // Define a simple camera
    Camera cam = {
        {0.0f, 0.0f, 3.0f},   // position
        {0.0f, 0.0f, 1.0f},  // forward
        {0.0f, 1.0f, 0.0f},   // up
        {1.0f, 0.0f, 0.0f},   // right
        60.0f                // fov in degrees
    };
    CameraController controller(cam, window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Camera uniforms
    glUseProgram(computeProgram);
    GLint locPos = glGetUniformLocation(computeProgram, "camPos");
    GLint locForward = glGetUniformLocation(computeProgram, "camForward");
    GLint locUp = glGetUniformLocation(computeProgram, "camUp");
    GLint locRight = glGetUniformLocation(computeProgram, "camRight");
    GLint locFov = glGetUniformLocation(computeProgram, "camFOV");

    // Frame counters
    GLint locTotalFrameCount = glGetUniformLocation(computeProgram, "globalFrameID");
    GLint locFrameCountSinceMove = glGetUniformLocation(computeProgram, "frameID");

    glUniform3fv(locPos, 1, cam.position);
    glUniform3fv(locForward, 1, cam.forward);
    glUniform3fv(locUp, 1, cam.up);
    glUniform3fv(locRight, 1, cam.right);
    glUniform1f(locFov, cam.fov);

    // Create output texture
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, WIDTH, HEIGHT);
    glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Create accumulation texture
    GLuint accumTex;
    glGenTextures(1, &accumTex);
    glBindTexture(GL_TEXTURE_2D, accumTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, WIDTH, HEIGHT);
    glBindImageTexture(1, accumTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    // Fullscreen triangle setup
    float quadVerts[] = {
        -1.0f, -1.0f,
         3.0f, -1.0f,
        -1.0f,  3.0f
    };
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    float lastTime = (float)glfwGetTime();

    // Frame counters
    int totalFrameCount = 0;
    int frameCountSinceCameraMove = 0;
    Camera lastCamera = cam;

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        controller.update(deltaTime);

        // Detect camera movement (simple position/orientation check)
        bool cameraMoved = false;
        if (memcmp(&cam, &lastCamera, sizeof(Camera)) != 0) {
            cameraMoved = true;
            lastCamera = cam;
        }

        if (cameraMoved) {
            frameCountSinceCameraMove = 0;
            // Optionally clear accumulation image here if you use one
            glClearTexImage(accumTex, 0, GL_RGBA, GL_FLOAT, nullptr);
        }

        glUseProgram(computeProgram);
        glUniform3fv(locPos, 1, cam.position);
        glUniform3fv(locForward, 1, cam.forward);
        glUniform3fv(locUp, 1, cam.up);
        glUniform3fv(locRight, 1, cam.right);
        glUniform1f(locFov, cam.fov);

        // Pass frame counters to shader
        glUniform1i(locTotalFrameCount, totalFrameCount);
        glUniform1i(locFrameCountSinceMove, frameCountSinceCameraMove);

        glDispatchCompute(WIDTH / 8, HEIGHT / 8, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Display output texture
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(screenProgram);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // Increment counters
        ++totalFrameCount;
        ++frameCountSinceCameraMove;
    }

    glfwTerminate();
    return 0;
}