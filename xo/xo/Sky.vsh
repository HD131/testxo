float4x4 mat_mvp;  //Произведение мировой, видовой и проекционной матриц.
float4x4 mat_view; //Мировая матрица.
float4   scale;
// Входящие данные.
struct VS_INPUT_STRUCT
{
  float4 position: POSITION;
};
// Исходящие данные.
struct VS_OUTPUT_STRUCT
{
  float4 position: POSITION;
  float3 uv      : TEXCOORD0;    
};

VS_OUTPUT_STRUCT main (VS_INPUT_STRUCT In_struct)
{
  VS_OUTPUT_STRUCT Out_struct;

    //Вычисляем позицию вершины.
  Out_struct.position = In_struct.position;

  Out_struct.uv = mul( (float3x3)mat_view,float3( In_struct.position.xy * scale, 1 ) );
  return Out_struct;
}