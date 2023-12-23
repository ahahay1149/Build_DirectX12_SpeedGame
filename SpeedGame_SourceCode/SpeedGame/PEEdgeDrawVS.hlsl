#include "PEEdgeDrawHeader.hlsli"

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    
    float2 texSize;
    float level;
    
    //�@���o�b�t�@����e�N�X�`���̃T�C�Y���擾
    NormalTex.GetDimensions(0, texSize.x, texSize.y, level);
    
    float2 uv = input.uv;   //����UV���W
    output.pos = input.pos;
    
    //���W�ϊ����蓮�ōs��
    if(input.pos.x < 0.0f)
        output.pos.x = -1.0f;
    else
        output.pos.x = 1.0f;
    
    if(input.pos.y < 0.0f)
        output.pos.y = -1.0f;
    else
        output.pos.y = 1.0f;
    
    output.tex0 = uv;
    
    float offset = 0.5f;    //�@���p�א�UV�l
    
    float xDef = offset / texSize.x;    //����U
    float yDef = offset / texSize.y;    //����V
    
    //======�@��
    output.tex1.xy = uv + float2(-xDef, -yDef);     //Left Top
    output.tex2.xy = uv + float2(-xDef, 0.0f);      //Left Center
    output.tex3.xy = uv + float2(-xDef, yDef);      //Left Bottom
    output.tex4.xy = uv + float2(0.0f, -yDef);      //Center Top
    output.tex5.xy = uv + float2(0.0f, yDef);       //Center Bottom
    output.tex6.xy = uv + float2(xDef, -yDef);      //Right Top
    output.tex7.xy = uv + float2(xDef, 0.0f);       //Right Center
    output.tex8.xy = uv + float2(xDef, yDef);       //Right Bottom
    //======�@�� End
    
    offset = 1.0f;  //�[�x�p�א�UV�l
    xDef = offset / texSize.x;
    yDef = offset / texSize.y;
    
    //======�[�x
    output.tex1.zw = uv + float2(-xDef, -yDef);     //Left Top
    output.tex2.zw = uv + float2(-xDef, 0.0f);      //Left Center
    output.tex3.zw = uv + float2(-xDef, yDef);      //Left Bottom
    output.tex4.zw = uv + float2(0.0f, -yDef);      //Center Top
    output.tex5.zw = uv + float2(0.0f, yDef);       //Center Bottom
    output.tex6.zw = uv + float2(xDef, -yDef);      //Right Top
    output.tex7.zw = uv + float2(xDef, 0.0f);       //Right Center
    output.tex8.zw = uv + float2(xDef, yDef);       //Right Bottom
    //======�[�x End
    
    return output;
}