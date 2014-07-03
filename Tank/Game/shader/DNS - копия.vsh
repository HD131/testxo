float4x4 mat_mvp;   //Произведение мировой, видовой и проекционной матриц.
float4x4 mat_world; //Мировая матрица.
float4   vLightPos; //Позиция источника света
float4   vViewPos;   // View position

struct VS_INPUT_STRUCT
{
    float4 position: POSITION;  // Vertex position
	float3 tangent:  TANGENT;   // Tangent ( u )
	float3 binormal: BINORMAL;  // Binormal( v )
    float3 normal:   NORMAL0;   // Vertex normal
    float2 texcoord: TEXCOORD0; // Vertex texcoord     
};

struct VS_OUTPUT_STRUCT
{
    float4 position: POSITION;  // Vertex position
    float2 texcoord: TEXCOORD0; // Texture coordinates
    float3 light:    TEXCOORD1; // Light vector
    float3 view:     TEXCOORD2; // View vector
};

VS_OUTPUT_STRUCT main(VS_INPUT_STRUCT In_struct)
{
    VS_OUTPUT_STRUCT Out_struct;
  
    Out_struct.position = mul( In_struct.position, mat_mvp );
    Out_struct.texcoord = In_struct.texcoord;
    
    float3x3 matTangentSpace;    

    matTangentSpace[0] = mul( In_struct.tangent,  mat_world );  // x
    matTangentSpace[1] = mul( In_struct.binormal, mat_world );  // y
    matTangentSpace[2] = mul( In_struct.normal,   mat_world );  // z
 
    // Calculate light vector and move into tangent space
    Out_struct.light.xyz = mul( matTangentSpace, normalize( vLightPos ) );
	
    // Calculate view vector and move into tangent space
    Out_struct.view = mul( vViewPos, matTangentSpace );
    
    return Out_struct;
}