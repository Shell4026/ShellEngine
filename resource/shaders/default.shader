#version 430 core

Shader "Default Shader"
{
	Pass
	{
		Stage Vertex
		{
			layout(location = 0) in vec3 verts;
			layout(location = 1) in vec2 uvs;
			layout(location = 2) in vec3 normals;

			layout(location = 0) out vec2 fragUvs;
			layout(location = 1) out vec3 fragPos;
			layout(location = 2) out vec3 fragNormals;

			layout(set = 0, binding = 0) uniform MVP
			{
				mat4 model;
				mat4 view;
				mat4 proj;
			} mvp;

			void main()
			{
				gl_Position = mvp.proj * mvp.view * mvp.model * vec4(verts, 1.0f);
				fragUvs = uvs;
				fragPos = (mvp.model * vec4(verts, 1.0f)).xyz;
				fragNormals = normalize((mvp.model * vec4(normals, 0.0f)).xyz);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			layout(location = 0) in vec2 uvs;
			layout(location = 1) in vec3 fragPos;
			layout(location = 2) in vec3 fragNormals;

			layout(set = 0, binding = 1) uniform Lights
			{
				vec4 lightPosRange[10];
				int lightCount;
			} lights;
			layout(set = 1, binding = 0) uniform Material
			{
				float ambient;
			} material;
			layout(set = 1, binding = 1) uniform sampler2D tex;

			void main() 
			{
				float diffuse = 0.f;
				for (int i = 0; i < lights.lightCount; ++i)
				{
					vec3 toLightVec = lights.lightPosRange[i].xyz - fragPos;
					vec3 toLightDir = normalize(toLightVec);
					float lightDis = length(toLightVec);
					float attenuation = clamp(1.0 - (lightDis / lights.lightPosRange[i].w), 0.0, 1.0);
					
					diffuse += max(dot(fragNormals, toLightDir), 0.0) * attenuation;
				}
				outColor = texture(tex, uvs);
				outColor.xyz *= diffuse + material.ambient;
			}
		}
	}
}