#include <iostream>
#include <filesystem>

#include "../dependencies/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../dependencies/stb_image.h"

#include "Shader.cpp"
#include "Camera.cpp"
#include "Primitives/Triangle.cpp"
#include "Primitives/Quad.cpp"
#include "Primitives/Circle.cpp"
#include "Primitives/Cuboid.cpp"
#include "Primitives/Sphere.cpp"
#include "Lights/DirectionalLight.cpp"
#include "Lights/PointLight.cpp"


// functions
void buildScene(std::vector<Primitive> &sceneObjects, std::vector<PointLight> &pointLights, DirectionalLight &dirLight, std::vector<unsigned int> &textureStorage);
void buildShadowMap(std::vector<Primitive> &sceneObjects, glm::mat4 lightSpaceMatrix, Shader depthShader, unsigned int &depthMapFBO);
void renderScene(std::vector<Primitive> &sceneObjects, std::vector<PointLight> &pointLights, DirectionalLight dirLight, glm::mat4 lightSpaceMatrix, unsigned int &depthMap);
void renderDebugQuad(unsigned int depthMap);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window, std::vector<Primitive> objects);
unsigned int loadTexture(char const* path);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

const unsigned int SHADOW_WIDTH = 1600;
const unsigned int SHADOW_HEIGHT = 900;

// camera
Camera camera(glm::vec3(-10.0f, -10.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    // GLFW set up
    glfwInit();
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // Face culling 
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Scene objects
    std::vector<Primitive> sceneObjects; 
    std::vector<PointLight> pointLights;
    DirectionalLight dirLight;
    std::vector<unsigned int> textureStorage;

    buildScene(sceneObjects, pointLights, dirLight, textureStorage);

    // Generate FrameBufferObject and depthMap for directional light 
    // in order to generate direct light's hard shadows.
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Generate FrameBufferObject and depth cube map for point light 
    // in order to generate point light's soft shadows.
    // unsigned int depthCubemap;
    // glGenTextures(1, &depthCubemap);

    // glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    // for (unsigned int i = 0; i < 6; ++i) {
    //     glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    // }
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // // attach depth texture as FBO's depth buffer
    // glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    // glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    // glDrawBuffer(GL_NONE);
    // glReadBuffer(GL_NONE);
    // glBindFramebuffer(GL_FRAMEBUFFER, 1);

    Shader simpleDepthShader("../shaders/simpleDepthShader.vs", "../shaders/simpleDepthShader.fs");

    // FPS variables
    double prevTime = 0.0f;
    double crntTime = 0.0f;
    double timeDiff;
    unsigned int counter = 0;

    while (!glfwWindowShouldClose(window)) {
        // FPS counter
        crntTime = glfwGetTime();
        timeDiff = crntTime - prevTime;
        counter++;
        if (timeDiff >= 10.0 / 30.0) {
            std::string FPS = std::to_string((1.0 / timeDiff) * counter);
            std::string ms = std::to_string((timeDiff / counter) * 1000);
            std::string newTitle = "Basic project - " + FPS + "FPS / " + ms + "ms";
            glfwSetWindowTitle(window, newTitle.c_str());
            prevTime = crntTime;
            counter = 0;
        }

        // inputs
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window, sceneObjects);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 0.1f, far_plane = 50.0f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

        glm::vec3 lightDir = dirLight.direction;
        glm::vec3 lightPos = -lightDir * 10.0f; // back off from origin
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

        lightSpaceMatrix = lightProjection * lightView;

        glCullFace(GL_FRONT);
        // Calcualte depthMap texture
        buildShadowMap(sceneObjects, lightSpaceMatrix, simpleDepthShader, depthMapFBO);
        
        glCullFace(GL_BACK);

        // color buffer and depth buffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // renderDebugQuad(depthMap);

        renderScene(sceneObjects, pointLights, dirLight, lightSpaceMatrix, depthMap);

        // swap buffers, do events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    for (int i = 0; i < sceneObjects.size(); i++) {
        glDeleteVertexArrays(1, &sceneObjects[i].VAO);
        glDeleteBuffers(1, &sceneObjects[i].VBO);
    }

    glfwTerminate();
    return 0;
}

