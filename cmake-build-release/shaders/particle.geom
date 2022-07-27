#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 1000000) out;

uniform int  nbParticles;




void main() {

    for(int i=0; i<5; i++){
        float offset= ((nbParticles)/100.0f);
        gl_Position = gl_in[0].gl_Position + vec4(offset, i, offset,1.0f);
        EmitVertex();
        gl_Position = gl_in[1].gl_Position + vec4(-offset);
        EmitVertex();
        gl_Position = gl_in[2].gl_Position+vec4(-offset);
        EmitVertex();
        EndPrimitive();
    }
    for(int i=0; i<5; i++){
        float offset= ((nbParticles)/100.0f);
        gl_Position = gl_in[0].gl_Position + vec4(offset, i, -offset,1.0f);
        EmitVertex();
        gl_Position = gl_in[1].gl_Position + vec4(i, offset, -offset,1.0f);
        EmitVertex();
        gl_Position = gl_in[2].gl_Position+vec4(offset, i, -offset,1.0f);
        EmitVertex();
        EndPrimitive();
    }
    for(int i=0; i<5; i++){
        float offset= ((nbParticles)/100.0f);
        gl_Position = gl_in[0].gl_Position + vec4(offset, i, offset,1.0f);
        EmitVertex();
        gl_Position = gl_in[1].gl_Position +vec4(offset, i, offset,1.0f);
        EmitVertex();
        gl_Position = gl_in[2].gl_Position+vec4(i, offset, offset,1.0f);
        EmitVertex();
        EndPrimitive();
    }
    for(int i=0; i<5; i++){
        float offset= ((nbParticles)/100.0f);
        gl_Position = gl_in[0].gl_Position + vec4(offset, i, -offset,1.0f);
        EmitVertex();
        gl_Position = gl_in[1].gl_Position + vec4(-offset);
        EmitVertex();
        gl_Position = gl_in[2].gl_Position+vec4(offset);
        EmitVertex();
        EndPrimitive();
    }
    for(int i=0; i<10; i++){
        float offset= ((nbParticles)/100.0f);
        gl_Position = gl_in[0].gl_Position + vec4(offset, i, -offset,1.0f);
        EmitVertex();
        gl_Position = gl_in[1].gl_Position + vec4(i, -offset, offset,1.0f);
        EmitVertex();
        gl_Position = gl_in[2].gl_Position+vec4(offset, -i, -offset,1.0f);
        EmitVertex();
        EndPrimitive();
    }
    for(int i=0; i<10; i++){
        float offset= ((nbParticles)/100.0f);
        gl_Position = gl_in[0].gl_Position + vec4(-offset, -i, -offset,1.0f);
        EmitVertex();
        gl_Position = gl_in[1].gl_Position +vec4(offset, -i, -offset,1.0f);
        EmitVertex();
        gl_Position = gl_in[2].gl_Position+vec4(i, -offset, -offset,1.0f);
        EmitVertex();
        EndPrimitive();
    }

    for(int i=0; i<5; i++){
        float offset= ((nbParticles)/100.0f);
        gl_Position = gl_in[0].gl_Position + vec4(offset, i, offset,1.0f);
        EmitVertex();
        gl_Position = gl_in[1].gl_Position +vec4(offset, offset, offset,1.0f);
        EmitVertex();
        gl_Position = gl_in[2].gl_Position+vec4(offset, i,offset,1.0f);
        EmitVertex();
        EndPrimitive();
    }





}

