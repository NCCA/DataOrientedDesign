#version 330 core


/// @brief the vertex passed in
in vec3 inVert;
/// @brief the normal passed in
//in vec3 inDir;
uniform mat4 MVP;
uniform float gravity;

void main()
{

  gl_PointSize=clamp(inVert.z,0,24);

  gl_Position = MVP*vec4(inVert,1.0);

}
