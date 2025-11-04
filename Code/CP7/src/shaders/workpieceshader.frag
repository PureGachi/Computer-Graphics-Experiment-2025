#version 330 core

uniform vec3 Colors;
layout(location = 0) out vec4 FragColor;


void main(){
    FragColor = vec4(Colors,1.0);
}