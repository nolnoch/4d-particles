/**
 * main.cpp
 *
 *    Created on: Apr 8, 2013
 *   Last Update: Apr 21, 2013
 *  Orig. Author: Wade Burch (nolnoch@cs.utexas.edu)
 *  Contributors: [none]
 */

#include "./helper.hpp"


/*********************************
 * Display
 */

void CrystalDisplay() {
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  CollapseMatrices();

  // OpenGL program
  progCube.enable();
  RenderParticles();
  progCube.disable();

  glFlush();
  glutSwapBuffers();
}


/*********************************
 * Rendering
 */


void RenderParticles() {
  GLuint locLight0;
  GLuint blockBindingLight0 = 1;

  // Load matrices.
  progCube.setUniformMatrix(4, "modelviewMatrix", glm::value_ptr(mModel));
  progCube.setUniformMatrix(4, "projectionMatrix", glm::value_ptr(mProj));

  // Bind the (static) location/properties of the light.
  glBindBuffer(GL_UNIFORM_BUFFER, uboID);
  locLight0 = glGetUniformBlockIndex(progCube.getProgramId(), "Light");
  glUniformBlockBinding(progCube.getProgramId(), locLight0, blockBindingLight0);
  glBindBufferBase(GL_UNIFORM_BUFFER, blockBindingLight0, uboID);

  // Load the VBO.
  glBindBuffer(GL_ARRAY_BUFFER, vboID);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);
  glEnableVertexAttribArray(4);
  glEnableVertexAttribArray(5);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), OFFSET_PTR(0));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), OFFSET_PTR(12));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), OFFSET_PTR(24));
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), OFFSET_PTR(32));
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), OFFSET_PTR(44));
  glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), OFFSET_PTR(56));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(iboID));
  glDrawElements(GL_TRIANGLES, particles.size(), GL_UNSIGNED_INT, OFFSET_PTR(0));

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  glDisableVertexAttribArray(3);
  glDisableVertexAttribArray(4);
  glDisableVertexAttribArray(5);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}


/*********************************
 * Interaction
 */

/**
 * Disc rotation with a slide: orbit left/right, translate up/down.
 */
void MouseClick(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      orbitAnchor = glm::vec3(x, WIN_HEIGHT - y, vEye.z);
      orbitDest = glm::vec3(x, WIN_HEIGHT - y, vEye.z);
      stateOrbiting = true;
    } else {
      stateOrbiting = false;
    }
  } else if (button == GLUT_RIGHT_BUTTON) {
    // Right click action?

  } else if (button == 3 || button == 4) {
    float step = vEye.z / 15.0f;

    vEye.z += (button == 4) ? step : -step;
  }

  glutPostRedisplay();
}

/**
 * Disc rotation with a slide: orbit left/right, translate up/down.
 */
void MouseMotion(int x, int y) {
  if (stateOrbiting) {
    orbitAnchor = orbitDest;
    orbitDest = glm::vec3(x, WIN_HEIGHT - y, vEye.z);

    // Vertical Slide
    if (orbitAnchor.y != orbitDest.y) {
      float slideFactor = vEye.z / WIN_WIDTH;
      GLfloat slideDist = slideFactor * (orbitDest.y - orbitAnchor.y);
      mTrans = glm::translate(mTrans, glm::vec3(0.0, slideDist, 0.0));
    }

    // Horizontal Orbit
    if (orbitAnchor.x != orbitDest.x) {
      float signAxis = orbitAnchor.x < orbitDest.x ? 1.0f : -1.0f;
      glm::vec3 orbitAxis = glm::vec3(0.0, signAxis, 0.0);
      GLfloat orbitAngle = FindRotationAngle(orbitDest, orbitAnchor);
      Quaternion qOrbitRot = Quaternion(orbitAngle, orbitAxis, RAD);
      qTotalRotation = qOrbitRot * qTotalRotation;
    }
  }

  glutPostRedisplay();
}

/**
 * Zoom to/from cursor on mouse-wheel scroll.
 */
void MouseWheel(int wheel, int direction, int x, int y) {
  cout << "Wheel signal received." << endl;

  // This function, although registered, is not being called.
}

void Keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 'r':
      CameraInit();
      MatrixInit();
      glutPostRedisplay();
      break;
    case 'q':
    case 27:
      exit(0);
      break;
    default:
      break;
  }
}

void Idle() {
  // Currently not registered. Needed?

  glutPostRedisplay();
}


/*********************************
 * Init Functions
 */

