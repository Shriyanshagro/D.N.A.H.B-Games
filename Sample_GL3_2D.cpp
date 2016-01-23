#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

// void initGLUT (argc, argv, width, height);
// void addGLUTMenus ();
// void initGL (width, height);
// void glutMainLoop ();
// void keyboardDown(unsigned char key, int x, int y);
// void keyboardUp(unsigned char key, int x, int y);
// void keyboardSpecialUp(int key, int x, int y);
// void keyboardSpecialDown(int 2  key, int x, int y);

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float bird1_rot_dir = 1;
float bird2_rot_dir = 1;
float bird3_rot_dir = 1;
float rectangle_rot_dir = 1;
float canon_rot_dir = 1;
bool bird1_rot_status = false;
bool bird2_rot_status = false;
bool bird3_rot_status = false;
bool rectangle_rot_status = true;
bool canon_rot_status = true;
float canon_rotate_angle = 0 ;

/* Executed when a regular key is pressed */
void keyboardDown (unsigned char key, int x, int y)
{
    switch (key) {
        case 'Q':
        case 'q':
        case 27: //ESC
            exit (0);
        default:
            break;
    }
}

/* Executed when a regular key is released */
void keyboardUp (unsigned char key, int x, int y)
{
    switch (key) {
        case 'c':
        case 'C':
            rectangle_rot_status = !rectangle_rot_status;
            break;
        // case 'p':
        // case 'P':
        //     triangle_rot_status = !triangle_rot_status;
        //     break;
        case 'w':
        case 'W':
            canon_rot_status = ! canon_rot_status;
            break;
        case 'a':
        case 'A':
            canon_rot_dir = -1;
        break;
        case 'd':
        case 'D':
            canon_rot_dir = 1;
        break;
        default:
            break;
    }
}

/* Executed when a special key is pressed */
void keyboardSpecialDown (int key, int x, int y)
{
}

/* Executed when a special key is released */
void keyboardSpecialUp (int key, int x, int y)
{
}

/* Executed when a mouse button 'button' is put into state 'state'
 at screen position ('x', 'y')
 */
void mouseClick (int button, int state, int x, int y)
{
    switch (button) {
    //     case GLUT_LEFT_BUTTON:
    //         if (state == GLUT_UP)
    //             triangle_rot_dir *= -1;
    //         break;
        case GLUT_RIGHT_BUTTON:
            if (state == GLUT_UP) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}

/* Executed when the mouse moves to position ('x', 'y') */
void mouseMotion (int x, int y)
{
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (int width, int height)
{
	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);

	// set the projection matrix as perspective/ortho
	// Store the projection matrix in a variable for future use

    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) width / (GLfloat) height, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *bird1,*bird2,*bird3,*canon, *rectangle;

int i=0;
GLfloat vertex_buffer_data [500] ;
GLfloat color_buffer_data [500] ;
void add(GLdouble x,GLdouble y)
{
    vertex_buffer_data[i]=x;
    color_buffer_data[i++]=0.184314  ;
    vertex_buffer_data[i]=y;
    color_buffer_data[i++]=0.309804;
    vertex_buffer_data[i]=0;
    color_buffer_data[i++]=0.3098040;

}
// Creates the triangle object used in this sample code
void create_angry_bird (GLdouble centrex,GLdouble centrey)
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    const double TWO_PI = 6.2831853;
    GLdouble hexTheta,x,y,radius=.30,previousx,previousy;
    // adds point to the vertex_buffer array

    hexTheta = TWO_PI * 0/40;
    x = centrex + radius * cos(hexTheta);
    y = centrey + radius * sin(hexTheta);
    add(centrex,centrey);
    add(x,y);
    hexTheta = TWO_PI * 1/20;
    x = centrex + radius * cos(hexTheta);
    y = centrey + radius * sin(hexTheta);
    add(x,y);
    previousy = y;
    previousx = x;

    int j;
    // building many triangles to form one circle
    for(j=2;j<=20;j++)
    {
        hexTheta = TWO_PI * j/20;
        // defining the new vertices
        x = centrex + radius * cos(hexTheta);
        y = centrey + radius * sin(hexTheta);
        // assigining vertices to new triangle
        add(centrex,centrey);
        add(previousx,previousy);
        add(x,y);
        previousy = y;
        previousx = x;
    }

  // create3DObject creates and returns a handle to a VAO that can be used later
  bird1 = create3DObject(GL_TRIANGLES, 180, vertex_buffer_data, color_buffer_data, GL_FILL);
  bird2 = create3DObject(GL_TRIANGLES, 180, vertex_buffer_data, color_buffer_data, GL_FILL);
  bird3 = create3DObject(GL_TRIANGLES, 180, vertex_buffer_data, color_buffer_data, GL_FILL);
  i=0;
}

