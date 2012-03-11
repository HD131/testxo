float4x4 mat_mvp;  //Произведение мировой, видовой и проекционной матриц.
float4x4 mat_world; //Мировая матрица.
float4   vec_light; //Позиция источника света

// Входящие данные.
struct VS_INPUT_STRUCT
{
  float4 position: POSITION;
  float3 normal  : NORMAL;
  float3 uv      : TEXCOORD0;
};
// Исходящие данные.
struct VS_OUTPUT_STRUCT
{
  float4 position: POSITION;
  float3 uv      : TEXCOORD0;
  float3 light   : TEXCOORD1;
  float3 normal  : TEXCOORD2;
};

VS_OUTPUT_STRUCT main (VS_INPUT_STRUCT In_struct)
{
  VS_OUTPUT_STRUCT Out_struct;

    //Вычисляем позицию вершины.
  Out_struct.position = mul( In_struct.position, mat_mvp);
    //Сохраняем позицию источника для передачи во фрагментный шейдер в виде
    //3D текстурных координат.
  Out_struct.light = vec_light;

    //Рассчитываем нормаль поверхности и сохраняем для фрагментного шейдера.
  Out_struct.normal = normalize(mul( In_struct.normal, (float3x3)mat_world));

  Out_struct.uv = In_struct.uv;
  return Out_struct;
}