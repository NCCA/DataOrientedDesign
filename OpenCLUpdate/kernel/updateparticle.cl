typedef struct Particle
{

	/// @brief the curent particle position
	//ngl::Vec3 m_pos;
	float m_px;
	float m_py;
	float m_pz;

	/// @brief the direction vector of the particle
	float m_dx;
	float m_dy;
	float m_dz;
	/// @brief the current life value of the particle
	float m_currentLife;
}Particle;

typedef struct GLParticle
{
	float px;
	float py;
	float pz;
}GLParticle;

typedef struct Vec3
{
  float m_x;
  float m_y;
  float m_z;
}Vec3;

__kernel void updateparticle( __global Particle* input,   __global GLParticle* output, Vec3 wind, Vec3 pos, float gravity)
{
   unsigned int i = get_global_id(0);
   output[i].px=pos.m_x+(wind.m_x*input[i].m_dx*input[i].m_currentLife);
   output[i].py= pos.m_y+(wind.m_y*input[i].m_dy*input[i].m_currentLife)+gravity*(input[i].m_currentLife*input[i].m_currentLife);
   output[i].pz=pos.m_z+(wind.m_z*input[i].m_dz*input[i].m_currentLife);

}
