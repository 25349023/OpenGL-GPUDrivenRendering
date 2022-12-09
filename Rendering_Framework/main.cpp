#include "src\Shader.h"
#include "src\SceneRenderer.h"
#include <GLFW\glfw3.h>
#include "src\MyImGuiPanel.h"

#include "src\ViewFrustumSceneObject.h"
#include "src\InfinityPlane.h"

#include <glm/gtx/quaternion.hpp>

#pragma comment (lib, "lib-vc2015\\glfw3.lib")
#pragma comment(lib, "assimp-vc141-mt.lib")

int FRAME_WIDTH = 1024;
int FRAME_HEIGHT = 512;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void cursorPosCallback(GLFWwindow* window, double x, double y);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void updatePlayerViewMat();
void recalculateLocalZ();
bool initializeGL();
void resizeGL(GLFWwindow* window, int w, int h);
void paintGL();
void resize(const int w, const int h);

glm::vec3 rotateCenterAccordingToEye(const glm::vec3& center, const glm::vec3& eye,
                                     const glm::mat4& viewMat, const float rad);

bool m_leftButtonPressed = false;
bool m_rightButtonPressed = false;
double cursorPos[2];


MyImGuiPanel* m_imguiPanel = nullptr;

void vsyncDisabled(GLFWwindow* window);

// ==============================================
SceneRenderer* defaultRenderer = nullptr;
ShaderProgram* defaultShaderProgram = new ShaderProgram();

glm::mat4 godProjMat;
glm::mat4 godViewMat;
glm::mat4 playerProjMat;
glm::mat4 playerViewMat;

glm::vec3 godEye(0, 0, 5), godViewDir(0, 0, -1), godUp(0, 1, 0);
glm::vec3 playerEye(0.0, 8.0, 10.0), playerCenter(0.0, 5.0, 0.0), playerUp(0.0, 1.0, 0.0);
glm::vec3 playerLocalX(-1, 0, 0), playerLocalY(0, 1, 0), playerLocalZ(0, 0, -1);

ViewFrustumSceneObject* viewFrustumSO = nullptr;
InfinityPlane* infinityPlane = nullptr;
// ==============================================

