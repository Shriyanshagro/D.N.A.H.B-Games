#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <string.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct Point{
    float x;
    float y;
};
typedef struct Point Point;

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
int num_coin =6 ;
float camera_rotation_angle = 90;
float bird1_rot_dir = 1;
float bird2_rot_dir = -1;
float bird3_rot_dir = 1;
float rectangle_rot_dir = 1;
float rectangle2_rot_dir = 1;
float rectangle3_rot_dir = 1;
float rectangle4_rot_dir = 1;
float rectangle5_rot_dir = 1;
float coins_rot_dir[500] ;
float canon_rot_dir = 1;
bool bird1_rot_status = true;
bool bird2_rot_status = false;
bool bird3_rot_status = true;
bool coins_rot_status[500];
bool rectangle_rot_status = false;
bool rectangle2_rot_status = false;
bool rectangle3_rot_status = false;
bool rectangle4_rot_status = false;
bool rectangle5_rot_status = false;
bool canon_rot_status = false;
float rectangle_rotation = 0;
float rectangle2_rotation = 0;
float rectangle3_rotation = 0;
float rectangle4_rotation = 0;
float rectangle5_rotation = 0;
float bird1_rotation = 0;
float bird2_rotation = 0;
float bird3_rotation = 0;
float coins_rotation[500];
float canon_rotation = 20;
float power = 8 ;
float power_meter = 8 ;
bool shoot = false ;
float flying_time = -987 ;
float newx =0 ;
float newy =0 ;
double o=0;
double theta = 0 ;
double collisionx = 0;
double collisiony = 0;
double accelarationx = 0;
double accelarationy =0 ;
double gravity = -10 ;
double water_friction = 0.5;
double ground_friction = 0.2;
double ux =0 ;
double vx=0;
double uy =0 ;
double vy =0;
double finalx =0 ;
double finaly =0 ;
double direction =9 ;
double moving_wheelx =0;
double energy =0;
double ex=0; // friction_coefficient at x axis
double ey=0; // friction coefficient at y axis
double radius_coins=.10f; // radius of angry coins
double radius_object=.30f ;  // radius of angryobject
double centerx_coin[500] ;
double centery_coin[500] ;
int score=0;
bool flag_coin[500] ;
double zoom =0 ;
float panx=0;
float pany=0;
float mousex=0;
float mousey=0;
Point *mousePos;
int level=1;
double trappy=0;
double dir=1; // direction of trappy box

void shoot_func(){
    shoot = true ;
    flying_time = -987;
    theta = (canon_rotation)*M_PI/180.0f ;
    power = power_meter ;
    collisionx =0 ;
    newy =0 ;
    newx =0 ;
    collisiony =0 ;
    ux = power*cos(theta);
    vx=0;
    uy = power*(sin(theta));
    vy =0;
    // flag_coin[6] = {true,true,true,true,true,true};
    o=0;
    return;
}

void reshapeWindow(int width,int height);

void exit_func(){
    cout<<"Your final Score is "<<score<<endl;
    cout<<"Level = "<<level<<endl;
    cout<<"Thanks For Playing"<<endl;
    exit (0);
}
/* Executed when a regular key is pressed */
void keyboardUp (unsigned char key, int x, int y)
{
    switch (key) {
        case 'Q':
        case 'q':
        case 27: //ESC
            exit_func();
        default:
            break;
    }
}

