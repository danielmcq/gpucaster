// 64 rays for GetSurfaceNormalMQ().
static const float4 g_poisson_64[64] = { 
    float4(    -0.273561f, -0.772808f, -0.572654f , 0),    float4(    -0.662813f, -0.563365f, -0.493254f , 0),    float4(    -0.481786f, -0.866285f, -0.132034f , 0),    float4(    -0.871765f,  0.388017f, -0.299112f , 0),  
    float4(    -0.311603f,  0.094995f, -0.945452f , 0),    float4(     0.907133f, -0.419975f, -0.027050f , 0),    float4(    -0.247884f, -0.433742f, -0.866269f , 0),    float4(    -0.856946f, -0.048178f, -0.513150f , 0),
    float4(    -0.070369f, -0.971783f, -0.225135f , 0),    float4(    -0.985704f, -0.095313f, -0.138935f , 0),    float4(     0.974397f,  0.023183f,  0.223637f , 0),    float4(     0.483691f,  0.072047f, -0.872269f , 0),
    float4(     0.112239f,  0.325632f, -0.938811f , 0),    float4(     0.640779f, -0.204800f,  0.739905f , 0),    float4(     0.755598f,  0.488083f, -0.436859f , 0),    float4(    0.093706f, -0.150605f, -0.984143f  , 0),
    float4(   -0.388171f, -0.717095f,  0.578876f  , 0),    float4(    0.424155f, -0.735267f,  0.528654f  , 0),    float4(   -0.145809f, -0.954081f,  0.261668f  , 0),    float4(   -0.795397f, -0.009678f,  0.606011f  , 0),
    float4(    0.406075f,  0.547495f, -0.731677f  , 0),    float4(    0.935125f,  0.345985f, -0.076394f  , 0),    float4(  -0.235995f,  0.954026f, -0.184771f   , 0),    float4(   0.184488f,  0.477052f,  0.859294f   , 0),
    float4(   0.958729f, -0.077807f, -0.273467f   , 0),    float4(   0.789553f,  0.079553f, -0.608504f   , 0),    float4(  -0.143271f, -0.050693f,  0.988384f   , 0),    float4(  -0.500941f,  0.712374f, -0.491508f   , 0),
    float4(  -0.168276f, -0.422310f,  0.890695f   , 0),    float4(    0.003948f,  0.822251f, -0.569111f  , 0),    float4(   -0.864418f, -0.384420f,  0.324042f  , 0),    float4(   -0.563149f,  0.244265f,  0.789429f  , 0),
    float4(    0.159904f, -0.589848f, -0.791524f  , 0),    float4(    0.369539f,  0.848316f,  0.379210f  , 0),    float4(   -0.025722f,  0.950788f,  0.308772f  , 0),    float4(   -0.152069f,  0.729672f,  0.666674f  , 0),
    float4(  0.058188f, -0.752741f,  0.655740f    , 0),    float4(  0.263671f, -0.833565f, -0.485435f    , 0),    float4(  0.659577f, -0.721236f,  0.211602f    , 0),    float4( -0.231048f,  0.544712f, -0.806167f    , 0),
    float4( -0.872600f,  0.465859f,  0.146779f    , 0),    float4(  0.796086f,  0.515036f,  0.317781f    , 0),    float4( -0.436394f,  0.848105f,  0.300462f    , 0),    float4(-0.592389f, -0.212111f, -0.777229f     , 0),
    float4( 0.245208f, -0.964587f,  0.097181f     , 0),    float4(-0.969221f,  0.080171f,  0.232773f     , 0),    float4(-0.207633f,  0.350494f,  0.913259f     , 0),    float4(-0.651118f,  0.554240f,  0.518520f     , 0),
    float4(-0.641318f,  0.763081f, -0.080116f     , 0),    float4( 0.186895f,  0.979163f, -0.079437f     , 0),    float4( -0.627617f, -0.738731f,  0.245711f    , 0),    float4(  0.499249f,  0.562093f,  0.659395f    , 0),
    float4( -0.578916f, -0.335735f,  0.743060f    , 0),    float4(  0.528080f, -0.365175f, -0.766667f    , 0),    float4( -0.643685f,  0.323735f, -0.693445f    , 0),    float4(  0.562780f, -0.799455f, -0.210122f    , 0),
    float4(  0.427496f,  0.816229f, -0.388608f    , 0),    float4( -0.847434f, -0.515899f, -0.125314f    , 0),    float4(  0.830392f, -0.357533f,  0.427341f    , 0),    float4(  0.297373f, -0.386429f,  0.873065f    , 0),
    float4(  0.644596f,  0.764522f, -0.001120f    , 0),    float4(  0.745686f, -0.482984f, -0.458997f    , 0),    float4(  0.337361f,  0.075642f,  0.938331f    , 0),    float4(  0.762799f,  0.187441f,  0.618873f    , 0),
};