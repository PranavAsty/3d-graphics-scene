#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <tuple>
#include <unistd.h>
#include <math.h>
#include "scene.h"
#include "sphere.h"
#include "cube.h"
#include "cylinder.h"
#include "monkeybars.h"

#define PI 3.14159265
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define BOUND_X1 -20.0
#define BOUND_X2  20.0
#define BOUND_Y1  0.0
#define BOUND_Y2  10.0
#define BOUND_Z1 -20.0
#define BOUND_Z2  20.0

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

glm::vec3 cameraPos   = glm::vec3(0.0f, 1.0f,  6.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

bool firstMouse = true;
float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;
float fov   =  45.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
    if (cameraPos.y < 0.5)
        cameraPos.y = 0.5;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cameraPos.y += (yoffset/2);
    // cameraPos = glm::normalize(cameraPos);
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (cameraPos.x < BOUND_X1)
        cameraPos.x = BOUND_X1;
    if (cameraPos.x > BOUND_X2)
        cameraPos.x = BOUND_X2;
    if (cameraPos.y < BOUND_Y1)
        cameraPos.y = BOUND_Y1;
    if (cameraPos.y > BOUND_Y2)
        cameraPos.y = BOUND_Y2;
    if (cameraPos.z < BOUND_Z1)
        cameraPos.z = BOUND_Z1;
    if (cameraPos.z > BOUND_Z2)
        cameraPos.z = BOUND_Z2;
}

void renderMesh(Mesh * mesh, glm::vec3 color_vec)
{
    vector<GLfloat> v = mesh -> getVertices();

    int len = v.size();
    GLfloat * vertices = &v[0];


    GLfloat color[] = {color_vec.x, color_vec.y, color_vec.z};
    GLfloat color_vector[len];
    for (int i = 0; i < len;)
    {
        for (int j = 0; j < 3; j++)
        {
            color_vector[i++] = color[j];
        }
    }

    glColorPointer(3, GL_FLOAT, 0, color_vector);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLES, 0, len/3);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void drawCube(glm::vec3 center, glm::vec3 dimensions, glm::vec3 color_vec, glm::mat4 transformation = glm::mat4(1.0f)) // pass vertices as quads
{
    Cube * cube = new Cube(dimensions.x, dimensions.y, dimensions.z);
    Mesh * mesh = cube -> getMesh();

    glm::mat4 T;
    T = glm::translate(T, center);
    T = glm::translate(T, glm::vec3(-dimensions.x/2, -dimensions.y/2, -dimensions.z/2));
    mesh -> transform(transformation*T);

    renderMesh(mesh, color_vec);

    free(cube);
    free(mesh);
}

void drawCylinder(glm::vec3 center, float height, float radius, glm::vec3 color_vec, glm::vec3 orientation, glm::mat4 transformation = glm::mat4(1.0f))
{
    Cylinder * cylinder = new Cylinder(height, radius, orientation);

    glm::mat4 T;
    T = glm::translate(T, center);

    Mesh * mesh = cylinder -> getMesh();
    mesh -> transform(transformation*T);

    renderMesh(mesh, color_vec);

    free(cylinder);
    free(mesh);
}

void drawSphere(glm::vec3 center, float radius, glm::vec3 color_vec, glm::mat4 transformation, int detail_level=2)
{
    Sphere * sphere = new Sphere(radius, detail_level);
    Mesh * mesh  = sphere -> getMesh();

    glm::mat4 T;
    T = glm::translate(T, center);
    mesh -> transform(transformation*T);

    renderMesh(mesh, color_vec);

    free(sphere);
    free(mesh);
}

void drawMonkeyBars(glm::vec3 position, glm::vec3 color_vec, int length, int height, glm::mat4 transformation)
{
    MonkeyBars * monkeybars = new MonkeyBars(length, height);
    Mesh * mesh = monkeybars -> getMesh();

    glm::mat4 T;
    T = glm::translate(T, position);
    mesh -> transform(transformation*T);

    renderMesh(mesh, color_vec);

    free(monkeybars);
    free(mesh);
}

glm::mat4 updateView()
{
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    return view;
}

