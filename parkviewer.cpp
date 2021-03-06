#include "parkviewer.h"

static Camera camera(SCREEN_WIDTH, SCREEN_HEIGHT, x_min, x_max, y_min, y_max, z_min, z_max);

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.scroll_callback(window, xoffset, yoffset);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    camera.mouse_callback(window, xpos, ypos);
}

void drawGenericObject(GLuint &VAO, GLuint programID,
                        glm::mat4 proj,
                        glm::mat4 view,
                        int size,
                        bool elemental,
                        glm::vec3 translationVector,
                        glm::vec3 scaleVector,
                        GLfloat rotationAngle,
                        glm::vec3 rotationAxis)
{
    // Must match the name specified in the vertex shader.
    GLuint matrixID = glGetUniformLocation(programID, "MVP");
    GLuint modelID = glGetUniformLocation(programID, "model");
    GLuint lightID = glGetUniformLocation(programID, "LightPos");

    // Binding the vertex array is equivalent to setting a global variable for the rest of the gl calls here
    glBindVertexArray(VAO);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, translationVector);
    model = glm::scale(model, scaleVector);
    model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);
    glm::mat4 MVP = proj*view*model;

    // Pass our arrays over to the vertexshader for further transformations
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
    glUniform3fv(lightID, 1, &lightPos[0]);
    if (elemental) {
        glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, size*3);
    }
    // Unbinds the vertex array
    glBindVertexArray(0);
}

bool loadAssImp(const char * path, std::vector<unsigned short> &indices,
                std::vector<glm::vec3> &vertices,
                std::vector<glm::vec3> &normals) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 0);
    const aiMesh* mesh = scene->mMeshes[0];
    vertices.reserve(mesh->mNumVertices);
    for(ulong64_t i = 0; i < mesh->mNumVertices; i++){
        aiVector3D pos = mesh->mVertices[i];
        vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
    }
    normals.reserve(mesh->mNumVertices);
    for(ulong64_t i = 0; i < mesh->mNumVertices; i++){
        aiVector3D n = mesh->mNormals[i];
        normals.push_back(glm::vec3(n.x, n.y, n.z));
    }
    indices.reserve(3*mesh->mNumFaces);
    for (ulong64_t i=0; i<mesh->mNumFaces; i++){
        indices.push_back(mesh->mFaces[i].mIndices[0]);
        indices.push_back(mesh->mFaces[i].mIndices[1]);
        indices.push_back(mesh->mFaces[i].mIndices[2]);
    }
    return true;
}

bool initOpenGL()
{
    // Set OpenGL version to 3.3
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    return true;
}

