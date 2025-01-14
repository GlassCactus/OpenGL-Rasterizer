#shader vertex
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;

out vec3 FragPos;
out vec3 Normal;
out vec2 Tex;

uniform mat4 normalMat;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0f));
	Tex = aTex;
	gl_Position = projection * view * vec4(FragPos, 1.0f);
	Normal = normalize(mat3(normalMat) * aNormal);
}

//==============================================================================================================//
//==============================================================================================================//

#shader fragment
#version 330 core

out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 Tex;

const float attConst = 1.0f;
const float attLinear = 0.0035f;
const float attQuad = 0.0005f;
const float PI = 3.141596;

const int NR_POINT_LIGHTS = 3;


struct TexMaterial
{
	sampler2D diffuse;
	sampler2D specular;
	float alpha;
};

struct Materials
{
	vec3 ambientCol;
	vec3 diffCol;
	vec3 specCol;
	float alpha;
};

struct DirLight
{
	vec3 lightDir;

	vec3 ambient;
	vec3 diff;
	vec3 spec;
};

struct PointLight
{
	vec3 position;

	vec3 ambient;
	vec3 diff;
	vec3 spec;
};

uniform TexMaterial material;
uniform Materials box;
uniform sampler2D normalMap;
uniform float GAMMA;

uniform DirLight dirlight;
uniform PointLight pointlight[NR_POINT_LIGHTS];
uniform vec3 viewPos;

uniform bool Blinn;
uniform bool ModifiedSpecNorm;
uniform bool SpecNorm;


vec3 PointLights(PointLight light, vec3 FragPos)
{
	float distance = length(light.position - FragPos);
	float attenuation = 1.0f / (attConst + (attLinear * distance) + (attQuad * (distance * distance)));

	//ambient
	vec3 ambience = light.ambient * texture(material.diffuse, Tex).rgb;

	//diffuse
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(Normal, lightDir), 0.0f);
	vec3 diffuse = light.diff * diff * texture(material.diffuse, Tex).rgb;


	//specular with normalization constant
	vec3 viewDir = normalize(viewPos - FragPos);
	float spec;

	if (Blinn)
	{
		vec3 halfwayDir = normalize(viewDir + lightDir); //Blinn-Phong's halfwayDir vector
		spec = max(dot(Normal, halfwayDir), 0.0f);
	}

	else
	{
		vec3 reflectDir = reflect(-lightDir, Normal);
		spec = max(dot(viewDir, reflectDir), 0.0f);
	}

	float specNormalization = 1.0f;

	if (ModifiedSpecNorm)
		specNormalization = (box.alpha + 2.0) / (4.0 * PI * (2.0 - exp(-box.alpha / 2.0)));

	if (SpecNorm)
		specNormalization = (box.alpha + 1.0) / (2.0 * PI);


	spec = pow(spec, box.alpha) * specNormalization;
	vec3 specular = spec * (light.spec) * texture(material.specular, Tex).rgb;

	//Blinn-Phong!!!
	return (ambience + diffuse + specular) * attenuation;
}


void main()
{
	box;
	material;
	vec3 phong = vec3(0.0, 0.0, 0.0);

	for (int i = 0; i < NR_POINT_LIGHTS; i++)
	{
		phong += (PointLights(pointlight[i], FragPos));
	}
	//phong *= texture(material.diffuse, Tex);

	FragColor.rgb = pow(phong.rgb, vec3(1.0 / GAMMA));
}