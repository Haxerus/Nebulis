#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>

// Camera state
struct Camera {
    float posX = 0.0f, posY = 0.0f, posZ = 2.0f; // Position
    float yaw = -90.0f, pitch = 0.0f;            // Orientation (in degrees)
    float speed = 2.5f;                          // Movement speed
    float sensitivity = 0.1f;                    // Mouse sensitivity
} camera;

// Mouse input state
bool firstMouse = true;
double lastX = 400, lastY = 300;

constexpr int POINTS_PER_FRAME = 30000;

glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)2560 / 1440, 0.1f, 100.0f);

// Callback for resizing the window
static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
}

// Mouse movement callback
static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos; // Reversed: y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= camera.sensitivity;
    yoffset *= camera.sensitivity;

    camera.yaw += xoffset;
    camera.pitch += yoffset;

    // Constrain pitch to avoid flipping
    if (camera.pitch > 89.0f) camera.pitch = 89.0f;
    if (camera.pitch < -89.0f) camera.pitch = -89.0f;
}

// Handle WASD movement
static void processInput(GLFWwindow* window, float deltaTime) {
    float velocity = camera.speed * deltaTime;
    float forwardX = cos(glm::radians(camera.yaw));
    float forwardZ = sin(glm::radians(camera.yaw));
    float upX = 0, upY = 1, upZ = 0;

    // W: Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.posX += forwardX * velocity;
        camera.posZ += forwardZ * velocity;
    }
    // S: Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.posX -= forwardX * velocity;
        camera.posZ -= forwardZ * velocity;
    }
    // A: Move left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.posX += forwardZ * velocity;
        camera.posZ -= forwardX * velocity;
    }
    // D: Move right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.posX -= forwardZ * velocity;
        camera.posZ += forwardX * velocity;
    }
    // Shift: Move down
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.posY -= upY * velocity;
    }
    // Space: Move up
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.posY += upY * velocity;
    }
}

// Build the view matrix
static glm::mat4 getViewMatrix() {
    glm::vec3 position(camera.posX, camera.posY, camera.posZ);
    glm::vec3 direction(
        cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch)),
        sin(glm::radians(camera.pitch)),
        sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch))
    );
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    return glm::lookAt(position, position + direction, up);
}

// Generate random points for testing
static void generatePoints(std::vector<float>& points, size_t count) {
    points.clear();
    for (size_t i = 0; i < count; ++i) {
        points.push_back(static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f); // X
        points.push_back(static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f); // Y
        points.push_back(static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f); // Z
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(2560, 1440, "OpenGL Point Cloud", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW!" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Set up viewport and callback
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_PROGRAM_POINT_SIZE);

    // Create a large buffer to hold points
    const size_t maxPoints = 20000000;
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, maxPoints * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    // Simple shader program
    const char* vertexShaderSource = R"(
        #version 450 core
        layout(location = 0) in vec3 position;
		uniform mat4 view;
        uniform mat4 projection;
        uniform vec3 cameraPos;

        void main() {
            // Transform the vertex position
            vec4 worldPosition = vec4(position, 1.0);
            gl_Position = projection * view * worldPosition;

            // Calculate distance from the camera
            float distance = length(cameraPos - position);

            // Adjust point size based on distance (inverse relationship)
            float minSize = 1.0;
            float maxSize = 8.0;
            float size = max(maxSize / (distance + 1.0), minSize);
            gl_PointSize = size;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 450 core
        out vec4 fragColor;
        void main() {
            fragColor = vec4(1.0, 0.5, 0.2, 1.0);
        }
    )";

    // Compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Link program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint cameraPosLoc = glGetUniformLocation(shaderProgram, "cameraPos");

    // Set up VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // Main loop
    std::vector<float> points;
    generatePoints(points, 1); // Initial points

    size_t currentOffset = 0;

    double previousTime = glfwGetTime();
    double deltaTime = 0.0f;
    double accumulator = 0.0f;

    int frameCount = 0;

    while (!glfwWindowShouldClose(window)) {
        // Calculate deltaTime
        double currentTime = glfwGetTime();
        frameCount++;
        deltaTime = currentTime - previousTime;
        previousTime = currentTime;
        accumulator += deltaTime;

        if (accumulator >= 1.0) { // One second has passed
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            accumulator = 0.0f;
        }

        // Process input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        processInput(window, deltaTime);

        // Update buffer (simulate live data update)
        currentOffset = (currentOffset + points.size() * sizeof(float)) % (maxPoints * 3 * sizeof(float));
        glBufferSubData(GL_ARRAY_BUFFER, currentOffset, points.size() * sizeof(float), points.data());

        // Render
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);

        glm::mat4 view = getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glm::vec3 cameraPos(camera.posX, camera.posY, camera.posZ);
        glUniform3fv(cameraPosLoc, 1, glm::value_ptr(cameraPos));

        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, maxPoints);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Regenerate points for next frame
        generatePoints(points, POINTS_PER_FRAME); // Simulate new incoming data
    }

    // Clean up
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
