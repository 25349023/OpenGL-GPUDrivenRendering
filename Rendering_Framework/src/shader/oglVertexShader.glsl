#version 430 core

layout(location=0) in vec3 v_vertex;
layout(location=1) in vec3 v_normal ;
layout(location=2) in vec3 v_uv ;

out vec3 f_viewVertex ;
out vec3 f_viewNormal ;
out vec3 f_worldVertex ;
out vec3 f_worldNormal ;
out vec3 f_uv ;

uniform mat4 projMat ;
uniform mat4 viewMat ;
uniform mat4 modelMat ;

void commonProcess(){
	mat4 modelViewMat = viewMat * modelMat ;
	vec4 viewVertex = modelViewMat * vec4(v_vertex, 1.0) ;
	vec4 viewNormal = modelViewMat * vec4(v_normal, 0.0) ;
	
	f_viewVertex = viewVertex.xyz ;
	f_viewNormal = viewNormal.xyz ;
	f_worldVertex = (modelMat * vec4(v_vertex, 1.0)).xyz ;
	f_worldNormal = (modelMat * vec4(v_normal, 0.0)).xyz ;
	f_uv = v_uv ;

	gl_Position = projMat * viewVertex ;
}

void main(){	
	commonProcess() ;	
}