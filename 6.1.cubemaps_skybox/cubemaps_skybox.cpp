#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

// function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(std::vector<std::string> faces);

// screen settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 2.0f, 6.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// game variables
glm::vec3 playerPosition(0.0f, 0.0f, 0.0f);

struct Coin {
    glm::vec3 position;
    float speed;
    bool collected;
};

std::vector<Coin> coins;
float spawnTimer = 0.0f;
int score = 0;
bool gameOver = false;

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};



int main()
{
    srand((unsigned)time(NULL));

    // GLFW init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Collect Game", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader("6.1.cubemaps.vs", "6.1.cubemaps.fs");
    Shader skyboxShader("6.1.skybox.vs", "6.1.skybox.fs");
    //Shader groundShader("6.1ground.vs", "6.1ground.fs");


    // load models
    Model playerModel(FileSystem::getPath("resources/objects/player/player.obj"));
    Model coinModel(FileSystem::getPath("resources/objects/coin/coin.obj"));
    Model groundModel(FileSystem::getPath("resources/objects/ground/ground.obj"));

    // skybox texture
    std::vector<std::string> faces{
        FileSystem::getPath("resources/textures/skybox/right.jpg"),
        FileSystem::getPath("resources/textures/skybox/left.jpg"),
        FileSystem::getPath("resources/textures/skybox/top.jpg"),
        FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
        FileSystem::getPath("resources/textures/skybox/front.jpg"),
        FileSystem::getPath("resources/textures/skybox/back.jpg")
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    // render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        
        // camera follows player
        glm::vec3 cameraOffset(0.0f, 2.0f, 6.0f);
        camera.Position = playerPosition + cameraOffset;
        camera.Front = glm::normalize(playerPosition - camera.Position);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            static bool keyPressed = false;
            if (!keyPressed) {
                keyPressed = true;

                // หยุดอัปเดตทุกอย่างชั่วคราว
                std::cout << "Game paused!" << std::endl;
                int result = MessageBoxA(NULL,
                    "Game Paused\n\nPress YES to Resume or NO to Exit.",
                    "Pause Menu",
                    MB_YESNO | MB_ICONQUESTION);

                if (result == IDNO) {
                    glfwSetWindowShouldClose(window, true);
                }
                else {
                    std::cout << "Game resumed!" << std::endl;
                }
            }
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
            static bool keyPressed = false;
            keyPressed = false;
        }


        // --- update coins (spawn + move) ---
        spawnTimer += deltaTime;
        if (spawnTimer >= 0.3f) {

            // spawn coin ภายในขอบเขตของพื้น
           
            glm::vec3 groundCenter = glm::vec3(10.0f, 0.0f, 10.0f); // ขยับขึ้นและเข้ามาหากล้องเล็กน้อย
            glm::vec3 groundSize = glm::vec3(18.0f, 0.1f, 18.0f);
            Coin c;
            spawnTimer = 0.0f; // reset timer หลัง spawn
            float spawnX = ((rand() % 1000) / 1000.0f) * groundSize.x - groundSize.x / 2.0f;
            float spawnZ = ((rand() % 1000) / 1000.0f) * groundSize.z - groundSize.z / 2.0f;
            c.position = glm::vec3(groundCenter.x + spawnX, 10.0f, groundCenter.z + spawnZ);

            c.speed = 2.0f + (rand() % 100) / 100.0f;
            c.collected = false;
            coins.push_back(c);


        }

        for (auto& c : coins) {
            if (!c.collected) {
                // เหรียญตกลงมา
                c.position.y -= c.speed * deltaTime;

                // ถ้าตกต่ำกว่าพื้น -> หายไป
                if (c.position.y < -1.0f)
                    c.collected = true;

                // ✅ ตรวจจับชนกับ player เพื่อเก็บเหรียญ
                float dist = glm::length(playerPosition - c.position);
                if (dist < 0.8f) { // ระยะชน
                    c.collected = true;
                    score++;
                    std::cout << "Score: " << score << std::endl;
                }
            }
        }



        // --- rendering ---
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        // ✅ ส่งค่าแสง (ตำแหน่งไฟ) และตำแหน่งกล้องให้ shader
        shader.setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f)); // จุดไฟเหนือฉาก
        shader.setVec3("viewPos", camera.Position);

        

        // --- Ground ---
        // --- Draw ground ---
        shader.setBool("isGround", true);
        shader.setBool("isCoin", false);
        shader.setVec3("modelColor", glm::vec3(0.9f, 0.9f, 0.9f)); // light gray color

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // adjust Y if needed
        model = glm::scale(model, glm::vec3(20.0f, 0.1f, 20.0f));     // bigger ground
        shader.setMat4("model", model);
        groundModel.Draw(shader);

        // reset after drawing
        shader.setBool("isGround", false);

        

        // player
        shader.setBool("isGround", false);
        shader.setBool("isCoin", false);
        shader.setVec3("modelColor", glm::vec3(1.0f, 1.0f, 1.0f));
        model = glm::mat4(1.0f);
        model = glm::translate(model, playerPosition);
        shader.setMat4("model", model);
        playerModel.Draw(shader);

 
        // coins
        shader.setBool("isCoin", true);
        shader.setBool("isGround", false);
        shader.setVec3("modelColor", glm::vec3(1.0f, 0.85f, 0.1f)); // golden yellow
        glDisable(GL_CULL_FACE);
        for (auto& c : coins) {
            if (!c.collected) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, c.position);
                model = glm::rotate(model, (float)glfwGetTime() * 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::scale(model, glm::vec3(0.3f));
                shader.setMat4("model", model);
                coinModel.Draw(shader);
            }
        }
        glEnable(GL_CULL_FACE);
        shader.setBool("isCoin", false);


        // --- HUD: simple green score bar ---
        glDisable(GL_DEPTH_TEST);
        float hudVertices[] = {
            -0.95f, 0.90f,  0.2f, 1.0f, 0.2f,
            -0.95f + (score * 0.05f), 0.90f,  0.2f, 1.0f, 0.2f,
            -0.95f + (score * 0.05f), 0.95f,  0.2f, 1.0f, 0.2f,
            -0.95f, 0.95f,  0.2f, 1.0f, 0.2f
        };
        unsigned int hudIndices[] = { 0, 1, 2, 2, 3, 0 };
        unsigned int hudVAO, hudVBO, hudEBO;
        glGenVertexArrays(1, &hudVAO);
        glGenBuffers(1, &hudVBO);
        glGenBuffers(1, &hudEBO);
        glBindVertexArray(hudVAO);
        glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(hudVertices), hudVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hudEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(hudIndices), hudIndices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        shader.use();
        glm::mat4 hudModel = glm::mat4(1.0f);
        shader.setMat4("model", hudModel);
        shader.setMat4("view", glm::mat4(1.0f));
        shader.setMat4("projection", glm::mat4(1.0f));
        glBindVertexArray(hudVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glEnable(GL_DEPTH_TEST);

        // ==========================


        // --- draw skybox last ---
        glDepthFunc(GL_LEQUAL);  // ให้ depth test ผ่านง่ายขึ้นสำหรับ skybox
        skyboxShader.use();
        glm::mat4 viewSkybox = glm::mat4(glm::mat3(view)); // ตัดการแปลกล้องออก
        skyboxShader.setMat4("view", viewSkybox);
        skyboxShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        // วาด Score Bar
// ==========================

        glDisable(GL_DEPTH_TEST); // ปิด depth test เพื่อให้วาดทับทุกอย่าง

        // ความยาว bar ตามคะแนน (จำกัดสูงสุด 100)
        float maxScore = 100.0f;
        float barLength = std::min((float)score / maxScore, 1.0f); // 0.0 - 1.0

        // วาดพื้นหลังสีเทา
        glBegin(GL_QUADS);
        glColor3f(0.2f, 0.9f, 0.3f);
        glVertex2f(-0.9f, 0.9f);
        glVertex2f(0.9f, 0.9f);
        glVertex2f(0.9f, 0.85f);
        glVertex2f(-0.9f, 0.85f);
        glEnd();

        // วาด bar สีเขียวตามคะแนน
        glBegin(GL_QUADS);
        glColor3f(0.2f, 0.9f, 0.3f);
        glVertex2f(-0.9f, 0.9f);
        glVertex2f(-0.9f + 1.8f * barLength, 0.9f);
        glVertex2f(-0.9f + 1.8f * barLength, 0.85f);
        glVertex2f(-0.9f, 0.85f);
        glEnd();

        glEnable(GL_DEPTH_TEST); // เปิด depth test กลับ
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// --- input ---
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float moveSpeed = 5.0f * deltaTime;

    glm::vec3 forward = glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z));
    glm::vec3 right = glm::normalize(glm::cross(forward, camera.Up));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        playerPosition += forward * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        playerPosition -= forward * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        playerPosition -= right * moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        playerPosition += right * moveSpeed;

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        static bool keyPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        static bool keyPressed = false;
        keyPressed = false;
    }


}


// --- callbacks ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {}

unsigned int loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int w, h, n;
    unsigned char* data = stbi_load(path, &w, &h, &n, 0);
    GLenum format = GL_RGB;
    if (data) {
        if (n == 1) format = GL_RED;
        else if (n == 3) format = GL_RGB;
        else if (n == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