/* Executed when a regular key is released */
void keyboardDown (unsigned char key, int x, int y)
{
    switch (key) {
        case 'a':
        case 'A':
            if(canon_rotation >=3)
            canon_rotation += -3;
        break;
        case 'd':
        case 'D':
            if(canon_rotation <177)
            canon_rotation += 3;
        break;
        case 'w':
        case 'W':
            power_meter +=0.5f;
        break;
        case 's':
        case 'S':
            power_meter -=0.5f;
        break;
        case 32:
            shoot_func();
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
    switch(key){
            case GLUT_KEY_UP:
                if(zoom<=1)
                {
                    zoom+=0.2f;
                    reshapeWindow(600,600);
                }
            break;
            case GLUT_KEY_DOWN:
            if(zoom>=-1)
            {
                zoom-=0.2f;
                reshapeWindow(600,600);
            }
            break;
            case GLUT_KEY_RIGHT:
                if(panx<=35)
                {
                    panx+=3.5;
                    reshapeWindow(600,600);
                }
            break;
            case GLUT_KEY_LEFT:
                if(panx>=-35)
                {
                    panx-=3.5;
                    reshapeWindow(600,600);
                }
            break;
            case 32:
                // if(shoot == false){
                shoot = true ;
                flying_time = -987;
                theta = (canon_rotation)*M_PI/180.0f ;
                power = power_meter ;
                collisionx =0 ;
                newy =0 ;
                newx =0 ;
                collisiony =0 ;
                ux = power*cos(theta);
                vx=0;
                uy = power*(sin(theta));
                vy =0;
                // flag_coin[6] = {true,true,true,true,true,true};
                o=0;
            // }
            break;

    }
}

/* Executed when a mouse button 'button' is put into state 'state'
 at screen position ('x', 'y')
 */
void mouseClick (int button, int state, int x, int y)
{
    switch (button) {
        case GLUT_LEFT_BUTTON:
            shoot_func();
        break;
        case GLUT_RIGHT_BUTTON:
            if(panx<=35)
            {
                panx+=3.5;
                reshapeWindow(600,600);
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
	glViewport (0+panx, 0+pany, (GLsizei) width+5, (GLsizei) height);

	// set the projection matrix as perspective/ortho
	// Store the projection matrix in a variable for future use

    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) width / (GLfloat) height, 0.1f, 500.0f);

    // Ortho projection for 2D views
    float x = -4.0f - zoom ;
    float y = 4.0f + zoom;
    Matrices.projection = glm::ortho(x, y, x, y, 0.1f, 500.0f);
}

VAO *bird1,*bird2,*bird3,*canon, *rectangle , *rectangle2 , *rectangle3 , *rectangle4 ,*rectangle5,*rectangle6,*coins[500];
// VAO coins[] = malloc(num_coin*sizeof(VAO));

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

void add_coins(GLdouble x,GLdouble y)
{
    vertex_buffer_data[i]=x;
    color_buffer_data[i++]= 0.8  ;
    vertex_buffer_data[i]=y;
    color_buffer_data[i++]=0.498039;
    vertex_buffer_data[i]=0;
    color_buffer_data[i++]=0.196078;

}

// Creates the triangle object used in this sample code
void create_angry_bird (GLdouble centrex,GLdouble centrey)
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    const double TWO_PI = 6.2831853;
    GLdouble hexTheta,x,y,previousx,previousy;
    // adds point to the vertex_buffer array

    hexTheta = TWO_PI * 0/40;
    x = centrex + radius_object * cos(hexTheta);
    y = centrey + radius_object * sin(hexTheta);
    add(centrex,centrey);
    add(x,y);
    hexTheta = TWO_PI * 1/20;
    x = centrex + radius_object * cos(hexTheta);
    y = centrey + radius_object * sin(hexTheta);
    add(x,y);
    previousy = y;
    previousx = x;

    int j;
    // building many triangles to form one circle
    for(j=2;j<=20;j++)
    {
        hexTheta = TWO_PI * j/20;
        // defining the new vertices
        x = centrex + radius_object * cos(hexTheta);
        y = centrey + radius_object * sin(hexTheta);
        // assigining vertices to new triangle
        add(centrex,centrey);
        add(previousx,previousy);
        add(x,y);
        previousy = y;
        previousx = x;
    }

  // create3DObject creates and returns a handle to a VAO that can be used later
  bird1 = create3DObject(GL_TRIANGLES, 180, vertex_buffer_data, color_buffer_data, GL_LINE); // moving_wheel
  bird2 = create3DObject(GL_TRIANGLES, 180, vertex_buffer_data, color_buffer_data, GL_FILL); //canon's wheel
  bird3 = create3DObject(GL_TRIANGLES, 180, vertex_buffer_data, color_buffer_data, GL_LINE); // angry_bird
  i=0;
}

