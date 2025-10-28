#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool isGround;
uniform bool isCoin;
uniform vec3 modelColor;

void main()
{
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;

    vec3 ambient = 0.6 * color;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = abs(dot(norm, lightDir));
    vec3 diffuse = 0.8 * diff * color;

   
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = vec3(0.15) * spec;

    vec3 result = ambient + diffuse + specular;

    if (isGround || isCoin)
        result *= modelColor;

    FragColor = vec4(result, 1.0);
}
