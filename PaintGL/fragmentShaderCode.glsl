#version 430

out vec4 daColor;
in vec3 theColor;

void main()
{
	//daColor = vec4(0.5,1.0,0.9,0.0);
	daColor = vec4(theColor,1.0);
}