void createcanon (GLdouble centrex,GLdouble centrey)
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */
    const double TWO_PI = 6.2831853;
    GLdouble hexTheta,x,y,radius=.50,previousx,previousy;
    // adds point to the vertex_buffer array

    hexTheta = TWO_PI * 0/20;
    x = centrex + radius * cos(hexTheta);
    y = centrey + radius * sin(hexTheta)+ canon_rotate_angle;
    add(0,0);
    add(x,y);
    hexTheta = TWO_PI * 1/20;
    x = centrex + radius * cos(hexTheta);
    y = centrey + radius * sin(hexTheta)+ canon_rotate_angle;
    add(x,y);
    previousy = y;
    previousx = x;

    int j;
    // building many triangles to form one circle
    for(j=2;j<=20;j++)
    {
        hexTheta = TWO_PI * j/20;
        // defining the new vertices
        x = centrex + radius * cos(hexTheta);
        y = centrey + radius * sin(hexTheta)+ canon_rotate_angle;
        // assigining vertices to new triangle
        add(0,0);
        add(previousx,previousy);
        add(x,y);
        previousy = y;
        previousx = x;
    }

  // create3DObject creates and returns a handle to a VAO that can be used later
  canon = create3DObject(GL_TRIANGLES, 180, vertex_buffer_data, color_buffer_data, GL_LINE);
  i=0;
}

void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float bird1_rotation = 0;
float bird2_rotation = 0;
float bird3_rotation = 0;
float canon_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  /* Render your scene */

  // bird1
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translatebird1 = glm::translate (glm::vec3(0.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotatebird1 = glm::rotate((float)(bird1_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0) , vec3 decides the axis about which it have to be rotated
  glm::mat4 bird1Transform = translatebird1 ;
  Matrices.model *= translatebird1 * rotatebird1;
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(bird1);

  // bird2
  Matrices.model = glm::mat4(1.0f);


  glm::mat4 translatebird2 = glm::translate (glm::vec3(1.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotatebird2 = glm::rotate((float)(bird2_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0) , vec3 decides the axis about which it have to be rotated
  glm::mat4 bird2Transform = translatebird2 ;
  Matrices.model *= translatebird2 * rotatebird2;
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(bird2);

    // bird3
  Matrices.model = glm::mat4(1.0f);


  glm::mat4 translatebird3 = glm::translate (glm::vec3(-1.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotatebird3 = glm::rotate((float)(bird3_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0) , vec3 decides the axis about which it have to be rotated
  glm::mat4 bird3Transform = translatebird3 ;
  Matrices.model *= translatebird3 * rotatebird3;
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(bird3);

  // rectangle
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);

  // canon
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translatecanon = glm::translate (glm::vec3(-3, -3, 0));        // glTranslatef
  glm::mat4 rotatecanon = glm::rotate((float)(canon_rotation*M_PI/80.0f), glm::vec3(0,0,1)); // rotate about vector (0,0,1)
  Matrices.model *= (translatecanon * rotatecanon);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(canon);

  // Swap the frame buffers
  glutSwapBuffers ();

  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  bird1_rotation = bird1_rotation + increments*bird1_rot_dir*bird1_rot_status;
  bird2_rotation = bird2_rotation + increments*bird2_rot_dir*bird2_rot_status;
  bird3_rotation = bird3_rotation + increments*bird3_rot_dir*bird3_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
  canon_rotation = canon_rotation + (increments/5)*canon_rot_dir*canon_rot_status*-1;
}

/* Executed when the program is idle (no I/O activity) */
void idle () {
    // OpenGL should never stop drawing
    // can draw the same scene or a modified scene
    draw (); // drawing same scene
}

void initGL (int width, int height );
/* Initialise glut window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
void initGLUT (int& argc, char** argv, int width, int height)
{
    // Init glut
    glutInit (&argc, argv);

    // Init glut window
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3); // Init GL 3.3
    glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
    glutInitWindowSize (width, height);
    glutCreateWindow ("Sample OpenGL3.3 Application");

    // Initialize GLEW, Needed in Core profile
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
        exit (1);
    }

    // register glut callbacks
    glutKeyboardFunc (keyboardDown);
    glutKeyboardUpFunc (keyboardUp);

    glutSpecialFunc (keyboardSpecialDown);
    glutSpecialUpFunc (keyboardSpecialUp);

    glutMouseFunc (mouseClick);
    glutMotionFunc (mouseMotion);

    // initGL(width, height);
    glutReshapeFunc (reshapeWindow);

    glutDisplayFunc (draw); // function to draw when active
    glutIdleFunc (idle); // function to draw when idle (no I/O activity)

    glutIgnoreKeyRepeat (true); // Ignore keys held down
}

/* Process menu option 'op' */
void menu(int op)
{
    switch(op)
    {
        case 'Q':
        case 'q':
            exit(0);
    }
}

void addGLUTMenus ()
{
    // create sub menus
    int subMenu = glutCreateMenu (menu);
    glutAddMenuEntry ("Do Nothing", 0);
    glutAddMenuEntry ("Really Quit", 'q');

    // create main "middle click" menu
    glutCreateMenu (menu);
    glutAddSubMenu ("Sub Menu", subMenu);
    glutAddMenuEntry ("Quit", 'q');
    glutAttachMenu (GLUT_MIDDLE_BUTTON);
}


/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (int width, int height )
{
	// Create the models
	create_angry_bird (0,0); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();
    createcanon (1,1); // pointed at -3   .5,-3
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (width, height);

	// Background color of the scene
	glClearColor (0.56 , 0.56 , 0.56, 0.0f); // R, G, B, A
	glClearDepth (0.5f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);


	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;

    initGLUT (argc, argv, width, height);

    addGLUTMenus ();

	initGL (width, height);

    glutMainLoop ();

    return 0;
}
