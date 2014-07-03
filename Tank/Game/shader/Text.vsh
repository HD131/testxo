float4x4 mat_world; 
int number; 

// Входящие данные.
struct VS_INPUT_STRUCT
{
  float4 position: POSITION;  
  float2 uv      : TEXCOORD0;
};
// Исходящие данные.
struct VS_OUTPUT_STRUCT
{
	float2 uv      : TEXCOORD0;  
    float4 position: POSITION;  
};

VS_OUTPUT_STRUCT main (VS_INPUT_STRUCT In_struct)
{
	VS_OUTPUT_STRUCT Out_struct;
	Out_struct.uv.x = ( In_struct.position.x + 1 ) / 2 * 0.1 + number * 0.1;
	Out_struct.uv.y = ( 1 - In_struct.position.y ) / 2;

	//Вычисляем позицию вершины.
	Out_struct.position = mul( In_struct.position, mat_world);
	
return Out_struct;
}