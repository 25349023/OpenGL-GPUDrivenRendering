#define GLM_FORCE_SWIZZLE

#include <array>

#include "src\Shader.h"
#include "src\SceneRenderer.h"
#include <GLFW\glfw3.h>
#include "src\MyImGuiPanel.h"

#include "src\ViewFrustumSceneObject.h"
#include "src\InfinityPlane.h"

#include "glm/gtx/quaternion.hpp"
#include "src/MyPoissonSample.h"

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "src/Shape.h"
#include "src/Material.h"
#include "src/Model.h"

#pragma comment (lib, "lib-vc2015\\glfw3.lib")
#pragma comment(lib, "assimp-vc141-mt.lib")

int FRAME_WIDTH = 1024;
int FRAME_HEIGHT = 512;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void cursorPosCallback(GLFWwindow* window, double x, double y);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void updateGodViewMat();
void updatePlayerViewMat();
void recalculateLocalZ();
bool initializeGL();
void resizeGL(GLFWwindow* window, int w, int h);
void paintGL();
void resize(const int w, const int h);

glm::vec3 rotateCenterAccordingToEye(const glm::vec3& center, const glm::vec3& eye,
                                     const glm::mat4& viewMat, const float rad);

bool leftButtonPressed = false;
bool rightButtonPressed = false;

glm::vec2 lastCursorPos;


MyImGuiPanel* m_imguiPanel = nullptr;

void vsyncDisabled(GLFWwindow* window);

// ==============================================
SceneRenderer* defaultRenderer = nullptr;
ShaderProgram* defaultShaderProgram = new ShaderProgram();

glm::mat4 godProjMat;
glm::mat4 godViewMat;
glm::mat4 playerProjMat;
glm::mat4 playerViewMat;

glm::vec3 godEye(0.0, 50.0, 20.0), godViewDir(0.0, -30.0, -30.0), godUp(0.0, 1.0, 0.0);
glm::vec3 godLocalX(-1, 0, 0), godLocalY(0, 1, 0), godLocalZ(0, 0, -1);
glm::vec3 playerEye(0.0, 8.0, 10.0), playerCenter(0.0, 5.0, 0.0), playerUp(0.0, 1.0, 0.0);
glm::vec3 playerLocalZ(0, 0, -1);

ViewFrustumSceneObject* viewFrustumSO = nullptr;
InfinityPlane* infinityPlane = nullptr;
// ==============================================

Model merged_grass;
int numSamples[3];
const float* samplePositions[3];

GLuint vao, ssbo;

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


void loadModel()
{
    std::vector<Model> grasses(3);

    grasses[0] = Model("assets/grassB.obj", "assets/grassB_albedo.png");
    grasses[1] = Model("assets/bush01_lod2.obj", "assets/bush01.png");
    grasses[2] = Model("assets/bush05_lod2.obj", "assets/bush05.png");

    merged_grass = Model::merge(grasses);
    
    MyPoissonSample* sample0 = MyPoissonSample::fromFile("assets/poissonPoints_155304.ppd");
    numSamples[0] = sample0->m_numSample;
    samplePositions[0] = sample0->m_positions; // (size = num_sample * 3)
    std::cout << "There are " << numSamples[0] << " Samples." << std::endl;

    MyPoissonSample* sample1 = MyPoissonSample::fromFile("assets/poissonPoints_1010.ppd");
    numSamples[1] = sample1->m_numSample;
    samplePositions[1] = sample1->m_positions;
    std::cout << "There are " << numSamples[1] << " Samples." << std::endl;

    MyPoissonSample* sample2 = MyPoissonSample::fromFile("assets/poissonPoints_2797.ppd");
    numSamples[2] = sample2->m_numSample;
    samplePositions[2] = sample2->m_positions;
    std::cout << "There are " << numSamples[2] << " Samples." << std::endl;
}

void initSSBO()
{
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, (numSamples[0] + numSamples[1] + numSamples[2]) * 3 * sizeof(int),
        NULL, GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT);

    long long offset = 0;
    for (int i = 0; i < 3; i++)
    {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, numSamples[i] * 3 * sizeof(int), samplePositions[i]);
        offset += numSamples[i] * 3 * sizeof(int);
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
}