void updateWhenPlayerProjectionChanged(const float nearDepth, const float farDepth);
void viewFrustumMultiClipCorner(const std::vector<float>& depths, const glm::mat4& viewMat, const glm::mat4& projMat,
                                float* clipCorner);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(FRAME_WIDTH, FRAME_HEIGHT, "rendering", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // load OpenGL function pointer
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetFramebufferSizeCallback(window, resizeGL);

    if (initializeGL() == false)
    {
        std::cout << "initialize GL failed\n";
        glfwTerminate();
        system("pause");
        return 0;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    // disable vsync
    glfwSwapInterval(0);

    // start game-loop
    vsyncDisabled(window);


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void vsyncDisabled(GLFWwindow* window)
{
    double previousTimeForFPS = glfwGetTime();
    int frameCount = 0;

    static int IMG_IDX = 0;

    while (!glfwWindowShouldClose(window))
    {
        // measure speed
        const double currentTime = glfwGetTime();
        frameCount = frameCount + 1;
        const double deltaTime = currentTime - previousTimeForFPS;
        if (deltaTime >= 1.0)
        {
            m_imguiPanel->setAvgFPS(frameCount * 1.0);
            m_imguiPanel->setAvgFrameTime(deltaTime * 1000.0 / frameCount);

            // reset
            frameCount = 0;
            previousTimeForFPS = currentTime;
        }

        glfwPollEvents();
        paintGL();
        glfwSwapBuffers(window);
    }
}


bool initializeGL()
{
    // initialize shader program
    // vertex shader
    Shader* vsShader = new Shader(GL_VERTEX_SHADER);
    vsShader->createShaderFromFile("src\\shader\\oglVertexShader.glsl");
    std::cout << vsShader->shaderInfoLog() << "\n";

    // fragment shader
    Shader* fsShader = new Shader(GL_FRAGMENT_SHADER);
    fsShader->createShaderFromFile("src\\shader\\oglFragmentShader.glsl");
    std::cout << fsShader->shaderInfoLog() << "\n";

    // shader program
    ShaderProgram* shaderProgram = new ShaderProgram();
    shaderProgram->init();
    shaderProgram->attachShader(vsShader);
    shaderProgram->attachShader(fsShader);
    shaderProgram->checkStatus();
    if (shaderProgram->status() != ShaderProgramStatus::READY)
    {
        return false;
    }
    shaderProgram->linkProgram();

    vsShader->releaseShader();
    fsShader->releaseShader();

    delete vsShader;
    delete fsShader;
    // =================================================================
    // init renderer
    defaultRenderer = new SceneRenderer();
    if (!defaultRenderer->initialize(FRAME_WIDTH, FRAME_HEIGHT, shaderProgram))
    {
        return false;
    }

    // =================================================================
    // initialize camera
    godViewMat = glm::lookAt(glm::vec3(0.0, 50.0, 20.0), glm::vec3(0.0, 20.0, -10.0), glm::vec3(0.0, 1.0, 0.0));
    updatePlayerViewMat();

    const glm::vec4 directionalLightDir = glm::vec4(0.4, 0.5, 0.8, 0.0);

    defaultRenderer->setDirectionalLightDir(directionalLightDir);
    // =================================================================
    // initialize camera and view frustum
    infinityPlane = new InfinityPlane(2);
    defaultRenderer->appendObject(infinityPlane->sceneObject());

    viewFrustumSO = new ViewFrustumSceneObject(2, SceneManager::Instance()->m_fs_pixelProcessIdHandle,
        SceneManager::Instance()->m_fs_pureColor);
    defaultRenderer->appendObject(viewFrustumSO->sceneObject());

    resize(FRAME_WIDTH, FRAME_HEIGHT);
    // =================================================================	
    m_imguiPanel = new MyImGuiPanel();

    return true;
}

void resizeGL(GLFWwindow* window, int w, int h)
{
    resize(w, h);
}

void updatePlayerViewMat()
{
    playerViewMat = glm::lookAt(playerEye, playerCenter, playerUp);
}

void recalculateLocalZ()
{
    playerLocalZ = playerCenter - playerEye;
    playerLocalZ.y = 0;
    playerLocalZ = glm::normalize(playerLocalZ);
}

void paintGL()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    updatePlayerViewMat();
    // ===============================
    // update infinity plane with player camera
    // const glm::vec3 PLAYER_VIEW_POSITION = glm::vec3(0.0, 8.0, 10.0);
    infinityPlane->updateState(playerViewMat, playerEye);

    // update player camera view frustum
    viewFrustumSO->updateState(playerViewMat, playerEye);

    // =============================================
    // start new frame
    defaultRenderer->setViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
    defaultRenderer->startNewFrame();

    // rendering with god view
    const int HALF_W = FRAME_WIDTH * 0.5;
    defaultRenderer->setViewport(0, 0, HALF_W, FRAME_HEIGHT);
    defaultRenderer->setProjection(godProjMat);
    defaultRenderer->setView(godViewMat);
    defaultRenderer->renderPass();

    // rendering with player view
    defaultRenderer->setViewport(HALF_W, 0, HALF_W, FRAME_HEIGHT);
    defaultRenderer->setProjection(playerProjMat);
    defaultRenderer->setView(playerViewMat);
    defaultRenderer->renderPass();
    // ===============================

    ImGui::Begin("My name is window");
    m_imguiPanel->update();
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

////////////////////////////////////////////////
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{ }
void cursorPosCallback(GLFWwindow* window, double x, double y)
{ }

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const float translateSpeed = 0.5f, rotateSpeed = 1.5f;
    const glm::vec3 translateAmount = translateSpeed * playerLocalZ;

    switch (key)
    {
    case GLFW_KEY_W:
        playerEye += translateAmount;
        playerCenter += translateAmount;
        break;
    case GLFW_KEY_S:
        playerEye -= translateAmount;
        playerCenter -= translateAmount;
        break;
    case GLFW_KEY_A:
        playerCenter = rotateCenterAccordingToEye(
            playerCenter, playerEye, playerViewMat, glm::radians(rotateSpeed));
        recalculateLocalZ();
        break;
    case GLFW_KEY_D:
        playerCenter = rotateCenterAccordingToEye(
            playerCenter, playerEye, playerViewMat, glm::radians(-rotateSpeed));
        recalculateLocalZ();
        break;
    }
}

void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {}

void updateWhenPlayerProjectionChanged(const float nearDepth, const float farDepth)
{
    // get view frustum corner
    const int NUM_CASCADE = 2;
    const float HY = 0.0;

    float dOffset = (farDepth - nearDepth) / NUM_CASCADE;
    float* corners = new float[(NUM_CASCADE + 1) * 12];
    std::vector<float> depths(NUM_CASCADE + 1);
    for (int i = 0; i < NUM_CASCADE; i++)
    {
        depths[i] = nearDepth + dOffset * i;
    }
    depths[NUM_CASCADE] = farDepth;
    // get viewspace corners
    glm::mat4 tView = glm::lookAt(glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    // calculate corners of view frustum cascade
    viewFrustumMultiClipCorner(depths, tView, playerProjMat, corners);

    // update infinity plane
    for (int i = 0; i < NUM_CASCADE; i++)
    {
        float* cascadeBuffer = infinityPlane->cascadeDataBuffer(i);

        cascadeBuffer[0] = corners[((i + 1) * 4 + 1) * 3 + 0];
        cascadeBuffer[1] = HY;
        cascadeBuffer[2] = corners[((i + 1) * 4 + 1) * 3 + 2];

        cascadeBuffer[3] = corners[((i + 1) * 4 + 1) * 3 + 0];
        cascadeBuffer[4] = HY;
        cascadeBuffer[5] = corners[(i * 4 + 1) * 3 + 2];

        cascadeBuffer[6] = corners[((i + 1) * 4 + 2) * 3 + 0];
        cascadeBuffer[7] = HY;
        cascadeBuffer[8] = corners[(i * 4 + 2) * 3 + 2];

        cascadeBuffer[9] = corners[((i + 1) * 4 + 2) * 3 + 0];
        cascadeBuffer[10] = HY;
        cascadeBuffer[11] = corners[((i + 1) * 4 + 2) * 3 + 2];
    }
    infinityPlane->updateDataBuffer();

    // update view frustum scene object
    for (int i = 0; i < NUM_CASCADE + 1; i++)
    {
        float* layerBuffer = viewFrustumSO->cascadeDataBuffer(i);
        for (int j = 0; j < 12; j++)
        {
            layerBuffer[j] = corners[i * 12 + j];
        }
    }
    viewFrustumSO->updateDataBuffer();
}

void resize(const int w, const int h)
{
    FRAME_WIDTH = w;
    FRAME_HEIGHT = h;

    // half for god view, half for player view
    const int HALF_W = w * 0.5;
    const double PLAYER_PROJ_FAR = 150.0;

    godProjMat = glm::perspective(glm::radians(75.0), HALF_W * 1.0 / h, 0.1, 500.0);
    playerProjMat = glm::perspective(glm::radians(45.0), HALF_W * 1.0 / h, 0.1, PLAYER_PROJ_FAR);

    defaultRenderer->resize(w, h);

    updateWhenPlayerProjectionChanged(0.1, PLAYER_PROJ_FAR);
}

void viewFrustumMultiClipCorner(const std::vector<float>& depths, const glm::mat4& viewMat, const glm::mat4& projMat,
                                float* clipCorner)
{
    const int NUM_CLIP = depths.size();

    // Calculate Inverse
    glm::mat4 viewProjInv = glm::inverse(projMat * viewMat);

    // Calculate Clip Plane Corners
    int clipOffset = 0;
    for (const float depth : depths)
    {
        // Get Depth in NDC, the depth in viewSpace is negative
        glm::vec4 v = glm::vec4(0, 0, -1 * depth, 1);
        glm::vec4 vInNDC = projMat * v;
        if (fabs(vInNDC.w) > 0.00001)
        {
            vInNDC.z = vInNDC.z / vInNDC.w;
        }
        // Get 4 corner of clip plane
        float cornerXY[] = {
            -1, 1,
            -1, -1,
            1, -1,
            1, 1
        };
        for (int j = 0; j < 4; j++)
        {
            glm::vec4 wcc = {
                cornerXY[j * 2 + 0], cornerXY[j * 2 + 1], vInNDC.z, 1
            };
            wcc = viewProjInv * wcc;
            wcc = wcc / wcc.w;

            clipCorner[clipOffset * 12 + j * 3 + 0] = wcc[0];
            clipCorner[clipOffset * 12 + j * 3 + 1] = wcc[1];
            clipCorner[clipOffset * 12 + j * 3 + 2] = wcc[2];
        }
        clipOffset = clipOffset + 1;
    }
}


glm::vec3 rotateCenterAccordingToEye(const glm::vec3& center, const glm::vec3& eye,
                                     const glm::mat4& viewMat, const float rad)
{
    glm::mat4 vt = glm::transpose(viewMat);
    glm::vec4 yAxisVec4 = vt[1];
    glm::vec3 yAxis(yAxisVec4.x, yAxisVec4.y, yAxisVec4.z);
    glm::quat q = glm::angleAxis(rad, yAxis);
    glm::mat4 rotMat = glm::toMat4(q);
    glm::vec3 p = center - eye;
    glm::vec4 resP = rotMat * glm::vec4(p.x, p.y, p.z, 1.0);
    return glm::vec3(resP.x + eye.x, resP.y + eye.y, resP.z + eye.z);
}