int main()
{
    GLFWwindow *window;
    // Initialize the library
    if (!glfwInit()) {
        return -1;
    }
    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "IS F311 Computer Graphics Assignment", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context
    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glViewport(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective((float)fov, (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);

    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    glm::vec3 cubePositions[] = {
        glm::vec3( 2.0f,  0.0f, -0.0f),
        glm::vec3( 2.0f,  0.0f, -5.0f),
        glm::vec3( 2.0f,  0.0f, -10.0f),
        glm::vec3( 2.0f,  0.0f, -15.0f),
        glm::vec3( 2.0f,  0.0f, -20.0f),
        glm::vec3(-2.0f,  0.0f, 0.0f),
        glm::vec3(-2.0f,  0.0f, -5.0f),
        glm::vec3(-2.0f,  0.0f, -10.0f),
        glm::vec3(-2.0f,  0.0f, -15.0f),
        glm::vec3(-2.0f,  0.0f, -20.0f),
    };

    Scene * scene = new Scene();
    glm::mat4 view = updateView();
    // scene -> addCube(glm::vec3(0, 0, 0), glm::vec3(2 ,2, 1), glm::vec3(1, 0, 0));
    scene->addMonkeyBars(glm::vec3(-2, 2, 0), glm::vec3(0.5, 0.5, 0.5), 7, 3);
    vector<tuple<Mesh, glm::vec3>> mesh_group = scene -> getMesh();

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glm::mat4 old_view;
        view = updateView();
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // DRAW WALLS
        // Left
        drawCube(glm::vec3(BOUND_X1-1, 0, 0), glm::vec3(1, (BOUND_Y2+BOUND_Y1+1)*2, (BOUND_Z2-BOUND_Z1+2)), glm::vec3(0, 0, 1), view);
        // Right
        drawCube(glm::vec3(BOUND_X2+1, 0, 0), glm::vec3(1, (BOUND_Y2+BOUND_Y1+1)*2, (BOUND_Z2-BOUND_Z1+2)), glm::vec3(0, 1, 0), view);
        // Bottom
        drawCube(glm::vec3(0, BOUND_Y1-1, 0), glm::vec3((BOUND_X2-BOUND_X1+2), 1, (BOUND_Z2-BOUND_Z1+2)), glm::vec3(0, 1, 1), view);
        // Top
        drawCube(glm::vec3(0, BOUND_Y2+1, 0), glm::vec3((BOUND_X2-BOUND_X1+2), 1, (BOUND_Z2-BOUND_Z1+2)), glm::vec3(1, 0, 1), view);
        // Back
        drawCube(glm::vec3(0, 0, BOUND_Z1-1), glm::vec3((BOUND_X2-BOUND_X1+2), (BOUND_Y2+BOUND_Y1+1)*2, 1), glm::vec3(0.5, 0.5, 1), view);
        //Front
        drawCube(glm::vec3(0, 0, BOUND_Z2+1), glm::vec3((BOUND_X2-BOUND_X1+2), (BOUND_Y2+BOUND_Y1+1)*2, 1), glm::vec3(0, 0, 0.5), view);

        // drawMonkeyBars(glm::vec3(-2, 2, 0), glm::vec3(0.5, 0.5, 0.5), 7, 3, view);
        // drawCylinder(glm::vec3(1, 1, 0), 1, 0.2, glm::vec3(1.0, 0.9, 0), glm::vec3(1, 0, 0), view);
        // drawSphere(glm::vec3(0, 1, 0), 0.5, glm::vec3(0, 0.5, 0.5), view);
        // for (unsigned int i = 0; i < 10; i++)
        // {
        //     drawCube(cubePositions[i], glm::vec3(2 ,2, 1), glm::vec3(1, 0, 0), view);
        // }

        for (int i = 0; i < mesh_group.size(); i++)
        {
            tuple<Mesh, glm::vec3> object = mesh_group.at(i);
            Mesh mesh = get<0>(object);
            glm::vec3 color_vec = get<1>(object);
            mesh.transform(view);
            renderMesh(&mesh, color_vec);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