void create_angry_coins (GLdouble centrex,GLdouble centrey)
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    const double TWO_PI = 6.2831853;
    GLdouble hexTheta,x,y,previousx,previousy;
    // adds point to the vertex_buffer array

    hexTheta = TWO_PI * 0;
    x = centrex + radius_coins * cos(hexTheta);
    y = centrey + radius_coins * sin(hexTheta);
    add_coins(centrex,centrey);
    add_coins(x,y);
    hexTheta = TWO_PI * 1/10;
    x = centrex + radius_coins * cos(hexTheta);
    y = centrey + radius_coins * sin(hexTheta);
    add_coins(x,y);
    previousy = y;
    previousx = x;

    int j;
    // building many triangles to form one circle
    for(j=2;j<=10;j++)
    {
        hexTheta = TWO_PI * j/10;
        // defining the new vertices
        x = centrex + radius_coins * cos(hexTheta);
        y = centrey + radius_coins * sin(hexTheta);
        // assigining vertices to new triangle
        add_coins(centrex,centrey);
        add_coins(previousx,previousy);
        add_coins(x,y);
        previousy = y;
        previousx = x;
    }

    srand((unsigned)time(0));
  // create3DObject creates and returns a handle to a VAO that can be used later
  for(int r=1;r<=490;r++)
    {
        coins[r]= create3DObject(GL_TRIANGLES, 30, vertex_buffer_data, color_buffer_data, GL_LINE);
        flag_coin[r] = true ;
        coins_rot_dir[r] = -1;
        coins_rotation[r] = 0;
        coins_rot_status[r] = true;
        double temp1 = (rand()%78 -39);
            temp1/= 10;
        double temp2 = (rand()%60 -30);
            temp2/= 10;
        centerx_coin[r] =  temp1;
        // cout<<centerx_coin[r]<<endl;
        centery_coin[r] = temp2;
    }
  i=0;
}