void ParticleInit() {
  for (int w = 0; w < 5; w++) {
    for (int z = P_SPACE + P_RADIUS; z < P_CUBE_WIDTH; z += P_INIT_STEP) {
      for (int y = P_SPACE + P_RADIUS; y < P_CUBE_WIDTH; y += P_INIT_STEP) {
        for (int x = P_SPACE + P_RADIUS; x < P_CUBE_WIDTH; x += P_INIT_STEP) {
          Particle p;
          p.color = glm::vec4(0.2f * w);
          p.startLoc = glm::vec4(x, y, z, w);
          p.position = p.startLoc;
          particles.push_back(p);
          iboArray.push_back(iboIdx++);
        }
      }
    }
  }
}

void OpenCLInit() {
  cl_int errorCode;

  // Redirect Vertex Buffer to OpenCL
  clVBObuffer = clCreateFromGLBuffer(clContext, CL_MEM_READ_WRITE, vboID, &errorCode);
  if (errorCode != CL_SUCCESS) {
    exit(ProcessErrorCL(errorCode));
  }
  clQueue = clCreateCommandQueue(clContext, clDeviceId, 0, &errorCode);
  if (errorCode != CL_SUCCESS) {
    exit(ProcessErrorCL(errorCode));
  }

  // Specify CL program to execute on kernel and release object.
}

void BufferInit() {
  GLint nVBO = particles.size();
  GLfloat align = 0.0f;

  // Vertex Buffer Object
  glGenBuffers(1, &vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vboID);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Particle)*nVBO, NULL, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle)*nVBO, &particles[0]);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), OFFSET_PTR(0));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), OFFSET_PTR(12));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), OFFSET_PTR(24));
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), OFFSET_PTR(32));
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), OFFSET_PTR(44));
  glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), OFFSET_PTR(56));

  // Uniform Buffer Object
  GLfloat uLight0[16] = { light_position.x, light_position.y, light_position.z, align,
                          light_ambient.x, light_ambient.y, light_ambient.z, align,
                          light_diffuse.x, light_diffuse.y, light_diffuse.z, align,
                          light_specular.x, light_specular.y, light_specular.z, align
                        };
  glGenBuffers(1, &uboID);
  glBindBuffer(GL_UNIFORM_BUFFER, uboID);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(uLight0), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(uLight0), uLight0);

  // Index Buffer Objects
  glGenBuffers(1, iboID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(iboID));
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, iboArray.size() * sizeof(GLuint),
      &iboArray[0], GL_STATIC_DRAW);
}

void ShaderInit() {
  GLint texLoad;

  progCube.addShader("shader0.vert", GL_VERTEX_SHADER);
  progCube.addShader("shader0.frag", GL_FRAGMENT_SHADER);
  progCube.init();
  progCube.bindAttribute(0, "vertexLocation");
  progCube.bindAttribute(3, "vertexMatDiffuse");
  progCube.bindAttribute(4, "vertexMatSpecular");
  progCube.bindAttribute(5, "vertexShininess");
  progCube.linkAndValidate();
}

void OpenGLInit() {
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LEQUAL);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  glEnable(GL_NORMALIZE);
  glEnable(GL_RESCALE_NORMAL);

  // View/Projection
  fovy = 40.0f;
  aspect = WIN_WIDTH / static_cast<float>(WIN_HEIGHT);
  zNear = 1.0f;
  zFar = 800.0f;

  stateOrbiting = false;
  iboIdx = 0;

  CameraInit();
  MatrixInit();
}

int main(int argc, char* argv[]) {
  // Initialize freeglut
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
  glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
  glutInitWindowPosition(50, 50);

  glutCreateWindow("Crystal-Water");
  glutDisplayFunc(CrystalDisplay);
  glutMouseFunc(MouseClick);
  glutMotionFunc(MouseMotion);
  glutMouseWheelFunc(MouseWheel);
  glutKeyboardFunc(Keyboard);
  glutIdleFunc(NULL);

  // Initialize GLEW
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    cout << "GLEW initialization failed. Aborting program..." << endl;
    return -1;
  }

  // Initialize OpenCL
  clGetPlatformIDs(1, &clPlatformId, NULL);
  clGetDeviceIDs(clPlatformId, CL_DEVICE_TYPE_GPU, 1, &clDeviceId, NULL);
  cl_context_properties properties[] = {
      CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext(),
      CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay(),
      CL_CONTEXT_PLATFORM, (cl_context_properties) clPlatformId, 0
  };
  cl_int contextError;
  clContext = clCreateContext(properties, 1, &clDeviceId, NULL, NULL, &contextError);
  if (contextError != CL_SUCCESS) {
    return ProcessErrorCL(contextError);
  }

  OpenGLInit();
  ShaderInit();
  BufferInit();
//  OpenCLInit();

  glutMainLoop();

  return 0;
}
