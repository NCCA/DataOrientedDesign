#version 330 core


/// @brief the vertex passed in
in vec3 inVert;
/// @brief the normal passed in
in vec3 inDir;
uniform mat4 MVP;

void main()
{
  gl_Position = MVP*vec4(inVert,1.0);

}