void setupMeshVAO(Mesh mesh, GLfloat* color_vector, vector<ObjectData> &objectVector)
{
    ObjectData object;

    vector<GLfloat> v = mesh.getVertices();
    GLfloat* vertices = &v[0];
    object.indexSize = v.size()/3; //# of vertices = arraysize/3 (x,y,z)
    int size = v.size()*sizeof(GLfloat);

    vector<glm::vec3> normals = mesh.getNormals();
    GLfloat ModelNormalArray[normals.size()*3];

    ulong64_t i = 0;
    // Working with a regular array is easier than working with vectors for passing on to the VS/FS
    for (auto it = normals.begin(); it != normals.end(); it++) {
        ModelNormalArray[i++] = it->x;
        ModelNormalArray[i++] = it->y;
        ModelNormalArray[i++] = it->z;
    }

    // We are only generating one VA at a time for each object
    glGenVertexArrays(1, &(object.ModelArrayID));
    glBindVertexArray(object.ModelArrayID);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // Separate buffers are created for the vertex buffer, color buffer, and normal buffer of the 3D object
    // These buffers store the data and forward it on to the GPU to be processed further by the VS/FS
    glGenBuffers(1, &(object.ModelVBO));
    glBindBuffer(GL_ARRAY_BUFFER, object.ModelVBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glGenBuffers(1, &(object.ModelColorVBO));
    glBindBuffer(GL_ARRAY_BUFFER, object.ModelColorVBO);
    glBufferData(GL_ARRAY_BUFFER, size, color_vector, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glGenBuffers(1, &(object.ModelNormalVBO));
    glBindBuffer(GL_ARRAY_BUFFER, object.ModelNormalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ModelNormalArray), ModelNormalArray, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);

    objectVector.push_back(object);
}

void setCallBacks(GLFWwindow* window)
{
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
}

void generateModelVAO(string path, ObjectData &object, const GLfloat* color_array)
{
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;

    if(loadAssImp(path.c_str(), indices, vertices, normals)) {
        GLfloat* ModelVertexArray = new GLfloat[vertices.size()*3];
        GLfloat* ModelColorArray = new GLfloat[vertices.size()*3];
        GLfloat* ModelNormalArray = new GLfloat[normals.size()*3];

        unsigned int indexList[indices.size()];

        ulong64_t i = 0;
        for (auto it = vertices.begin(); it != vertices.end(); it++) {
            ModelVertexArray[i++] = it->x;
            ModelVertexArray[i++] = it->y;
            ModelVertexArray[i++] = it->z;
        }
        cout << path << i;
        i = 0;
        for (auto it = normals.begin(); it != normals.end(); it++) {
            ModelNormalArray[i++] = it->x;
            ModelNormalArray[i++] = it->y;
            ModelNormalArray[i++] = it->z;
        }

        for (ulong64_t j = 0; j < indices.size(); j++) {
            indexList[j] = indices[j];
        }

        for (ulong64_t j = 0; j < i;) {
            for (int ctr = 0; ctr < 9; ctr++) {
                ModelColorArray[j++] = color_array[ctr];
            }
        }

        object.indexSize = indices.size();

        glGenVertexArrays(1, &(object.ModelArrayID));
        glBindVertexArray(object.ModelArrayID);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        glGenBuffers(1, &(object.ModelVBO));
        glBindBuffer(GL_ARRAY_BUFFER, object.ModelVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*3*sizeof(GLfloat),
                    ModelVertexArray, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glGenBuffers(1, &(object.ModelColorVBO));
        glBindBuffer(GL_ARRAY_BUFFER, object.ModelColorVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*3*sizeof(GLfloat),
                    ModelColorArray, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glGenBuffers(1, &(object.ModelNormalVBO));
        glBindBuffer(GL_ARRAY_BUFFER, object.ModelNormalVBO);
        glBufferData(GL_ARRAY_BUFFER, normals.size()*3*sizeof(GLfloat),
                    ModelNormalArray, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glGenBuffers(1, &(object.EBO));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexList),
                    indexList, GL_STATIC_DRAW);

        glBindVertexArray(0);

        // We wouldn't want memory leaks, would we?
        // Once we have pushed the data to the GPU, we can safely delete the data
        delete[] ModelVertexArray;
        delete[] ModelColorArray;
        delete[] ModelNormalArray;
    }
}

int main()
{
    if(!glfwInit()) {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return false;
    }
    if (!initOpenGL()) {
        return -1;
    }

    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cubes!", glfwGetPrimaryMonitor(), NULL);
    if(window == NULL){
        glfwTerminate();
        return -1;
    }
    setCallBacks(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return false;
    }

    GLuint ModelArrayID, ModelVBO, ModelColorVBO, EBO, indexSize;
    ObjectData swing, carousel, swingChair, cg;
    ObjectData coaster_track1, coaster_track2, coaster_track3, coaster_cart,
                coaster_glasses, coaster_head, coaster_mouth;
    generateModelVAO("swing_seat.obj", swingChair, swing_chair_color);
    generateModelVAO("swing_frame.obj", swing, swing_frame_color);
    generateModelVAO("1simple_round.obj", carousel, carousel_color);
    generateModelVAO("coaster_cart.obj", coaster_cart, cart_color);
    generateModelVAO("coaster_track1.obj", coaster_track1, track_color);
    generateModelVAO("coaster_track2.obj", coaster_track2, track_color);
    generateModelVAO("coaster_track3.obj", coaster_track3, track_color);
    generateModelVAO("coaster_glasses.obj", coaster_glasses, glasses_color);
    generateModelVAO("coaster_head.obj", coaster_head, monkey_color);
    generateModelVAO("coaster_mouth.obj", coaster_mouth, mouth_color);

    // Sky color
    glClearColor(1.0f, 0.8f, 0.45f, 0.9f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");

    glm::mat4 proj;
    glm::mat4 view;
    Scene scene = Scene();

    glm::mat4 T;

    scene.addSeeSaw(glm::vec3(10, 0, 6));

    T = glm::rotate(glm::mat4(1.0), (float)glm::radians(150.0), glm::vec3(0, 1, 0));
    scene.addSlide(glm::vec3(-5, 0, 8), glm::vec3(0.6, 0.3, 0.1), 4, T);

    T = glm::rotate(glm::mat4(1.0), (float)glm::radians(90.0), glm::vec3(0, 1, 0));
    scene.addMonkeyBars(glm::vec3(-8, 0, 11), glm::vec3(.0, 1.0, 0.2), 7, 3, T);

    scene.addPath(glm::vec2(-1, 0), glm::vec2(1, 20), glm::vec3(0.2, 0.2, 0.2));
    scene.addPath(glm::vec2(-19, 0), glm::vec2(19, 2), glm::vec3(0.2, 0.2, 0.2));

    scene.addFence(glm::vec4(x_min, x_max, z_min, z_max));
    scene.addFloor(glm::vec4(x_min, x_max, z_min, z_max));

    scene.addSculpture();

    vector<Mesh> mesh_group = scene.getMesh();
    vector<vector<GLfloat>> color_vector_group = scene.getColors();

    vector<ObjectData> sceneMesh;

    for (int i = 0; i < mesh_group.size(); i++) {
        setupMeshVAO(mesh_group.at(i), &color_vector_group.at(i)[0], sceneMesh);
    }

    while(glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && !glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        if (SHOW_FPS) {
            cout << "FPS: " << 1.0f/deltaTime << endl;
        }
        lastFrame = currentFrame;

        // Used for capturing the WASD keys and mouse
        camera.processInput(window, deltaTime);
        // Defines what can be seen by the camera along with the clip boundaries of the scene
        proj = glm::perspective(glm::radians(camera.getFOV()), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.2f, 100.0f);
        view = camera.getCameraView();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    	glUseProgram(programID);

        drawGenericObject(carousel.ModelArrayID, programID, proj, view,
                            carousel.indexSize, true, carouselPosition, carouselScale,
                            (float)glfwGetTime()*45.0f, glm::vec3(0,1,0));
        drawGenericObject(swing.ModelArrayID, programID, proj, view,
                            swing.indexSize, true, swingPosition, swingScale);
        drawGenericObject(swingChair.ModelArrayID, programID, proj, view,
                            swingChair.indexSize, true, swingChairPosition, swingChairScale);
        drawGenericObject(coaster_cart.ModelArrayID, programID, proj, view,
                            coaster_cart.indexSize, true, coasterPosition, coasterScale);
        drawGenericObject(coaster_track1.ModelArrayID, programID, proj, view,
                            coaster_track1.indexSize, true, coasterPosition, coasterScale);
        drawGenericObject(coaster_track2.ModelArrayID, programID, proj, view,
                            coaster_track2.indexSize, true, coasterPosition, coasterScale);
        drawGenericObject(coaster_track3.ModelArrayID, programID, proj, view,
                            coaster_track3.indexSize, true, coasterPosition, coasterScale);
        drawGenericObject(coaster_mouth.ModelArrayID, programID, proj, view,
                            coaster_mouth.indexSize, true, coasterPosition, coasterScale);
        drawGenericObject(coaster_head.ModelArrayID, programID, proj, view,
                            coaster_head.indexSize, true, coasterPosition, coasterScale);
        drawGenericObject(coaster_glasses.ModelArrayID, programID, proj, view,
                            coaster_glasses.indexSize, true, coasterPosition, coasterScale);

        for (auto it = sceneMesh.begin(); it != sceneMesh.end(); it++) {
            drawGenericObject(it->ModelArrayID, programID, proj, view, it->indexSize, false);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
	glDeleteProgram(programID);

	glfwTerminate();
    return 0;
}