void initInstancedSettings()
{
    GLuint offsetHandel = SceneManager::Instance()->m_offsetHandel;
    unsigned int offset = 0;

    glBindVertexArray(merged_grass.shape.vao);
    glBindBuffer(GL_ARRAY_BUFFER, ssbo);
    glEnableVertexAttribArray(offsetHandel);
    glVertexAttribPointer(offsetHandel, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)offset);
    glVertexAttribDivisor(offsetHandel, 1);

    glBindVertexArray(0);
}

void genDrawCommands()
{
    const int numCmd = 3;
    GLuint cmdList[numCmd * 5];

    unsigned int offset = 0;
    unsigned int baseInst = 0;

    for (int i = 0; i < numCmd; i++)
    {
        cmdList[i * 5] = merged_grass.drawCounts[i];
        cmdList[i * 5 + 1] = numSamples[i];
        cmdList[i * 5 + 2] = offset;
        ((GLint*)cmdList)[i * 5 + 3] = merged_grass.baseVertices[i];
        cmdList[i * 5 + 4] = baseInst;

        offset += merged_grass.drawCounts[i];
        baseInst += numSamples[i];
    }
    
    GLuint indirectBufHandle;

    glBindVertexArray(merged_grass.shape.vao);
    glGenBuffers(1, &indirectBufHandle);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBufHandle);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(cmdList), cmdList, GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
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
    // godViewMat = glm::lookAt(glm::vec3(0.0, 50.0, 20.0), glm::vec3(0.0, 20.0, -10.0), glm::vec3(0.0, 1.0, 0.0));
    godLocalZ = glm::normalize(godViewDir);
    godLocalY = glm::normalize(glm::cross(godLocalZ, godLocalX));
    updateGodViewMat();
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

    // =================================================================	
    // load objs, init buffers
    loadModel();
    initSSBO();
    initInstancedSettings();
    genDrawCommands();

    return true;
}

void resizeGL(GLFWwindow* window, int w, int h)
{
    resize(w, h);
}

void updateGodViewMat()
{
    godViewMat = glm::lookAt(godEye, godEye + godViewDir, godUp);
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

void drawGrass()
{
    auto sm = SceneManager::Instance();
    glUniform1i(sm->m_instancedDrawHandle, 1);

    glBindVertexArray(merged_grass.shape.vao);

    glActiveTexture(sm->m_albedoTexUnit);
    glUniform1i(sm->m_fs_albedoTexHandle, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, merged_grass.material.diffuse_tex);

    glUniform1i(sm->m_fs_pixelProcessIdHandle, sm->m_fs_textureMapping);
    glm::mat4 id(1);
    glUniformMatrix4fv(sm->m_modelMatHandle, 1, false, glm::value_ptr(id));

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, 3, 0);

    glBindVertexArray(0);
    glUniform1i(sm->m_instancedDrawHandle, 0);
}

void paintGL()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    updateGodViewMat();
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
    drawGrass();

    // rendering with player view
    defaultRenderer->setViewport(HALF_W, 0, HALF_W, FRAME_HEIGHT);
    defaultRenderer->setProjection(playerProjMat);
    defaultRenderer->setView(playerViewMat);
    defaultRenderer->renderPass();
    drawGrass();
    // ===============================

    ImGui::Begin("My name is window");
    m_imguiPanel->update();
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

////////////////////////////////////////////////
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            leftButtonPressed = true;
        }
        else
        {
            leftButtonPressed = false;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            rightButtonPressed = true;
        }
        else
        {
            rightButtonPressed = false;
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double x, double y)
{
    if (leftButtonPressed)
    {
        using namespace glm;
        glm::vec2 changed = glm::vec2(x, y) - lastCursorPos;
        vec2 change = 0.2f * (lastCursorPos - vec2(x, y));

        int sign = (godViewDir.z > 0) ? -1 : 1;
        mat4 R = mat4_cast(quat(vec3(radians(sign * change.y), radians(change.x), 0)));

        godViewDir = (R * vec4(godViewDir, 1)).xyz;

        godLocalZ = (R * vec4(godLocalZ, 1)).xyz;
        godLocalX = normalize(cross(godUp, godLocalZ));
        godLocalY = normalize(cross(godLocalZ, godLocalX));
    }
    lastCursorPos = glm::vec2(x, y);
}

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

void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // printf("%f, %f\n", xoffset, yoffset);
    godEye += (float)yoffset * godLocalZ;
}

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