// creates canon
void createcanon (GLdouble centrex,GLdouble centrey)
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */
    const double TWO_PI = 6.2831853;
    GLdouble hexTheta,x,y,radius_canon=.50,previousx,previousy;
    // adds point to the vertex_buffer array

    hexTheta = TWO_PI * 0/20;
    x = centrex + radius_canon * cos(hexTheta);
    y = centrey + radius_canon * sin(hexTheta);
    add(0,0);
    add(x,y);
    hexTheta = TWO_PI * 1/20;
    x = centrex + radius_canon * cos(hexTheta);
    y = centrey + radius_canon * sin(hexTheta);
    add(x,y);
    previousy = y;
    previousx = x;

    int j;
    // building many triangles to form one circle
    for(j=2;j<=20;j++)
    {
        hexTheta = TWO_PI * j/20;
        // defining the new vertices
        x = centrex + radius_canon * cos(hexTheta);
        y = centrey + radius_canon * sin(hexTheta);
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
    0,0,0,
    0,0.2f,0,
    1.2f,0,0,
    1.2f,0,0,
    1.2f,0.2f,0,
    0,0.2f,0,
  };

  static const GLfloat color_buffer_data [] = {
    0.847059,0.847059,0.74902, // color 1
    0.847059,0.847059,0.74902, // color 1
    0.847059,0.847059,0.74902, // color 1
    0.847059,0.847059,0.74902, // color 1
    0.847059,0.847059,0.74902, // color 1
    0.847059,0.847059,0.74902, // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  rectangle2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  rectangle3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// trapping_box
void water_rectangle(){
    const GLfloat vertex_buffer_data [] = {
        0.4,0.3,0,
        0.4,0.7,0,
        0.8,0.3,0,
        0.8,0.3,0,
        0.4,0.7,0,
        0.8,0.7,0,
    };

    static const GLfloat color_buffer_data [] = {
      0.59,0.41,0.31, // color 1
      0.59,0.41,0.31, // color 1
      0.59,0.41,0.31, // color 1
      0.59,0.41,0.31, // color 2
      0.59,0.41,0.31, // color 2
      0.59,0.41,0.31, // color 2
    };



  rectangle4 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void ground_rectangle(){
    const GLfloat vertex_buffer_data [] = {
      -180,-10,0,
      -180,0.2f,0,
      170,-10,0,
      170,-10,0,
      170,0.2f,0,
      -180,0.2f,0,
    };

    static const GLfloat color_buffer_data [] = {
    //   0.647059,0.164706,0.164706, // color 1
    //   0.647059,0.164706,0.164706, // color 1
    //   0.647059,0.164706,0.164706, // color 1
      0.22,0.69,0.87, // color 2
      0.22,0.69,0.87, // color 2
      0.22,0.69,0.87, // color 2
      0.22,0.69,0.87, // color 2
      0.22,0.69,0.87, // color 2
      0.22,0.69,0.87, // color 2
      };



  rectangle5 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


void power_rectangle(){
    const GLfloat vertex_buffer_data [] = {
      -1800,0.1,0,
      -1800,0.2f,0,
      -1,0.1,0,
      -1,0.1,0,
      -1,0.2f,0,
      -1800,0.2f,0,
    };

    static const GLfloat color_buffer_data [] = {
    //   0.647059,0.164706,0.164706, // color 1
    //   0.647059,0.164706,0.164706, // color 1
    //   0.647059,0.164706,0.164706, // color 1
      0.4,0.2,0.2, // color 2
      0.4,0.2,0.2, // color 2
      0.4,0.2,0.2, // color 2
      0.4,0.2,0.2, // color 2
      0.4,0.2,0.2, // color 2
      0.4,0.2,0.2, // color 2
      };



  rectangle6 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


/* Render the scene with openGL */
/* Edit this function according to your assignment */

double calc_dist(GLdouble x1 , GLdouble x2 , GLdouble y1 , GLdouble y2){
    double dist=0;

    x1 = pow(x1-x2,2);
    y1 = pow(y1-y2,2);
    x1 += y1;
    dist =sqrt(x1);

    return dist;
}

void accelaration_func(){

    accelarationx = 0;
    accelarationy = gravity ;
  //   need to include code of water and ground friction

}

void friction_coefficient(){
    if((collisiony + newy ) < 0.005f) // ground
            {
                ex = 1.2f ;
                ey = 1.5f ;
            }
        // left-most rectangle
    else if(((collisiony + newy -3) >= (1.95f-radius_object ) && (collisiony + newy -3) <= (2.21f+radius_object )) && (collisionx + newx -3) >= -1.7f && (collisionx + newx -3) <= -0.50f)
            {
                ex = 1.2f;
                ey =1.2f ;
            }
        // right-most rectangle
    else if(((collisiony + newy -3) >= (1.95f-radius_object ) && (collisiony + newy -3) <= (2.21f+radius_object )) && (collisionx + newx -3) >= 2.4f && (collisionx + newx -3) <= 3.6f)
            {
                ex = 1.2f;
                ey = 1.2f ;
            }
        // rough-ground rectangle
    else if(((collisiony + newy -3) >= (-3.25f-radius_object ) && (collisiony + newy -3) <= (-2.99f+radius_object )) && (collisionx + newx -3) >= 0.7f && (collisionx + newx -3) <= 1.9f)
        {
            ex = 3.0f;
            ey = 2.1f;
        }

}

void move_func(){
      if(shoot == false)
      return ;

      vx = ux + accelarationx*o;
      vy =uy + accelarationy*o;
      energy = pow(vx,2) + pow(vy,2);
      energy = sqrt(energy);
      newx = ux*o + accelarationx*o*o*2;
      newy = uy*o + accelarationy*o*o/2;

}

void collect_coins(){

    double dist=0,x,y;
    x= collisionx+newx-3;
    y= collisiony+newy-3;
    for(int r=1;r<=num_coin;r++)
    {
    dist = calc_dist(x,centerx_coin[r],y,centery_coin[r]);
    if(dist<=(radius_coins+radius_object) && flag_coin[r]==true)
        {
            score++;
            cout<<"Your current Score is "<<score<<endl;
            if((score%num_coin)==0 && score!=0)
                {
                    num_coin +=12;
                    level++;
                    cout<<"Hurray , You are now one level up!! "<<endl;
                    cout<<"Current level is "<<level<<endl;
                }
                if(num_coin > 499)
                    exit_func();
            flag_coin[r]=false;
            break;
        }
    }
    // cout<<num_coin<<endl;

}

void collision_func(){

    if(shoot == false)
    return ;

  // radius_object = radius_object of angryobject
  //   collision in x-plane
    if((collisiony + newy ) < 0.005f // ground
        ||
        // left-most rectangle
        (((collisiony + newy -3) >= (1.95f-radius_object ) && (collisiony + newy -3) <= (2.21f+radius_object )) && (collisionx + newx -3) >= (-1.7f-radius_object+0.08) && (collisionx + newx -3) <= (-0.50f+radius_object-0.08))
        ||
        // right-most rectangle
        (((collisiony + newy -3) >= (1.95f-radius_object ) && (collisiony + newy -3) <= (2.21f+radius_object )) && (collisionx + newx -3) >= (2.4f-radius_object+0.08) && (collisionx + newx -3) <= (3.6f+radius_object-0.08))
        ||
        // rough-ground rectangle
        (((collisiony + newy -3) >= (-3.25f-radius_object ) && (collisiony + newy -3) <= (-2.99f+radius_object )) && (collisionx + newx -3) >= 0.7f && (collisionx + newx -3) <= 1.9f)

      )
    {
        uy = vy;
        ux = vx ;
        // collision effect
        uy *= -1 ;
        ux /=ex;
        uy /=ey;
        // saved current co-ordinates
        collisionx +=newx;
        collisiony += newy;
        o=0; // time initialised to zero , new projectile started
    }

    // collision at y-plane
    else if( // rough-ground rectangle
        (((collisiony + newy -3) >= (-3.25f-radius_object ) && (collisiony + newy -3) <= (-2.99f+radius_object )) && (collisionx + newx -3) >= 0.7f-radius_object && (collisionx + newx -3) <= 0.7f)

      )
    {
        uy = vy;
        ux = vx ;
        // collision effect
        ux *= -1 ;
        ux /=ex;
        uy /=ey;
        // saved current co-ordinates
        collisionx +=newx;
        collisiony += newy;
        o=0; // time initialised to zero , new projectile started
    }

    int temp=0;

    //   condition of stopping the ball
      if(vx < 0.01 && vx > -0.01f){
          temp++;
      }
      if(vy < 0.01 && vy > -0.01f){
          temp++;
      }

  //   condition of initalising the shooting control
      if(temp==2 ){
          shoot=false;
      }
}

void trapping_box(){
    if(trappy>0.5 || trappy<-2.5)
        dir*=-1;
    trappy+=0.01*dir;
    // trappy*=dir;
    double object_x = -3+collisionx+newx;
    double object_y = -3 + collisiony + newy ;
    double dist =0 ;
    double trappy_x= 0.6+2;
    double trappy_y = 0.5+trappy;
    dist = calc_dist(object_x,trappy_x,object_y,trappy_y);
    if(dist< 0.5+radius_object)
        {
            cout<<"Oops , you got fired by the trappy box."<<endl;
            exit_func();
        }

}

void RenderString(float x, float y, void *font , string str, float r, float g, float b)
{
	char *c;
	glColor3f(r, g, b);
	glShadeModel(GL_SMOOTH);
	glRasterPos2f(x, y);
  	for(string::iterator it = str.begin(); it!=str.end();it++)
	{
		glutBitmapCharacter(font, *it);
	}
}

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


  // rectangle , left-up most
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3(-1.7, 2, 0));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);

  // rectangle 2 , right-up most
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle2 = glm::translate (glm::vec3(2.4, 2, 0));        // glTranslatef
  glm::mat4 rotateRectangle2 = glm::rotate((float)(rectangle2_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle2 * rotateRectangle2);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle2);

  // rectangle3 , rough-ground
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle3 = glm::translate (glm::vec3(0.7, -3.2, 0));        // glTranslatef
  glm::mat4 rotateRectangle3 = glm::rotate((float)(rectangle3_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle3 * rotateRectangle3);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle3);

  RenderString(0,0,GLUT_BITMAP_TIMES_ROMAN_10,(string)"dsf",0,0,0);

  // rectangle5 , ground
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle5 = glm::translate (glm::vec3(-1.7, -3.4, 0));        // glTranslatef
  glm::mat4 rotateRectangle5 = glm::rotate((float)(rectangle5_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle5 * rotateRectangle5);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle5);

  // coins1 , beside ground block
  // coins2 ,at last of ground
  // coins3 , up-left most
  // coins4 up-left second coin
  // coins5 up-right most coin
  // coins6 , insude water

  glm::mat4 translatecoins[num_coin+1],rotatecoins[num_coin+1],coinsTransform[num_coin+1];
  for(int r=1;r<=num_coin;r++)
  {

      Matrices.model = glm::mat4(1.0f);

       translatecoins[r] = glm::translate (glm::vec3(centerx_coin[r] , centery_coin[r], 0.0f)); // glTranslatef
       rotatecoins[r] = glm::rotate((float)(coins_rotation[r]*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0) , vec3 decides the axis about which it have to be rotated
       coinsTransform[r] = translatecoins[r] ;
      Matrices.model *= translatecoins[r] * rotatecoins[r];
      MVP = VP * Matrices.model; // MVP = p * V * M

      //  Don't change unless you are sure!!
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

      // draw3DObject draws the VAO given to it using current MVP matrix
      if(flag_coin[r]==true)
          draw3DObject(coins[r]);
  }

      // bird3
    Matrices.model = glm::mat4(1.0f);

    // o defines time
    o += 0.01;
    accelaration_func();
    move_func();
    collect_coins();
    friction_coefficient();
    collision_func();
    trapping_box();

    glm::mat4 translatebird3 = glm::translate (glm::vec3(-3.00f + collisionx+newx , -3.00f + collisiony+newy , 0.0f)); // glTranslatef
    glm::mat4 rotatebird3 = glm::rotate((float)((bird3_rotation+20)*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0) , vec3 decides the axis about which it have to be rotated
    glm::mat4 bird3Transform = translatebird3 ;
    Matrices.model *= translatebird3 * rotatebird3;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(bird3);

    // bird2
    Matrices.model = glm::mat4(1.0f);


    glm::mat4 translatebird2 = glm::translate (glm::vec3(-3.0f, -3.0f, 0.0f)); // glTranslatef
    glm::mat4 rotatebird2 = glm::rotate((float)(bird2_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0) , vec3 decides the axis about which it have to be rotated
    glm::mat4 bird2Transform = translatebird2 ;
    Matrices.model *= translatebird2 * rotatebird2;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(bird2);

  // bird1
  Matrices.model = glm::mat4(1.0f);

  moving_wheelx -= 0.017f;
  if(moving_wheelx <= -4.1f)
    moving_wheelx = 4.25f;
  glm::mat4 translatebird1 = glm::translate (glm::vec3(moving_wheelx, -3.7f, 0.0f)); // glTranslatef
  glm::mat4 rotatebird1 = glm::rotate((float)(bird1_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0) , vec3 decides the axis about which it have to be rotated
  glm::mat4 bird1Transform = translatebird1 ;
  Matrices.model *= translatebird1 * rotatebird1;
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(bird1);

  // canon
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translatecanon = glm::translate (glm::vec3(-3,-3, 0));        // glTranslatef
  glm::mat4 rotatecanon = glm::rotate((float)((canon_rotation)*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (0,0,1)
  Matrices.model *= (translatecanon * rotatecanon);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(canon);

  // rectangle6 , ground
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle6 = glm::translate (glm::vec3(-1.7+ power_meter -8 , -3.4, 0));        // glTranslatef
  // glm::mat4 rotateRectangle6 = glm::rotate((float)(rectangle6_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle6 );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle6);
  // Swap the frame buffers


  // rectangle4 , water base
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle4 = glm::translate (glm::vec3(2, trappy, 0));        // glTranslatef
  glm::mat4 rotateRectangle4 = glm::rotate((float)(rectangle4_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle4 * rotateRectangle4);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle4);

  glutSwapBuffers ();

  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  bird1_rotation = bird1_rotation + increments*bird1_rot_dir*bird1_rot_status;
  bird2_rotation = bird2_rotation + increments*bird2_rot_dir*bird2_rot_status;
  bird3_rotation = bird3_rotation + energy*bird3_rot_dir*bird3_rot_status;
  for(int r=1;r<=num_coin;r++)
  {
      coins_rotation[r] = coins_rotation[r] + (increments+5)*coins_rot_dir[r]*coins_rot_status[r];
  }
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
  rectangle2_rotation = rectangle2_rotation + increments*rectangle2_rot_dir*rectangle2_rot_status;
  rectangle3_rotation = rectangle3_rotation + increments*rectangle3_rot_dir*rectangle3_rot_status;
  rectangle4_rotation = rectangle4_rotation + increments*rectangle4_rot_dir*rectangle4_rot_status;
  rectangle5_rotation = rectangle5_rotation + increments*rectangle5_rot_dir*rectangle5_rot_status;
  canon_rotation = canon_rotation + (increments)*canon_rot_dir*canon_rot_status;

  // const char s='df';
  // string str[] = "s++" ;

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
    glutCreateWindow ("D.N.A.H.B Games");

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

    // GetCursorPos(&mousePos);
    // mousex = mousePos.x ;
    // mousey =mousePos.y ;
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
	createRectangle ();
	create_angry_bird (0,0); // Generate the VAO, VBOs, vertices data & copy into the array buffer
    createcanon (2,0); // pointed at -3   .5,-3
    ground_rectangle();
    water_rectangle();
    create_angry_coins(0,0);
    power_rectangle();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (width, height);

	// Background color of the scene
	glClearColor (0.55,0.55,0.55, 0.0f); // R, G, B, A
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