void buildScene(std::vector<Primitive> &sceneObjects, std::vector<PointLight> &pointLights, DirectionalLight &dirLight, std::vector<unsigned int> &textureStorage) {

    // Shader programs
    Shader lightingShader("../shaders/simpleVertexShader.vs", "../shaders/lightingFragmentShader.fs");
    // shader to display the normals
    Shader normalShader("../shaders/normalVertexShader.vs", "../shaders/normalFragmentShader.fs", "../shaders/normalGeometryShader.gs");

    textureStorage.push_back(loadTexture("../textures/container2.png"));
    textureStorage.push_back(loadTexture("../textures/container2_specular.png"));

    // Triangle t(lightingShader, normalShader, 
    //     glm::vec3(0, 0, -2), glm::vec3(0.5f), glm::vec3(0), 
    //     glm::vec2(-1, 0), glm::vec2(1, 0), glm::vec2(0, 2), 
    //     glm::vec3(0.5f, 0.5f, 0.5f), true, &textureStorage[0], &textureStorage[1]);
    // sceneObjects.push_back(t);

    // Quad q(lightingShader, normalShader, 
    //     glm::vec3(2, 2, 0), glm::vec3(3), glm::vec3(0, 45, 0), 
    //     glm::vec2(0, 0), glm::vec2(1, 1), 
    //     glm::vec3(0.5f, 0.5f, 0.5f), true, &textureStorage[0], &textureStorage[1]);
    // sceneObjects.push_back(q);

    // Circle c1(lightingShader, normalShader, 
    //     glm::vec3(0, 5, -1), glm::vec3(0.5f), glm::vec3(0), 
    //     glm::vec2(0, 5), 5, 5, 
    //     glm::vec3(0.5f, 0.5f, 0.5f), true, &textureStorage[0], &textureStorage[1]);
    // sceneObjects.push_back(c1);

    // Circle c2(lightingShader, normalShader, 
    //     glm::vec3(10, 5, 0), glm::vec3(0.5f), glm::vec3(0), 
    //     glm::vec2(0, 0), 5, 100, 
    //     glm::vec3(0.5f, 0.5f, 0.5f), true, &textureStorage[0], &textureStorage[1]);
    // sceneObjects.push_back(c2);

    // Cuboid cuboid(lightingShader, normalShader, 
    //     glm::vec3(10, 5, -5), glm::vec3(0.5f), glm::vec3(0), 
    //     glm::vec3(1, 1, 1), glm::vec3(1, 1, 1), 
    //     glm::vec3(0.5f, 0.5f, 0.5f), false, &textureStorage[0], &textureStorage[1]);
    // sceneObjects.push_back(cuboid);

    // Sphere sphere(lightingShader, normalShader, 
    //     glm::vec3(-10, 5, -5), glm::vec3(0.5f), glm::vec3(0), 
    //     glm::vec3(0, 0, 0), 2, 50, 
    //     glm::vec3(0.5f, 0.5f, 0.5f), false, &textureStorage[0], &textureStorage[1]);
    // sceneObjects.push_back(sphere);

    Quad q(lightingShader, normalShader, 
        glm::vec3(1, 0, -3), glm::vec3(5), glm::vec3(0, 0, 0), 
        glm::vec2(0, 0), glm::vec2(1, 1), 
        glm::vec3(0.8f, 0.8f, 0.8f), true, &textureStorage[0], &textureStorage[1]);
    sceneObjects.push_back(q);

    Cuboid cuboid(lightingShader, normalShader, 
        glm::vec3(0, 1, 0), glm::vec3(0.2f), glm::vec3(0), 
        glm::vec3(1, 1, 1), glm::vec3(1, 2, 1), 
        glm::vec3(0.5f, 0.5f, 0.5f), true, &textureStorage[0], &textureStorage[1]);
    sceneObjects.push_back(cuboid);

    dirLight = DirectionalLight(1, glm::vec3(1.0f), glm::vec3(0.05f), glm::vec3(0.4f), glm::vec3(0.5f), glm::vec3(0.0f, 0.0f, -1.0f));

    PointLight p1(1, glm::vec3(1), glm::vec3(7, 5, 0), glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
    pointLights.push_back(p1);

    // PointLight p2(1, glm::vec3(1), glm::vec3(-7, 5, 0), glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
    // pointLights.push_back(p2);
}

void buildShadowMap(std::vector<Primitive> &sceneObjects, glm::mat4 lightSpaceMatrix, Shader depthShader, unsigned int &depthMapFBO) {
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT); 
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT); 

    depthShader.use();
    depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    for (int i = 0; i < sceneObjects.size(); i++) {

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, sceneObjects[i].rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, sceneObjects[i].rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, sceneObjects[i].rotation.z, glm::vec3(0, 0, 1));
        model = glm::translate(model, sceneObjects[i].translation);
        model = glm::scale(model, sceneObjects[i].scale);

        depthShader.setMat4("model", model);

        sceneObjects[i].draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderScene(std::vector<Primitive> &sceneObjects, std::vector<PointLight> &pointLights, DirectionalLight dirLight, glm::mat4 lightSpaceMatrix, unsigned int &depthMap) {
    // Render objects
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    for (int i = 0; i < sceneObjects.size(); i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, sceneObjects[i].rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, sceneObjects[i].rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, sceneObjects[i].rotation.z, glm::vec3(0, 0, 1));
        model = glm::translate(model, sceneObjects[i].translation);
        model = glm::scale(model, sceneObjects[i].scale);

        sceneObjects[i].shader.use();
        sceneObjects[i].shader.setMat4("projection", projection);
        sceneObjects[i].shader.setMat4("view", view);
        sceneObjects[i].shader.setMat4("model", model);
        sceneObjects[i].shader.setVec3("solidColor", sceneObjects[i].color);
        sceneObjects[i].shader.setBool("useSolidColor", sceneObjects[i].useSolidColor);

        sceneObjects[i].shader.setVec3("viewPos", camera.Position);

        sceneObjects[i].shader.setFloat("material.shininess", 32.0f);

        sceneObjects[i].shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // directional light
        sceneObjects[i].shader.setVec3("dirLight.direction", dirLight.direction);
        sceneObjects[i].shader.setVec3("dirLight.ambient", dirLight.ambient);
        sceneObjects[i].shader.setVec3("dirLight.diffuse", dirLight.diffuse);
        sceneObjects[i].shader.setVec3("dirLight.specular", dirLight.specular);

        for (size_t j = 0; j < pointLights.size(); ++j) {
            std::string prefix = "pointLights[" + std::to_string(j) + "].";
            sceneObjects[i].shader.setVec3(prefix + "position", pointLights[j].position);
            sceneObjects[i].shader.setVec3(prefix + "ambient", pointLights[j].ambient);
            sceneObjects[i].shader.setVec3(prefix + "diffuse", pointLights[j].diffuse);
            sceneObjects[i].shader.setVec3(prefix + "specular", pointLights[j].specular);
            sceneObjects[i].shader.setFloat(prefix + "constant", pointLights[j].constant);
            sceneObjects[i].shader.setFloat(prefix + "linear", pointLights[j].linear);
            sceneObjects[i].shader.setFloat(prefix + "quadratic", pointLights[j].quadratic);
        }

        if (sceneObjects[i].diffuseMap != nullptr && sceneObjects[i].specularMap != nullptr) {
            // pass sampler2D indexes
            sceneObjects[i].shader.setInt("material.diffuse", 0);
            sceneObjects[i].shader.setInt("material.specular", 1);

            // bind diffuse map
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, *sceneObjects[i].diffuseMap);
            // bind specular map
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, *sceneObjects[i].specularMap);
        }

        // Shadows
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        sceneObjects[i].shader.setInt("shadowMap", 2);

        // Draw object
        sceneObjects[i].draw();
        
        // Draw bb
        sceneObjects[i].bb.setTransformation(model);
        sceneObjects[i].drawBB();

        // Draw normals
        // sceneObjects[i].normalsShader.use();
        // sceneObjects[i].normalsShader.setMat4("projection", projection);
        // sceneObjects[i].normalsShader.setMat4("view", view);
        // sceneObjects[i].normalsShader.setMat4("model", model);
        // sceneObjects[i].draw();
    }
}

void renderDebugQuad(unsigned int depthMap) {
    static unsigned int quadVAO = 0, quadVBO;
    if (quadVAO == 0) {
        float quadVertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    static Shader debugShader("../shaders/debugQuad.vs", "../shaders/debugQuad.fs");
    debugShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    debugShader.setInt("depthMap", 0);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void processInput(GLFWwindow* window, std::vector<Primitive> objects) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime, objects);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime, objects);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime, objects);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime, objects);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

unsigned int loadTexture(char const* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
