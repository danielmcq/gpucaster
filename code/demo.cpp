
#include <stdio.h>
#include <assert.h>

#include "demo.h"

double NGetTime();   //TODO: move to proper header file

namespace gpucaster {

MTRand g_rand;  // Mersenne Twister random # generator.

// 512 evenly-distributed unit-length rays for GetSurfaceNormalHQ().
static const float g_poisson_512[512*3] = { 
     0.799579f, -0.298273f, -0.521255f,   -0.379600f, -0.569416f, -0.729157f,   -0.849737f, -0.413929f, -0.326511f,   -0.193230f, -0.838603f, -0.509321f,    0.179479f, -0.713749f,  0.677015f,    0.160994f, -0.107949f, -0.981034f,   -0.537529f, -0.786490f,  0.304132f,   -0.790809f, -0.334660f, -0.512468f,     // 0-7
     0.037251f, -0.922446f, -0.384325f,    0.436488f,  0.157932f,  0.885740f,   -0.799774f, -0.185605f, -0.570887f,   -0.761452f, -0.495162f, -0.418336f,   -0.742164f, -0.431268f,  0.513031f,   -0.462097f, -0.726455f, -0.508654f,   -0.221619f, -0.904049f, -0.365486f,   -0.633495f, -0.403931f, -0.659942f,     // 8-15
    -0.398356f,  0.242340f,  0.884638f,    0.173582f, -0.332326f, -0.927054f,   -0.596691f, -0.598806f, -0.534220f,   -0.872156f, -0.286226f, -0.396760f,    0.905889f, -0.416755f,  0.075366f,   -0.971689f, -0.154246f, -0.178966f,   -0.993460f, -0.056897f,  0.098991f,   -0.435320f, -0.898879f,  0.050124f,     // 16-23
    -0.208267f, -0.368599f, -0.905958f,   -0.532714f, -0.528225f, -0.661206f,   -0.073688f,  0.928405f,  0.364190f,   -0.546425f, -0.738061f, -0.395835f,   -0.403685f, -0.847966f, -0.343501f,   -0.390401f, -0.442131f, -0.807532f,    0.159146f, -0.956300f, -0.245283f,    0.578662f, -0.156891f,  0.800335f,     // 24-31
    -0.628447f, -0.194726f, -0.753084f,   -0.171322f, -0.974458f,  0.145190f,   -0.652104f, -0.710910f, -0.263378f,    0.282119f, -0.225162f, -0.932583f,   -0.697843f, -0.464973f, -0.544807f,   -0.415849f, -0.171756f, -0.893067f,   -0.354974f, -0.930843f, -0.086748f,    0.276558f, -0.958970f, -0.062379f,     // 32-39
    -0.020635f,  0.585024f, -0.810753f,   -0.708064f, -0.271919f, -0.651694f,   -0.674293f, -0.685155f,  0.275485f,    0.013756f, -0.372580f, -0.927898f,   -0.432883f, -0.652123f, -0.622373f,   -0.768335f, -0.575948f, -0.279185f,   -0.519129f,  0.319379f, -0.792781f,    0.043609f, -0.969281f,  0.242060f,     // 40-47
    -0.723925f,  0.477755f,  0.497677f,   -0.526331f, -0.383426f, -0.758920f,    0.214672f,  0.944338f, -0.249281f,    0.497703f, -0.848855f,  0.178147f,   -0.978869f,  0.177723f,  0.101141f,   -0.613619f, -0.780394f, -0.120241f,    0.260964f, -0.693932f, -0.671085f,    0.445829f, -0.895105f, -0.004971f,     // 48-55
    -0.931586f, -0.338699f,  0.132025f,   -0.882487f, -0.434425f, -0.180256f,   -0.836744f, -0.039722f, -0.546151f,   -0.506652f, -0.014451f, -0.862029f,    0.645645f, -0.760154f, -0.072860f,   -0.875295f,  0.478918f, -0.067050f,   -0.928282f, -0.370082f, -0.036489f,   -0.056503f, -0.872108f, -0.486040f,     // 56-63
    -0.317647f, -0.916778f, -0.242111f,   -0.936496f,  0.217754f, -0.274879f,   -0.933092f, -0.164964f, -0.319573f,   -0.337477f, -0.812051f, -0.476112f,   -0.933715f, -0.298883f, -0.197092f,   -0.243823f, -0.510532f, -0.824565f,   -0.073735f, -0.991222f, -0.109734f,   -0.208737f, -0.967782f, -0.140807f,     // 64-71
    -0.169533f,  0.603415f, -0.779197f,   -0.664759f, -0.629044f, -0.402987f,   -0.664173f,  0.305883f, -0.682136f,   -0.570463f,  0.428066f, -0.700950f,   -0.369497f, -0.317230f, -0.873405f,   -0.076179f, -0.260858f, -0.962367f,   -0.833260f, -0.552424f,  0.022468f,   -0.039993f,  0.293781f, -0.955036f,     // 72-79
    -0.806754f,  0.314302f, -0.500362f,   -0.523263f, -0.235033f, -0.819119f,   -0.976036f, -0.216698f, -0.019868f,   -0.514108f, -0.819294f, -0.253871f,   -0.118358f, -0.124954f, -0.985078f,   -0.482918f, -0.869053f, -0.107411f,   -0.221070f, -0.637121f, -0.738380f,   -0.892227f, -0.109341f, -0.438151f,     // 80-87
    -0.374973f, -0.039203f, -0.926207f,   -0.106710f, -0.947924f, -0.300089f,   -0.099717f, -0.471052f, -0.876451f,    0.192880f, -0.898737f, -0.393789f,   -0.250671f, -0.222212f, -0.942224f,   -0.035305f, -0.015115f, -0.999262f,   -0.583511f,  0.155594f, -0.797061f,    0.804607f,  0.226915f, -0.548741f,     // 88-95
    -0.210088f, -0.855051f,  0.474080f,   -0.266400f,  0.218082f, -0.938867f,   -0.829214f, -0.545634f, -0.121194f,   -0.080028f, -0.631707f, -0.771065f,   -0.728941f, -0.684128f,  0.024756f,   -0.237862f, -0.037959f, -0.970557f,    0.168471f,  0.195993f, -0.966025f,   -0.970028f, -0.199643f,  0.138520f,     // 96-103
     0.204742f, -0.949984f,  0.235820f,    0.150113f, -0.470621f, -0.869472f,   -0.069372f, -0.997066f,  0.032362f,   -0.763157f,  0.049222f, -0.644336f,    0.357515f, -0.852028f,  0.382403f,    0.336268f,  0.789089f, -0.514064f,    0.486016f, -0.198855f, -0.851026f,   -0.289034f, -0.726946f, -0.622904f,     // 104-111
    -0.470214f, -0.863479f,  0.182492f,   -0.259475f, -0.965263f,  0.030679f,   -0.752368f, -0.638228f,  0.163117f,    0.408561f, -0.896233f, -0.172752f,   -0.352810f,  0.109257f, -0.929294f,   -0.866344f,  0.096743f, -0.489989f,    0.663427f, -0.701322f, -0.260790f,   -0.879564f, -0.461783f,  0.114558f,     // 112-119
    -0.994934f, -0.059019f, -0.081382f,    0.023597f, -0.976688f, -0.213365f,   -0.816133f, -0.444854f,  0.368824f,   -0.777116f,  0.455787f, -0.433992f,   -0.382481f,  0.335985f, -0.860710f,    0.339468f,  0.916901f, -0.209891f,   -0.136880f, -0.756899f, -0.639036f,   -0.734237f, -0.665898f, -0.132197f,     // 120-127
     0.283531f,  0.899115f,  0.333472f,   -0.955424f, -0.005521f, -0.295185f,   -0.614341f, -0.010359f, -0.788973f,   -0.353932f, -0.677897f,  0.644351f,    0.008645f, -0.762725f, -0.646666f,   -0.642321f, -0.561018f,  0.522191f,    0.774153f,  0.413675f, -0.479124f,   -0.869996f,  0.322636f, -0.372844f,     // 128-135
    -0.608997f, -0.777997f,  0.154411f,   -0.241085f, -0.913047f,  0.328972f,   -0.159019f,  0.106553f, -0.981509f,   -0.426455f, -0.547502f,  0.719985f,   -0.006312f,  0.151396f, -0.988453f,    0.371354f, -0.895094f,  0.246787f,   -0.813505f,  0.184041f,  0.551669f,    0.507865f, -0.718449f, -0.475294f,     // 136-143
    -0.938028f,  0.000444f,  0.346558f,   -0.978662f,  0.089901f, -0.184767f,    0.514886f,  0.856575f,  0.034220f,   -0.727861f,  0.685439f,  0.019800f,   -0.975208f,  0.209062f, -0.072548f,   -0.941253f,  0.336899f,  0.023264f,   -0.915831f,  0.128240f, -0.380536f,    0.925442f, -0.176287f,  0.335380f,     // 144-151
     0.114723f, -0.990954f, -0.069628f,    0.078524f, -0.681700f, -0.727406f,   -0.602536f, -0.797822f,  0.020770f,    0.214892f, -0.897616f,  0.384847f,    0.406053f, -0.587429f, -0.700034f,   -0.710238f, -0.080106f, -0.699389f,    0.069310f, -0.203510f, -0.976616f,   -0.879838f,  0.087690f,  0.467114f,     // 152-159
     0.066688f, -0.842987f,  0.533784f,    0.081447f, -0.832634f, -0.547802f,   -0.136086f,  0.472708f, -0.870648f,    0.058037f, -0.993947f,  0.093283f,    0.337865f, -0.759378f, -0.556051f,   -0.805345f,  0.584465f,  0.099095f,    0.635880f,  0.038396f, -0.770832f,    0.710437f,  0.294585f, -0.639139f,     // 160-167
    -0.952282f, -0.133618f,  0.274416f,   -0.997808f,  0.065491f, -0.009449f,    0.285787f, -0.497037f, -0.819317f,   -0.932719f,  0.330131f, -0.145084f,    0.947894f, -0.266843f, -0.174047f,    0.326829f, -0.858299f, -0.395608f,   -0.891206f, -0.172313f,  0.419594f,   -0.460730f,  0.182380f, -0.868600f,     // 168-175
    -0.704704f,  0.582247f, -0.405438f,   -0.415118f,  0.623360f, -0.662646f,   -0.190796f,  0.575252f,  0.795414f,   -0.069659f,  0.527093f,  0.846948f,    0.470196f,  0.633007f,  0.614994f,   -0.788967f, -0.255988f,  0.558570f,    0.517919f,  0.449387f, -0.727881f,   -0.977600f,  0.047783f,  0.204974f,     // 176-183
    -0.113437f,  0.993278f,  0.023045f,    0.577815f, -0.770598f,  0.268902f,    0.106327f,  0.034553f, -0.993731f,   -0.606210f, -0.673155f,  0.423524f,   -0.731373f, -0.571198f,  0.372595f,   -0.172033f,  0.318210f, -0.932281f,   -0.878201f,  0.411022f, -0.244591f,    0.209471f, -0.811630f, -0.545324f,     // 184-191
     0.028596f, -0.547009f, -0.836638f,   -0.788835f,  0.210402f, -0.577469f,   -0.042726f,  0.975125f, -0.217500f,    0.396978f, -0.110484f, -0.911154f,   -0.848889f, -0.313534f,  0.425539f,   -0.825523f, -0.511636f,  0.238202f,   -0.789010f, -0.105424f,  0.605268f,    0.509777f,  0.324432f,  0.796788f,     // 192-199
    -0.863680f, -0.046440f,  0.501897f,   -0.926727f,  0.332379f,  0.175220f,   -0.816655f,  0.342052f,  0.464839f,    0.490288f,  0.018058f, -0.871373f,    0.290205f, -0.030974f, -0.956463f,   -0.324555f, -0.925930f,  0.193177f,    0.344668f, -0.343961f, -0.873438f,    0.292235f, -0.929620f, -0.224513f,     // 200-207
     0.985241f, -0.125932f, -0.115938f,    0.108086f,  0.322348f, -0.940430f,    0.021714f,  0.456001f, -0.889714f,   -0.811941f,  0.522826f, -0.259624f,    0.998915f, -0.045790f,  0.008486f,   -0.795786f,  0.598305f, -0.093578f,   -0.879641f,  0.464918f,  0.100413f,    0.205216f, -0.810332f,  0.548861f,     // 208-215
     0.819626f,  0.079608f,  0.567341f,   -0.418403f,  0.479537f, -0.771352f,   -0.908342f, -0.308976f,  0.281868f,   -0.692086f,  0.160613f, -0.703719f,   -0.690717f, -0.230159f,  0.685520f,    0.280296f,  0.109895f, -0.953602f,   -0.183442f,  0.838712f,  0.512748f,    0.808181f,  0.501253f,  0.309173f,     // 216-223
    -0.630548f,  0.100175f,  0.769658f,   -0.518789f,  0.848267f,  0.106308f,    0.074763f,  0.675431f, -0.733623f,    0.302875f, -0.543838f,  0.782628f,    0.669289f, -0.222246f,  0.708984f,   -0.276034f,  0.431408f, -0.858890f,   -0.804503f,  0.478972f,  0.351228f,   -0.388097f, -0.855519f,  0.342736f,     // 224-231
     0.110984f,  0.961857f,  0.250029f,    0.193284f,  0.664230f,  0.722108f,   -0.339429f, -0.410623f,  0.846272f,    0.483797f,  0.255945f, -0.836919f,    0.540785f,  0.020380f,  0.840914f,    0.427606f, -0.470621f, -0.771796f,    0.274049f,  0.548444f,  0.790004f,   -0.726894f,  0.171479f,  0.664996f,     // 232-239
     0.770586f, -0.567801f, -0.289480f,   -0.157774f, -0.683391f,  0.712800f,   -0.457927f,  0.784081f, -0.418951f,    0.193016f,  0.422499f, -0.885573f,    0.196651f, -0.349369f,  0.916117f,    0.198766f, -0.613770f, -0.764054f,   -0.079854f, -0.809089f,  0.582236f,    0.691741f, -0.148153f, -0.706785f,     // 240-247
    -0.099544f, -0.958011f,  0.268897f,    0.207604f, -0.974977f,  0.079500f,    0.447331f, -0.808009f, -0.383427f,    0.911310f, -0.404429f, -0.077138f,   -0.946573f,  0.183209f,  0.265393f,    0.603372f, -0.606157f, -0.518185f,    0.916451f,  0.231431f,  0.326432f,    0.585575f, -0.807574f,  0.070190f,     // 248-255
     0.349465f,  0.646405f,  0.678258f,    0.941906f,  0.308343f,  0.133182f,   -0.610449f,  0.792023f,  0.007171f,    0.353433f, -0.930077f,  0.100204f,   -0.350226f,  0.567608f,  0.745093f,   -0.748098f,  0.026731f,  0.663049f,    0.463207f,  0.868065f, -0.178613f,    0.863829f, -0.169428f, -0.474440f,     // 256-263
    -0.348933f, -0.794726f,  0.496645f,   -0.305917f,  0.589338f, -0.747727f,   -0.513419f,  0.852817f, -0.095412f,    0.562508f, -0.464688f, -0.683849f,    0.542393f, -0.831775f, -0.118158f,   -0.076222f, -0.902412f,  0.424079f,   -0.729368f,  0.635786f, -0.252584f,    0.979097f, -0.123964f,  0.161254f,     // 264-271
    -0.892209f,  0.222393f,  0.393070f,    0.518490f, -0.813167f, -0.264439f,    0.177221f, -0.485126f,  0.856297f,   -0.111193f,  0.965008f,  0.237478f,   -0.071241f,  0.928362f, -0.364785f,   -0.674619f, -0.389278f,  0.627178f,    0.297754f,  0.491950f, -0.818124f,   -0.241929f,  0.915711f, -0.320850f,     // 272-279
    -0.490571f, -0.651942f,  0.578196f,   -0.628606f,  0.569596f, -0.529542f,   -0.728287f,  0.339333f,  0.595358f,    0.683487f,  0.442016f, -0.580920f,   -0.192571f,  0.715744f, -0.671288f,    0.454241f, -0.656940f, -0.601744f,    0.789070f,  0.606145f,  0.099784f,   -0.058317f,  0.742352f, -0.667468f,     // 280-287
     0.590872f, -0.095509f, -0.801092f,    0.628169f,  0.747427f, -0.216232f,    0.308105f,  0.283938f, -0.907993f,   -0.880704f,  0.366759f,  0.299748f,   -0.678111f,  0.595328f,  0.430988f,    0.070803f, -0.916756f,  0.393123f,    0.418466f,  0.137716f, -0.897731f,   -0.559363f, -0.520225f,  0.645352f,     // 288-295
    -0.540666f,  0.553626f, -0.633386f,    0.026458f, -0.742902f,  0.668877f,   -0.365863f, -0.121170f,  0.922747f,    0.752652f, -0.265421f,  0.602550f,    0.492717f, -0.340609f, -0.800759f,    0.840365f, -0.541813f,  0.015008f,    0.792584f,  0.579546f, -0.189573f,    0.195773f,  0.445399f,  0.873666f,     // 296-303
    -0.202582f, -0.468958f,  0.859673f,   -0.680629f,  0.667612f,  0.301726f,   -0.015732f, -0.634672f,  0.772622f,    0.386465f,  0.917772f,  0.091317f,    0.920129f, -0.044867f, -0.389037f,    0.405993f,  0.381221f, -0.830566f,   -0.090180f, -0.056739f,  0.994308f,    0.065778f,  0.893130f, -0.444963f,     // 304-311
     0.065408f,  0.625642f,  0.777364f,    0.951410f,  0.088423f, -0.294957f,    0.474359f,  0.855101f,  0.209250f,   -0.479004f, -0.749575f,  0.456829f,   -0.348356f,  0.920118f, -0.178971f,    0.743509f, -0.652525f, -0.146304f,   -0.558895f,  0.793505f,  0.240802f,    0.855259f,  0.015641f, -0.517964f,     // 312-319
     0.641036f,  0.061020f,  0.765081f,   -0.045485f,  0.868082f,  0.494332f,   -0.230229f, -0.758608f,  0.609516f,    0.493824f,  0.728674f,  0.474523f,    0.919326f, -0.300736f,  0.253766f,    0.758588f,  0.519962f, -0.392664f,    0.352754f,  0.934588f, -0.045929f,    0.477354f,  0.632081f, -0.610415f,     // 320-327
     0.070352f,  0.762971f,  0.642593f,    0.407104f,  0.491491f,  0.769872f,    0.882169f,  0.139156f, -0.449904f,    0.441367f, -0.696955f,  0.565198f,    0.286877f, -0.004104f,  0.957959f,   -0.221451f, -0.324850f,  0.919474f,    0.626395f, -0.680146f, -0.380829f,   -0.396805f,  0.917489f, -0.027551f,     // 328-335
    -0.309844f,  0.837287f, -0.450496f,   -0.793374f,  0.558823f,  0.241402f,    0.969604f, -0.057578f, -0.237809f,   -0.672530f,  0.728436f, -0.130710f,    0.710499f,  0.698213f, -0.087685f,    0.743221f, -0.443037f, -0.501339f,    0.611685f, -0.489702f,  0.621316f,    0.213296f,  0.888589f, -0.406096f,     // 336-343
    -0.698867f,  0.420375f, -0.578680f,   -0.285191f,  0.866008f,  0.410726f,    0.874421f,  0.476395f,  0.091847f,    0.990537f,  0.040935f,  0.131002f,    0.717639f,  0.668365f,  0.195660f,    0.171145f,  0.291224f,  0.941221f,   -0.409266f,  0.861934f, -0.299284f,    0.856196f, -0.456363f,  0.242202f,     // 344-351
     0.365365f,  0.851967f, -0.375047f,   -0.164423f,  0.864835f, -0.474368f,   -0.564347f,  0.689619f, -0.453803f,    0.595921f,  0.175768f, -0.783571f,   -0.680076f,  0.714897f,  0.162538f,    0.723054f,  0.265633f,  0.637678f,   -0.669258f, -0.084425f,  0.738218f,    0.031071f, -0.460971f,  0.886871f,     // 352-359
     0.629048f, -0.367112f,  0.685221f,    0.504094f,  0.803720f, -0.316108f,    0.614711f,  0.617279f, -0.491017f,    0.856420f,  0.306901f, -0.415159f,    0.300512f, -0.165724f,  0.939270f,    0.665870f,  0.744987f,  0.040132f,    0.696754f, -0.704751f,  0.133638f,    0.312481f, -0.664509f,  0.678810f,     // 360-367
     0.338974f, -0.782041f,  0.522979f,    0.201165f,  0.975022f,  0.094155f,    0.388381f,  0.272837f,  0.880182f,   -0.117217f,  0.767540f,  0.630192f,    0.692680f, -0.665885f,  0.277114f,    0.750405f, -0.660956f,  0.005427f,   -0.503450f,  0.127584f,  0.854553f,    0.887023f, -0.014403f,  0.461501f,     // 368-375
     0.840516f, -0.517510f, -0.160362f,   -0.274448f,  0.764687f, -0.583037f,    0.484591f, -0.778249f,  0.399374f,   -0.240559f,  0.681735f,  0.690919f,    0.792723f, -0.077548f,  0.604629f,   -0.317127f,  0.770910f,  0.552384f,    0.128546f, -0.607194f,  0.784087f,    0.249591f,  0.849380f,  0.465035f,     // 376-383
     0.818223f,  0.573854f, -0.034672f,   -0.619613f,  0.722928f, -0.305705f,    0.787775f,  0.331044f,  0.519442f,   -0.189821f,  0.415386f,  0.889619f,    0.847030f, -0.379020f,  0.372671f,    0.611694f, -0.285231f, -0.737884f,   -0.635186f,  0.306284f,  0.709034f,    0.972429f, -0.233050f, -0.008273f,     // 384-391
     0.096693f,  0.773791f, -0.626018f,   -0.043154f,  0.838780f, -0.542757f,    0.058416f,  0.058274f,  0.996590f,   -0.621110f,  0.457155f,  0.636578f,    0.157481f,  0.570658f, -0.805946f,    0.420833f,  0.835362f,  0.353653f,   -0.256647f,  0.923692f,  0.284474f,    0.862065f, -0.157743f,  0.481623f,     // 392-399
     0.619891f,  0.658638f,  0.426534f,    0.767639f,  0.012951f, -0.640752f,    0.864605f,  0.406601f, -0.295186f,    0.688016f, -0.557130f,  0.465016f,    0.248352f,  0.659948f, -0.709077f,    0.571024f,  0.754768f,  0.322887f,    0.753463f,  0.461122f,  0.468679f,    0.155940f, -0.015539f,  0.987644f,     // 400-407
    -0.331426f,  0.442744f,  0.833147f,    0.398418f,  0.552975f, -0.731766f,   -0.497135f,  0.692761f,  0.522436f,   -0.103081f, -0.548100f,  0.830037f,    0.989166f,  0.072558f, -0.127620f,   -0.002166f,  0.988343f,  0.152229f,    0.600464f,  0.331235f, -0.727824f,    0.771027f, -0.503103f, -0.390391f,     // 408-415
     0.350907f,  0.699274f, -0.622800f,   -0.574076f,  0.583724f,  0.574197f,    0.642340f, -0.499800f, -0.581033f,   -0.183586f,  0.965774f, -0.183240f,    0.508060f, -0.306194f,  0.805059f,   -0.212205f, -0.202774f,  0.955956f,    0.594526f,  0.219034f,  0.773669f,   -0.459920f, -0.405337f,  0.790047f,     // 416-423
    -0.281765f, -0.584247f,  0.761094f,   -0.471734f,  0.395166f,  0.788232f,    0.803834f, -0.569814f,  0.170773f,   -0.235343f,  0.963512f,  0.127507f,    0.193416f,  0.811107f, -0.551992f,    0.604940f,  0.776856f,  0.174764f,    0.659197f,  0.552302f,  0.510316f,    0.727246f,  0.140817f, -0.671777f,     // 424-431
    -0.006589f, -0.154830f,  0.987919f,    0.202914f,  0.975420f, -0.085917f,    0.825038f, -0.298605f,  0.479737f,   -0.580120f, -0.359446f,  0.730930f,    0.567458f,  0.818375f, -0.090849f,    0.786188f, -0.137527f, -0.602490f,   -0.246353f,  0.968275f, -0.041879f,   -0.222295f,  0.136689f,  0.965350f,     // 432-439
    -0.380770f,  0.916113f,  0.125507f,    0.722718f,  0.137355f,  0.677357f,    0.955702f,  0.240623f, -0.169513f,    0.598649f, -0.682324f,  0.419588f,   -0.069780f,  0.107804f,  0.991720f,   -0.560306f, -0.085023f,  0.823910f,    0.851892f,  0.194552f,  0.486241f,    0.488016f, -0.447976f,  0.749104f,     // 440-447
    -0.024416f,  0.378871f,  0.925127f,    0.588276f,  0.534052f, -0.607223f,   -0.489866f,  0.516282f,  0.702484f,    0.883168f,  0.414509f,  0.219538f,    0.700725f, -0.324991f, -0.635110f,    0.365332f, -0.279501f,  0.887926f,    0.534252f,  0.438170f,  0.722898f,    0.623531f,  0.686687f, -0.373723f,     // 448-455
     0.003555f,  0.995967f, -0.089647f,   -0.543118f, -0.230883f,  0.807289f,    0.872608f,  0.458005f, -0.169665f,    0.957149f, -0.035379f,  0.287428f,    0.888391f, -0.388408f, -0.244747f,    0.105988f,  0.918952f,  0.379861f,    0.702728f, -0.061599f,  0.708786f,   -0.404707f, -0.250448f,  0.879482f,     // 456-463
    -0.431960f,  0.802447f,  0.411691f,    0.406512f,  0.028237f,  0.913209f,   -0.279180f,  0.294288f,  0.914031f,    0.486520f,  0.740597f, -0.463480f,    0.724477f,  0.623867f, -0.293125f,   -0.539957f,  0.265515f,  0.798716f,    0.371640f,  0.765254f,  0.525614f,    0.289243f,  0.935923f,  0.200966f,     // 464-471
    -0.466058f, -0.008879f,  0.884710f,    0.458724f, -0.133714f,  0.878461f,    0.952889f, -0.279096f,  0.118775f,   -0.116080f,  0.259361f,  0.958779f,   -0.415225f,  0.868271f,  0.271465f,    0.087039f,  0.957947f, -0.273428f,    0.052551f,  0.236544f,  0.970199f,    0.234556f,  0.158756f,  0.959052f,     // 472-479
     0.965980f,  0.170202f,  0.194715f,    0.557956f, -0.611841f,  0.560656f,   -0.396918f,  0.668205f,  0.629252f,    0.923905f, -0.203100f, -0.324270f,    0.092788f,  0.847997f,  0.521816f,   -0.565560f,  0.725560f,  0.392052f,    0.158524f, -0.187192f,  0.969448f,    0.046505f, -0.288490f,  0.956353f,     // 480-487
     0.442788f, -0.573229f,  0.689455f,    0.236175f,  0.762193f,  0.602730f,    0.658635f,  0.391775f,  0.642427f,    0.923334f,  0.239641f, -0.300045f,    0.308148f,  0.366209f,  0.878030f,   -0.228969f, -0.051897f,  0.972049f,    0.914150f,  0.401339f, -0.057061f,    0.083842f,  0.996282f,  0.019844f,     // 488-495
     0.959800f,  0.279927f, -0.020611f,    0.765392f, -0.541440f,  0.347876f,   -0.340471f,  0.069661f,  0.937671f,   -0.073078f, -0.349889f,  0.933936f,    0.860125f, -0.343400f, -0.377174f,   -0.537751f,  0.810939f, -0.230654f,    0.737696f, -0.427314f,  0.522692f,   -0.424924f,  0.713320f, -0.557328f,     // 496-503
     0.351474f, -0.412169f,  0.840585f,    0.989608f,  0.143524f,  0.008760f,    0.930352f,  0.088690f,  0.355780f,    0.074276f,  0.491888f,  0.867484f,    0.726269f,  0.608253f,  0.320252f,    0.860264f,  0.352721f,  0.368149f,   -0.057813f,  0.684544f,  0.726675f,    0.570305f,  0.538966f,  0.619893f,     // 504-511
};

// Produces a random rotation matrix.
void RandomRot(mat4x4& m)
{
    float3 vx = normalize(float3(FRAND*2-1, FRAND*2-1, FRAND*2-1));
    float3 vy = normalize(float3(FRAND*2-1, FRAND*2-1, FRAND*2-1));
    float3 vz = normalize(float3(FRAND*2-1, FRAND*2-1, FRAND*2-1));
    vx = normalize(cross(vy, vz));
    vy = normalize(cross(vx, vz));
    m.col[0] = float4(vx, 0);
    m.col[1] = float4(vy, 0);
    m.col[2] = float4(vz, 0);
    m.col[3] = float4(FRAND, FRAND, FRAND, 1);
}

unsigned __stdcall LoadShaderThread1(void* arg) {
  Demo* demo = (Demo*)arg;
  demo->temp_ps_norm_ = NULL;
  demo->hr_load_shader_1 = LoadShader(demo->getDevice(), L"shaders\\norm.hlsl", "PS", PS_VERSION, kShaderFlags, (void**)&demo->temp_ps_norm_);  // Will provide detailed message if it fails.
  return 0;
}
unsigned __stdcall LoadShaderThread2(void* arg) {
  Demo* demo = (Demo*)arg;
  demo->temp_ps_geom_ = NULL;
  demo->hr_load_shader_2 = LoadShader(demo->getDevice(), L"shaders\\geom.hlsl", "PS", PS_VERSION, kShaderFlags, (void**)&demo->temp_ps_geom_);  // Will provide detailed message if it fails.
  return 0;
}
unsigned __stdcall LoadShaderThread3(void* arg) {
  Demo* demo = (Demo*)arg;
  demo->temp_ps_ambo_ = NULL;
  demo->hr_load_shader_3 = LoadShader(demo->getDevice(), L"shaders\\ambo.hlsl", "PS", PS_VERSION, kShaderFlags, (void**)&demo->temp_ps_ambo_);  // Will provide detailed message if it fails.
  return 0;
}
unsigned __stdcall LoadShaderThread4(void* arg) {
  Demo* demo = (Demo*)arg;
  
  demo->hr_load_shader_4 = S_OK;
  HRESULT hr;

  demo->temp_vs_quad_fullscreen_ = NULL;
  demo->temp_vs_quad_tile_ = NULL;
  demo->temp_ps_relight_ = NULL;

  hr = LoadShader(demo->getDevice(), L"shaders\\quad_fullscreen.hlsl", "VS", VS_VERSION, kShaderFlags, (void**)&demo->temp_vs_quad_fullscreen_);  // Will provide detailed message if it fails.
  if (FAILED(hr)) demo->hr_load_shader_4 = hr;
  hr = LoadShader(demo->getDevice(), L"shaders\\quad_tile.hlsl", "VS", VS_VERSION, kShaderFlags, (void**)&demo->temp_vs_quad_tile_);  // Will provide detailed message if it fails.
  if (FAILED(hr)) demo->hr_load_shader_4 = hr;
  hr = LoadShader(demo->getDevice(), L"shaders\\relight.hlsl", "PS", PS_VERSION, kShaderFlags, (void**)&demo->temp_ps_relight_);  // Will provide detailed message if it fails.
  if (FAILED(hr)) demo->hr_load_shader_4 = hr;
  return 0;
}

HRESULT LoadShader(ID3D11Device* pDevice,
                   LPCTSTR shader_filename, 
                   LPCSTR function_name, 
                   LPCSTR shader_profile, 
                   UINT shader_flags, 
                   void** pShader) 
{
  double start_time_seconds = NGetTime();

  HRESULT hr;
  ID3DBlob* shader_blob = NULL;
  ID3DBlob* error_msg_blob = NULL;
  hr = D3DX11CompileFromFile(shader_filename, NULL /*#define symbols*/, NULL /*extra #includes*/,
                             function_name, shader_profile, shader_flags, NULL, NULL, &shader_blob, &error_msg_blob, NULL);
  if (FAILED(hr)) {
    char buf[16384];
    sprintf_s(buf, sizeof(buf)/sizeof(buf[0]), "Failed to compile shader\nFile: %S\nFunction: %s\nError message: %s", shader_filename, function_name, error_msg_blob ? error_msg_blob->GetBufferPointer() : "<no error message>");
    MessageBoxA(NULL, buf, "Error", MB_ICONERROR);
    SAFE_RELEASE( error_msg_blob );
    return hr;
  }
  SAFE_RELEASE( error_msg_blob );

  if (shader_profile[0] == 'v')
    hr = pDevice->CreateVertexShader( shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), NULL, (ID3D11VertexShader**)pShader );
  else if (shader_profile[0] == 'p')
    hr = pDevice->CreatePixelShader( shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), NULL, (ID3D11PixelShader**)pShader );
  else {
    char buf[16384];
    sprintf_s(buf, sizeof(buf)/sizeof(buf[0]), "Unrecognized shader profile type '%s'\nFile: %S\nFunction: %s", shader_profile, shader_filename, function_name);
    MessageBoxA(NULL, buf, "Error", MB_ICONERROR);
    return E_FAIL;
  }
  if (FAILED(hr)) {
    char buf[16384];
    sprintf_s(buf, sizeof(buf)/sizeof(buf[0]), "Failed to create shader\nFile: %S\nFunction: %s", shader_filename, function_name);
    MessageBoxA(NULL, buf, "Error", MB_ICONERROR);
    return hr;
  }

  SAFE_RELEASE( shader_blob );

  double end_time_seconds = NGetTime();

  char buf[2048];
  sprintf_s(buf, sizeof(buf)/sizeof(buf[0]), "Took %.2f seconds to load shader: %S\n", end_time_seconds - start_time_seconds, shader_filename);
  OutputDebugStringA(buf);

  return S_OK;
}

HRESULT RenderTarget::Init(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format)
{
  // Create render targets & corresponding views.
  HRESULT hr;

  // Create the render target texture.
  D3D11_TEXTURE2D_DESC td;
  td.Width  = width;
  td.Height = height;
  td.Format = format;
  td.MipLevels = 1;
  td.ArraySize = 1;
  td.SampleDesc.Count = 1;
  td.SampleDesc.Quality = 0;
  td.Usage = D3D11_USAGE_DEFAULT;
  td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  td.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
  td.MiscFlags = 0;   //D3D11_RESOURCE_MISC_GENERATE_MIPS

  hr = pDevice->CreateTexture2D(&td, NULL, &tex);
  if (FAILED(hr)) { 
    MessageBoxA(NULL, "CreateTexture2D() failed.", "Error", MB_ICONERROR); 
    return hr;
  }

  // Create the view to render to it.
  D3D11_RENDER_TARGET_VIEW_DESC vd;
  vd.Format = format;
  vd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  vd.Texture2D.MipSlice = 0;

  hr = pDevice->CreateRenderTargetView(tex, &vd, &rtview);
  if (FAILED(hr)) { 
    MessageBoxA(NULL, "CreateRenderTargetView() failed.", "Error", MB_ICONERROR); 
    return hr;
  }

  // Create the view to use it as a texture.
  D3D11_SHADER_RESOURCE_VIEW_DESC rd;
  rd.Format = format;
  rd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  rd.Texture2D.MipLevels = 1;
  rd.Texture2D.MostDetailedMip = 0;  //KIV
 
  hr = pDevice->CreateShaderResourceView(tex, &rd, &srview);
  if (FAILED(hr)) { 
    MessageBoxA(NULL, "CreateShaderResourceView() failed.", "Error", MB_ICONERROR); 
    return hr;
  }  

  return S_OK;
}

void FillWithRandomFloat(float* pData, int count)
{
  for (int i = 0; i < count; ++i) 
    pData[i] = FRAND;
}

void FillWithRandomUint32(unsigned __int32* pData, int count)
{
  for (int i = 0; i < count; ++i) 
    pData[i] = NRAND;
}
void FillWithRandomUint16(unsigned __int16* pData, int count)
{
  for (int i = 0; i < count; ++i) 
    pData[i] = (unsigned __int16)(NRAND & 0xFFFF);
}

HRESULT TextureFromDisk::Init(ID3D11Device* pDevice, DXGI_FORMAT format, const char* szFilename)
{
  D3DX11_IMAGE_LOAD_INFO info;
  info.Width = D3DX11_DEFAULT ;
  info.Height = D3DX11_DEFAULT ;
  info.Depth = 1;
  info.FirstMipLevel = 0;
  info.MipLevels = 1;
  info.Usage = D3D11_USAGE_DEFAULT;
  info.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  info.MiscFlags = 0;
  info.CpuAccessFlags = 0;
  info.Format = format;//DXGI_FORMAT_R8G8B8A8_UNORM;
  info.Filter = D3DX11_DEFAULT;
  info.MipFilter = D3DX11_DEFAULT;
  info.pSrcInfo = NULL;

  HRESULT hr = D3DX11CreateShaderResourceViewFromFileA(pDevice, szFilename, &info, NULL, &srview, NULL);
  if (FAILED(hr)) { 
    MessageBoxA(NULL, "D3DX11CreateShaderResourceViewFromFile() failed.", "Error", MB_ICONERROR); 
    return hr;
  }
  return S_OK;
}

HRESULT Texture1D::Init(ID3D11Device* pDevice, int width, DXGI_FORMAT format, FillWith fill)
{
  // Create render targets & corresponding views.
  HRESULT hr;

  // Create the render target texture.
  D3D11_TEXTURE1D_DESC td;
  td.Width  = width;
  td.MipLevels = 1;   // TODO: add mip levels.
  td.ArraySize = 1;
  td.Format = format;
  td.Usage = D3D11_USAGE_DEFAULT;
  td.BindFlags = D3D11_BIND_SHADER_RESOURCE;// | D3D11_BIND_RENDER_TARGET;
  td.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
  td.MiscFlags = 0;//D3D11_RESOURCE_MISC_GENERATE_MIPS;

  if (fill == ProgressiveAmboRayDirs)
  {
    if (format == DXGI_FORMAT_R32G32B32A32_FLOAT)
    {
      int ch = 4;
      int bytes_per_scalar_value = 4;
      float* pData = new float[width * ch];

      FILE* f = NULL;
      fopen_s(&f, AMBO_RAY_DIR_FILENAME, "r");
      if (!f) {
        MessageBoxA(NULL, "CreateTexture1D(): file not found: " AMBO_RAY_DIR_FILENAME, "Error", MB_ICONERROR); 
        return E_FAIL;
      }
      for (int i=0; i<width; i++)
      {
        float x, y, z;
        if (fscanf_s(f, "%f %f %f", &x, &y, &z) != 3) {
          MessageBoxA(NULL, "CreateTexture1D(): file incomplete: " AMBO_RAY_DIR_FILENAME, "Error", MB_ICONERROR); 
          return E_FAIL;
        }
        pData[i*ch+0] = x;
        pData[i*ch+1] = y;
        pData[i*ch+2] = z;
        pData[i*ch+3] = FRAND;
      }
      fclose(f);
      
      D3D11_SUBRESOURCE_DATA initial_data;
      initial_data.pSysMem = pData;
      initial_data.SysMemPitch = width * ch * bytes_per_scalar_value;
      initial_data.SysMemSlicePitch = 0;//width * height * ch * bytes_per_scalar_value;
      hr = pDevice->CreateTexture1D(&td, &initial_data, &tex);
      delete pData;
    }
    else
    {
      MessageBoxA(NULL, "CreateTexture1D(): UNIMPLEMENTED: don't know how to generate noise for that format.", "Error", MB_ICONERROR); 
      return E_FAIL;
    }
  }
  else if (fill == HomogenousNormalRayDirs)
  {
    if (format == DXGI_FORMAT_R32G32B32A32_FLOAT)
    {
      int ch = 4;
      int bytes_per_scalar_value = 4;
      float* pData = new float[width * ch];

      FILE* f = NULL;
      fopen_s(&f, NORMAL_RAY_DIR_FILENAME, "r");
      if (!f) {
        MessageBoxA(NULL, "CreateTexture1D(): file not found: " NORMAL_RAY_DIR_FILENAME, "Error", MB_ICONERROR); 
        return E_FAIL;
      }
      for (int i=0; i<width; i++)
      {
        float x, y, z;
        if (fscanf_s(f, "%f %f %f", &x, &y, &z) != 3) {
          MessageBoxA(NULL, "CreateTexture1D(): file incomplete: " NORMAL_RAY_DIR_FILENAME, "Error", MB_ICONERROR); 
          return E_FAIL;
        }
        pData[i*ch+0] = x;
        pData[i*ch+1] = y;
        pData[i*ch+2] = z;
        pData[i*ch+3] = FRAND;
      }
      fclose(f);
      
      D3D11_SUBRESOURCE_DATA initial_data;
      initial_data.pSysMem = pData;
      initial_data.SysMemPitch = width * ch * bytes_per_scalar_value;
      initial_data.SysMemSlicePitch = 0;//width * height * ch * bytes_per_scalar_value;
      hr = pDevice->CreateTexture1D(&td, &initial_data, &tex);
      delete pData;
    }
    else
    {
      MessageBoxA(NULL, "CreateTexture1D(): UNIMPLEMENTED: don't know how to generate noise for that format.", "Error", MB_ICONERROR); 
      return E_FAIL;
    }
  }
  else if (fill == SinCos)
  {
    if (format == DXGI_FORMAT_R16G16_SNORM)
    {
      int ch = 2;
      int bytes_per_scalar_value = 2;
      signed __int16* pData = new signed __int16[width * ch];
      for (int i=0; i<width; i++)
      {
        double angle = 3.1415926535897932384626433832795 * 2.0 * (double)i / (double)width;
        pData[i*2+0] = (signed __int16)(sin(angle) * 32767);
        pData[i*2+1] = (signed __int16)(cos(angle) * 32767);
      }
      
      D3D11_SUBRESOURCE_DATA initial_data;
      initial_data.pSysMem = pData;
      initial_data.SysMemPitch = width * ch * bytes_per_scalar_value;
      initial_data.SysMemSlicePitch = 0;//width * height * ch * bytes_per_scalar_value;
      hr = pDevice->CreateTexture1D(&td, &initial_data, &tex);
      delete pData;
    }
    else
    {
      MessageBoxA(NULL, "CreateTexture1D(): UNIMPLEMENTED: don't know how to generate noise for that format.", "Error", MB_ICONERROR); 
      return E_FAIL;
    }
  }
  else if (fill == Noise)
  {
    if (format == DXGI_FORMAT_R16_UINT  || 
        format == DXGI_FORMAT_R16_SINT  ||
        format == DXGI_FORMAT_R16_UNORM ||
        format == DXGI_FORMAT_R16_SNORM)
    {
      int ch = 1;
      int bytes_per_scalar_value = 2;
      unsigned __int16* pData = new unsigned __int16[width * ch];
      FillWithRandomUint16(pData, width * ch);
      D3D11_SUBRESOURCE_DATA initial_data;
      initial_data.pSysMem = pData;
      initial_data.SysMemPitch = width * ch * bytes_per_scalar_value;
      initial_data.SysMemSlicePitch = 0;//width * height * ch * bytes_per_scalar_value;
      hr = pDevice->CreateTexture1D(&td, &initial_data, &tex);
      delete pData;
    }
    else if (format == DXGI_FORMAT_R16G16_UINT  || 
             format == DXGI_FORMAT_R16G16_SINT  ||
             format == DXGI_FORMAT_R16G16_UNORM ||
             format == DXGI_FORMAT_R16G16_SNORM)
    {
      int ch = 2;
      int bytes_per_scalar_value = 2;
      unsigned __int16* pData = new unsigned __int16[width * ch];
      FillWithRandomUint16(pData, width * ch);
      D3D11_SUBRESOURCE_DATA initial_data;
      initial_data.pSysMem = pData;
      initial_data.SysMemPitch = width * ch * bytes_per_scalar_value;
      initial_data.SysMemSlicePitch = 0;//width * height * ch * bytes_per_scalar_value;
      hr = pDevice->CreateTexture1D(&td, &initial_data, &tex);
      delete pData;
    }
    else if (format == DXGI_FORMAT_R16G16B16A16_UINT  || 
             format == DXGI_FORMAT_R16G16B16A16_SINT  ||
             format == DXGI_FORMAT_R16G16B16A16_UNORM ||
             format == DXGI_FORMAT_R16G16B16A16_SNORM)
    {
      int ch = 4;
      int bytes_per_scalar_value = 2;
      unsigned __int16* pData = new unsigned __int16[width * ch];
      FillWithRandomUint16(pData, width * ch);
      D3D11_SUBRESOURCE_DATA initial_data;
      initial_data.pSysMem = pData;
      initial_data.SysMemPitch = width * ch * bytes_per_scalar_value;
      initial_data.SysMemSlicePitch = 0;//width * height * ch * bytes_per_scalar_value;
      hr = pDevice->CreateTexture1D(&td, &initial_data, &tex);
      delete pData;
    }
    else
    {
      MessageBoxA(NULL, "CreateTexture1D(): UNIMPLEMENTED: don't know how to generate noise for that format.", "Error", MB_ICONERROR); 
      return E_FAIL;
    }
  }
  else 
  {
    hr = pDevice->CreateTexture1D(&td, NULL, &tex);
  }
  if (FAILED(hr)) { 
    MessageBoxA(NULL, "CreateTexture1D() failed.", "Error", MB_ICONERROR); 
    return hr;
  }

  // Create the view to use it as a texture.
  D3D11_SHADER_RESOURCE_VIEW_DESC rd;
  rd.Format = format;
  rd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
  rd.Texture3D.MipLevels = 1;
  rd.Texture3D.MostDetailedMip = 0;  //KIV
  
  hr = pDevice->CreateShaderResourceView(tex, &rd, &srview);
  if (FAILED(hr)) { 
    MessageBoxA(NULL, "CreateShaderResourceView() failed.", "Error", MB_ICONERROR); 
    return hr;
  }

  return S_OK;
}

HRESULT TextureCube::Init(ID3D11Device* pDevice, const char* szDdsFile) {
  D3DX11_IMAGE_LOAD_INFO loadSMInfo;
	loadSMInfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hr = D3DX11CreateTextureFromFileA(pDevice, szDdsFile, &loadSMInfo, 0, (ID3D11Resource**)&tex, 0);
  if (FAILED(hr)) { 
    MessageBoxA(NULL, "D3DX11CreateTextureFromFileA() failed.", "Error", MB_ICONERROR); 
    return hr;
  }

	D3D11_TEXTURE2D_DESC SMTextureDesc;
	tex->GetDesc(&SMTextureDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = SMTextureDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = SMTextureDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	hr = pDevice->CreateShaderResourceView(tex, &SMViewDesc, &srview);
  if (FAILED(hr)) { 
    MessageBoxA(NULL, "CreateShaderResourceView() failed.", "Error", MB_ICONERROR); 
    return hr;
  }

  return S_OK;
}


HRESULT Texture2D::Init(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format, bool fill_with_noise)
{
  // Create render targets & corresponding views.
  HRESULT hr;

  // Create the render target texture.
  D3D11_TEXTURE2D_DESC td;
  td.Width  = width;
  td.Height = height;
  td.ArraySize = 1;
  td.MipLevels = 1;   // TODO: add mip levels.
  td.Format = format;
  td.SampleDesc.Count = 1;
  td.SampleDesc.Quality = 0;
  td.Usage = D3D11_USAGE_DEFAULT;
  td.BindFlags = D3D11_BIND_SHADER_RESOURCE;// | D3D11_BIND_RENDER_TARGET;
  td.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
  td.MiscFlags = 0;//D3D11_RESOURCE_MISC_GENERATE_MIPS;

  if (fill_with_noise)
  {
    if (format == DXGI_FORMAT_R16G16B16A16_UNORM ||
        format == DXGI_FORMAT_R16G16B16A16_SNORM ||
        format == DXGI_FORMAT_R16G16B16A16_UINT  ||
        format == DXGI_FORMAT_R16G16B16A16_SINT)
    {
      unsigned __int16* pData = new unsigned __int16[width * height * 4];
      for (int i = 0; i < width * height * 4; ++i)
        pData[i] = (unsigned __int16)((NRAND & 0xFFFF));
      D3D11_SUBRESOURCE_DATA initial_data;
      initial_data.pSysMem = pData;
      initial_data.SysMemPitch = width * sizeof(__int16) * 4;
      initial_data.SysMemSlicePitch = width * height * sizeof(__int16) * 4;
      hr = pDevice->CreateTexture2D(&td, &initial_data, &tex);
      delete pData;
    } 
    else 
    {
      MessageBoxA(NULL, "CreateTexture2D(): UNIMPLEMENTED: don't know how to generate noise for that format.", "Error", MB_ICONERROR); 
      return E_FAIL;
    }
  } 
  else 
  {
    hr = pDevice->CreateTexture2D(&td, NULL, &tex);
  }
  if (FAILED(hr)) { 
    MessageBoxA(NULL, "CreateTexture2D() failed.", "Error", MB_ICONERROR); 
    return hr;
  }

  // Create the view to use it as a texture.
  D3D11_SHADER_RESOURCE_VIEW_DESC rd;
  rd.Format = format;
  rd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  rd.Texture2D.MipLevels = 1;
  rd.Texture2D.MostDetailedMip = 0;  //KIV
  
  hr = pDevice->CreateShaderResourceView(tex, &rd, &srview);
  if (FAILED(hr)) { 
    MessageBoxA(NULL, "CreateShaderResourceView() failed.", "Error", MB_ICONERROR); 
    return hr;
  }  

  return S_OK;
}

void PackNeighbors(void* pData, const int width, const int height, const int depth, const int bytes_per_channel)
{
  // In the .yzw of each texel, store the .x value from the { +x, +y, and +xy } neighbor texels.
  // We assume that the data is 4 channels, and the values in .yzw are discarded, of course.

  if (bytes_per_channel == 1) {
    char* p = (char*)pData;
    for (int k = 0; k < depth; ++k) {
      for (int j = 0; j < height; ++j) {
        int j2 = (j + 1) % height;
        for (int i = 0; i < width; ++i) {
          int i2 = (i + 1) % width;
          p[ (((k * height) + j) * width + i) * 4 + 1 ] = p[ (((k * height) + j ) * width + i2) * 4 + 0 ];
          p[ (((k * height) + j) * width + i) * 4 + 2 ] = p[ (((k * height) + j2) * width + i ) * 4 + 0 ];
          p[ (((k * height) + j) * width + i) * 4 + 3 ] = p[ (((k * height) + j2) * width + i2) * 4 + 0 ];
        }
      }
    }
  }
  else if (bytes_per_channel == 2) {
    __int16* p = (__int16*)pData;
    for (int k = 0; k < depth; ++k) {
      for (int j = 0; j < height; ++j) {
        int j2 = (j + 1) % height;
        for (int i = 0; i < width; ++i) {
          int i2 = (i + 1) % width;
          p[ (((k * height) + j) * width + i) * 4 + 1 ] = p[ (((k * height) + j ) * width + i2) * 4 + 0 ];
          p[ (((k * height) + j) * width + i) * 4 + 2 ] = p[ (((k * height) + j2) * width + i ) * 4 + 0 ];
          p[ (((k * height) + j) * width + i) * 4 + 3 ] = p[ (((k * height) + j2) * width + i2) * 4 + 0 ];
        }
      }
    }
  }
  else if (bytes_per_channel == 4) {
    __int32* p = (__int32*)pData;
    for (int k = 0; k < depth; ++k) {
      for (int j = 0; j < height; ++j) {
        int j2 = (j + 1) % height;
        for (int i = 0; i < width; ++i) {
          int i2 = (i + 1) % width;
          p[ (((k * height) + j) * width + i) * 4 + 1 ] = p[ (((k * height) + j ) * width + i2) * 4 + 0 ];
          p[ (((k * height) + j) * width + i) * 4 + 2 ] = p[ (((k * height) + j2) * width + i ) * 4 + 0 ];
          p[ (((k * height) + j) * width + i) * 4 + 3 ] = p[ (((k * height) + j2) * width + i2) * 4 + 0 ];
        }
      }
    }
  }
  else {
    MessageBoxA(NULL, "CreateTexture3D/PackNeighbors(): UNIMPLEMENTED: unknown bytes per texel.", "Error", MB_ICONERROR); 
  }
}

HRESULT Texture3D::Init(ID3D11Device* pDevice, int width, int height, int depth, DXGI_FORMAT format, bool bFillWithNoise, bool bPackNeighbors)
{
  // Create render targets & corresponding views.
  HRESULT hr;

  // Create the render target texture.
  D3D11_TEXTURE3D_DESC td;
  td.Width  = width;
  td.Height = height;
  td.Depth  = depth;
  td.MipLevels = 1;   // TODO: add mip levels.
  td.Format = format;
  td.Usage = D3D11_USAGE_DEFAULT;
  td.BindFlags = D3D11_BIND_SHADER_RESOURCE;// | D3D11_BIND_RENDER_TARGET;
  td.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
  td.MiscFlags = 0;//D3D11_RESOURCE_MISC_GENERATE_MIPS;

  if (bFillWithNoise)
  {
    // Recommended format: DXGI_FORMAT_R8G8B8A8_SNORM, because the trinlinear 
    //   interpolation happens in full float32, for all of the _SNORM types.  
    //   Use DXGI_FORMAT_R16G16B16A16_SNORM only if you need more than 256 
    //   possible values *at the corners*; interp is the same perfect fp32 quality.
    // Testing history: 
    //   The format you use here doesn't matter much; when sampling within a voxel, 
    //   values will be sampled from the 8 corners, and then converted to full float, 
    //   and THEN interpolated.  This was verified by right-shifting the _SINT values
    //   (stored here) until they only had 3 or 4 bits in them, then sampling, and 
    //   scaling up by the inverse.  When you do this, and look up close at the 
    //   intra-voxel region, there is *never* any banding, which proves it.  
    //   This happened both for DXGI_FORMAT_R8G8B8A8_SNORM and DXGI_FORMAT_R16G16B16A16_SNORM.
    //   These results were further confirmed both with X61's Intel GPU (HW), and with 
    //   D3D_DRIVER_TYPE_REFERENCE.
    // 
    if (format == DXGI_FORMAT_R32G32B32A32_FLOAT)
    {
      float* pData = new float[width * height * depth * 4];
      for (int i = 0; i < width * height * depth * 4; ++i)
        pData[i] = (float)(NRAND & 0xFFFF) * (1.0f/65535.0f) * 2 - 1;
      if (bPackNeighbors) 
        PackNeighbors((void*)pData, width, height, depth, 4);
      D3D11_SUBRESOURCE_DATA initial_data;
      initial_data.pSysMem = pData;
      initial_data.SysMemPitch = width * sizeof(float) * 4;
      initial_data.SysMemSlicePitch = width * height * sizeof(float) * 4;
      hr = pDevice->CreateTexture3D(&td, &initial_data, &tex);
      delete pData;
    }
    else if (format == DXGI_FORMAT_R32G32B32A32_SINT  ||
             format == DXGI_FORMAT_R32G32B32A32_UINT) 
    {
      unsigned __int32* pData = new unsigned __int32[width * height * depth * 4];
      for (int i = 0; i < width * height * depth * 4; ++i)
        pData[i] = NRAND;
      if (bPackNeighbors) 
        PackNeighbors((void*)pData, width, height, depth, 4);
      D3D11_SUBRESOURCE_DATA initial_data;
      initial_data.pSysMem = pData;
      initial_data.SysMemPitch = width * sizeof(__int32) * 4;
      initial_data.SysMemSlicePitch = width * height * sizeof(__int32) * 4;
      hr = pDevice->CreateTexture3D(&td, &initial_data, &tex);
      delete pData;
    } 
    else if (format == DXGI_FORMAT_R16G16B16A16_SNORM || 
             format == DXGI_FORMAT_R16G16B16A16_UNORM || 
             format == DXGI_FORMAT_R16G16B16A16_SINT || 
             format == DXGI_FORMAT_R16G16B16A16_UINT) 
    {
      unsigned __int16* pData = new unsigned __int16[width * height * depth * 4];
      for (int i = 0; i < width * height * depth * 4; ++i)
        pData[i] = (unsigned __int16)(NRAND & 0xFFFF);
      if (bPackNeighbors) 
        PackNeighbors((void*)pData, width, height, depth, sizeof(__int16));
      D3D11_SUBRESOURCE_DATA initial_data;
      initial_data.pSysMem = pData;
      initial_data.SysMemPitch = width * sizeof(__int16) * 4;
      initial_data.SysMemSlicePitch = width * height * sizeof(__int16) * 4;
      hr = pDevice->CreateTexture3D(&td, &initial_data, &tex);
      delete pData;
    } 
    else if (format == DXGI_FORMAT_R8G8B8A8_UINT ||
             format == DXGI_FORMAT_R8G8B8A8_SINT ||
             format == DXGI_FORMAT_R8G8B8A8_UNORM ||
             format == DXGI_FORMAT_R8G8B8A8_SNORM) 
    {
      unsigned char* pData = new unsigned char[width * height * depth * 4];
      for (int i = 0; i < width * height * depth * 4; ++i)
        pData[i] = (unsigned char)(NRAND & 0xFF);
      if (bPackNeighbors) 
        PackNeighbors((void*)pData, width, height, depth, sizeof(char));
      D3D11_SUBRESOURCE_DATA initial_data;
      initial_data.pSysMem = pData;
      initial_data.SysMemPitch = width * sizeof(char) * 4;
      initial_data.SysMemSlicePitch = width * height * sizeof(char) * 4;
      hr = pDevice->CreateTexture3D(&td, &initial_data, &tex);
      delete pData;
    } 
    else 
    {
      MessageBoxA(NULL, "CreateTexture3D(): UNIMPLEMENTED: don't know how to generate noise for that format.", "Error", MB_ICONERROR); 
      return E_FAIL;
    }
  } 
  else 
  {
    hr = pDevice->CreateTexture3D(&td, NULL, &tex);
  }
  if (FAILED(hr)) { 
    MessageBoxA(NULL, "CreateTexture3D() failed.", "Error", MB_ICONERROR); 
    return hr;
  }

  // Create the view to use it as a texture.
  D3D11_SHADER_RESOURCE_VIEW_DESC rd;
  rd.Format = format;
  rd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
  rd.Texture3D.MipLevels = 1;
  rd.Texture3D.MostDetailedMip = 0;  //KIV
  
  hr = pDevice->CreateShaderResourceView(tex, &rd, &srview);
  if (FAILED(hr)) { 
    MessageBoxA(NULL, "CreateShaderResourceView() failed.", "Error", MB_ICONERROR); 
    return hr;
  }  

  return S_OK;
}


// Time is in seconds
// First call returns 0.0 and subsequent calls return the change in time
double Demo::GetTimeInSeconds() 
{
  // returns the amount of time that has elapsed 
  // since the FIRST call to NGetTime, in seconds.
  // (starts at zero)
  // 10000 = 0.1 ms accuracy.  Don't go higher or cast to (double) 
  // @ end will start to lose precision!
  const DWORD desired_ticks_per_second = 10000;   
  static DWORD quadpart_shift_bits = 0xFFFFFFFF;
  static double freq_after_shift;
  static LARGE_INTEGER orig_time;
  static double extra_time;
  static double last_time;

  if (quadpart_shift_bits == 0xFFFFFFFF) {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&orig_time);

    // if freq is 10 trillion and we only really need 'desired_ticks_per_second',
    // figure out how many bits we can lop off (>>) before doing any casts to (double),
    // to minimize precision losses.
    double instant_div = (double)freq.QuadPart / (double)desired_ticks_per_second;
    quadpart_shift_bits = (DWORD)(log(instant_div) / log(2.0));
    freq_after_shift = (double)freq.QuadPart / (double)pow(2.0, (double)quadpart_shift_bits);

    // for when .QuadPart wraps:
    extra_time = 0;
    last_time = 0;
  }

  /*
  // check to make sure freq isn't squirreling on us...
  LARGE_INTEGER freq_check;
  static LARGE_INTEGER freq;
  static bool bFreqReady = false;
  if (!bFreqReady) {
    QueryPerformanceFrequency(&freq);
    bFreqReady = true;
  }
  QueryPerformanceFrequency(&freq_check);
  assert(freq.LowPart == freq_check.LowPart);
  assert(freq.HighPart == freq_check.HighPart);
  */

  //if (!g_bTimeIsBeingOverridden)
  {
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);

    // catch wraps:
    if (time.QuadPart < orig_time.QuadPart) {
      OutputDebugStringA("NCommon: timer wrap\n");
      extra_time = last_time;
      orig_time.QuadPart = time.QuadPart;
    }
    
    // very important: NEVER ever cast the absolute .QuadPart straight to a double; 
    //   precision will suck.  (you drop down to 30-ms ticks)
    // you could just case the .LowPart to a double, but then 
    //   you have to watch it the .HighPart for ticks
    //   (which happens every 20 minutes if the freq is 3,579,000),
    //   and adjust accordingly.
    // so we tried subtracting the .QuadPart's as int64's
    //   to get the relative change (since the app began)
    //   (this has no precision loss), then casting to double hoping it would be fine,
    //   even if the app has been running for months...
    //   but it turns out the 'freq' is so damn huge, this can still give
    //   you VERY jump time sampling.
    // so now, we preemptively lop off some # of the bits in the LARGE_INTEGER domain,
    //   and THEN use that technique.  After lopping off those early bits 
    //   the cast to (double) is just fine.
    LONGLONG steps = (time.QuadPart - orig_time.QuadPart) >> quadpart_shift_bits;
    last_time = (double)steps / (double)freq_after_shift + extra_time;
  }
  /*else
  {
    last_time = g_dOverrideTime;
  }*/

  return last_time;
}

/*
bool Demo::SaveImageToDiskUsingD3DX()  // (SLOW...)
{
  const D3DX11_IMAGE_FILE_FORMAT kImageSaveFormat = D3DX11_IFF_TIFF; // D3DX11_IFF_JPG, D3DX11_IFF_PNG, or D3DX11_IFF_TIFF

  TCHAR szDir[1024];
  swprintf_s(szDir, sizeof(szDir)/sizeof(szDir[0]), L"%s%s", parent_dir_, L"screenshots");
  CreateDirectory(szDir, NULL);

  TCHAR base_filename[512];
  swprintf_s(base_filename, sizeof(base_filename)/sizeof(base_filename[0]), L"img_%08x__%08x_%04d", cb_data_.g_scene_seed, time(0), cb_data_.g_frame);
  TCHAR filename[512];
  swprintf_s(filename, sizeof(filename)/sizeof(filename[0]), L"%s\\%s", szDir, base_filename);
  
  switch (image_format)
  {
    case D3DX11_IFF_BMP : wcscat_s(filename, sizeof(filename)/sizeof(filename[0]), L".bmp"); break; // size: 100%
    case D3DX11_IFF_JPG : wcscat_s(filename, sizeof(filename)/sizeof(filename[0]), L".jpg"); break; // size: ~10%
    case D3DX11_IFF_PNG : wcscat_s(filename, sizeof(filename)/sizeof(filename[0]), L".png"); break;
    case D3DX11_IFF_DDS : wcscat_s(filename, sizeof(filename)/sizeof(filename[0]), L".dds"); break;
    case D3DX11_IFF_TIFF: wcscat_s(filename, sizeof(filename)/sizeof(filename[0]), L".tif"); break; // size: ~70%
    case D3DX11_IFF_GIF : wcscat_s(filename, sizeof(filename)/sizeof(filename[0]), L".gif"); break;
    case D3DX11_IFF_WMP : wcscat_s(filename, sizeof(filename)/sizeof(filename[0]), L".wmp"); break;
    default:              
      MessageBoxA(hwnd_, "SaveImageToDisk(): unknown format.", "Error", MB_ICONERROR); 
      return false;
  }

  ID3D11Texture2D* pBackBuffer = NULL;
  HRESULT hr = swap_chain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
  if (FAILED(hr)) {
    MessageBoxA(hwnd_, "SwapChain::GetBuffer() failed.", "Error", MB_ICONERROR); 
    return false;
  }

  hr = D3DX11SaveTextureToFile( immed_context_, pBackBuffer, image_format, filename);
  pBackBuffer->Release();
  if (FAILED(hr)) {
    MessageBoxA(hwnd_, "D3DX11SaveTextureToFile() failed.", "Error", MB_ICONERROR); 
    return false;
  }

  // Export the current scene (camera, lights, etc.) to a file. 
  TCHAR scene_file[2048];
  swprintf_s(scene_file, sizeof(scene_file)/sizeof(scene_file[0]), 
             L"%sbin\\_screenshot_.scn", 
             parent_dir_);
  SetCurrentDirectory(parent_dir_);
  bool bSuccess = Export(scene_file);

  // Finally, create a .zip with the .exe, shaders, code, scene file, etc.
  char cmdline[2048];
  sprintf_s(cmdline, sizeof(cmdline)/sizeof(cmdline[0]), 
    "bin\\rar.exe a screenshots\\%S.zip bin\\gpucaster*.exe bin\\_screenshot_.scn shaders\\*.* code\\*.*",
    base_filename);
  SetCurrentDirectory(parent_dir_);
  system(cmdline);  

  return true;
}
*/

bool Demo::SaveImageToDiskUncompressed()
{
  TCHAR szDir[1024];
  swprintf_s(szDir, sizeof(szDir)/sizeof(szDir[0]), L"%s%s", parent_dir_, L"screenshots");
  CreateDirectory(szDir, NULL);

  TCHAR base_filename[512];
  swprintf_s(base_filename, sizeof(base_filename)/sizeof(base_filename[0]), L"img_%08x__%08x_%04d", cb_data_.g_scene_seed, time(0), cb_data_.g_main_frame + cb_data_.g_refl_frame);
  TCHAR filename[512];
  swprintf_s(filename, sizeof(filename)/sizeof(filename[0]), L"%s\\%s", szDir, base_filename);
  wcscat_s(filename, sizeof(filename)/sizeof(filename[0]), L".tga");

  HRESULT hr = S_OK;

  /*
  ID3D11Texture2D* pBackBuffer = NULL;
  hr = swap_chain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
  if (FAILED(hr)) {
    MessageBoxA(hwnd_, "SwapChain::GetBuffer() failed.", "Error", MB_ICONERROR); 
    return false;
  }
  */

  // Create a temporary CPU-side texture that is a thin horizontal slice.
  int h = height_;// / super_sample_;
  ID3D11Texture2D* temp_tex = NULL;
  D3D11_TEXTURE2D_DESC td;
  td.Width  = width_ * super_sample_;
  td.Height = h;
  td.Format = DXGI_FORMAT_R8G8B8A8_UINT;
  td.MipLevels = 1;
  td.ArraySize = 1;
  td.SampleDesc.Count = 1;
  td.SampleDesc.Quality = 0;
  td.Usage = D3D11_USAGE_STAGING;
  td.BindFlags = 0;
  td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  td.MiscFlags = 0;
  device_->CreateTexture2D(&td, NULL, &temp_tex);

  // Open TGA file and write header.
  FILE* f = NULL;
  _wfopen_s(&f, filename, L"wb");
  if (!f) {
    MessageBox(hwnd_, filename, L"Unable to open file for output:", MB_ICONERROR); 
    SAFE_RELEASE(temp_tex);
    //SAFE_RELEASE(pBackBuffer);
    return false;
  }
  fprintf(f, "%c%c%c%c%c%c",  0,  0,  2,  0,  0,  0);
  fprintf(f, "%c%c%c%c%c%c",  0,  0,  0,  0,  0,  0);
  fprintf(f, "%c%c%c%c%c%c", (width_ * super_sample_) % 256, (width_ * super_sample_) / 256, 
                             (height_ * super_sample_) % 256, (height_ * super_sample_) / 256, 
                             24, 32);
  BYTE* scanline = new BYTE[width_ * super_sample_ * 3];

  for (int i=0; i<super_sample_; i++)
  {
    // Copy backbuffer to CPU-side memory.
    D3D11_BOX sourceRegion;
    sourceRegion.left   = 0;
    sourceRegion.right  = width_ * super_sample_;
    sourceRegion.top    = (i  )*h;
    sourceRegion.bottom = (i+1)*h;
    sourceRegion.front  = 0;
    sourceRegion.back   = 1;
    immed_context_->CopySubresourceRegion(temp_tex, 0, 0, 0, 0, final_color_.tex, 0, &sourceRegion);

    //TODO: lock the texture
    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    hr = immed_context_->Map(temp_tex, 0, D3D11_MAP_READ, 0, &mapped_resource);
    if (FAILED(hr)) {
      MessageBoxA(hwnd_, "immed_context_->Map() failed.", "ERROR", MB_ICONERROR); 
      fclose(f);
      delete scanline;
      SAFE_RELEASE(temp_tex);
      //SAFE_RELEASE(pBackBuffer);
      return false;
    }

    // Write to file.
    DWORD* src_buf = (DWORD*)mapped_resource.pData;
    for (int y=0; y<h; y++)
    {
      DWORD* src  = &src_buf[ y*(mapped_resource.RowPitch/4) ];    //TGA writes upside-down image
      BYTE*  dest = scanline;
      for (int x=0; x < width_ * super_sample_; x++)
      {
        DWORD col = *src++;        //TGA writes B, G, R.
        *dest++ = (BYTE)((col>>16) & 0xFF);
        *dest++ = (BYTE)((col>> 8) & 0xFF);
        *dest++ = (BYTE)((col    ) & 0xFF);
      }
      fwrite(scanline, width_ * super_sample_ * 3, 1, f);
    }
    
    // Unlock the texture.
    immed_context_->Unmap(temp_tex, 0);
  }

  fclose(f);
  delete scanline;
  SAFE_RELEASE(temp_tex);
  //SAFE_RELEASE(pBackBuffer);

  // Export the current scene (camera, lights, etc.) to a file. 
  TCHAR scene_file[2048];
  swprintf_s(scene_file, sizeof(scene_file)/sizeof(scene_file[0]), 
             L"%sbin\\_screenshot_.scn", 
             parent_dir_);
  SetCurrentDirectory(parent_dir_);
  bool bSuccess = Export(scene_file);

  // Finally, create a .zip with the .exe, shaders, code, scene file, etc.
  char cmdline[2048];
  sprintf_s(cmdline, sizeof(cmdline)/sizeof(cmdline[0]), 
    "bin\\rar.exe a screenshots\\%S.zip bin\\gpucaster*.exe bin\\_screenshot_.scn shaders\\*.* code\\*.*",
    base_filename);
  SetCurrentDirectory(parent_dir_);
  system(cmdline);  

  SetFocus(hwnd_);

  return true;
}

/*
bool Demo::SaveImageToDiskCompressed()
{
  TCHAR szDir[1024];
  swprintf_s(szDir, sizeof(szDir)/sizeof(szDir[0]), L"%s%s", parent_dir_, L"screenshots");
  CreateDirectory(szDir, NULL);

  TCHAR base_filename[512];
  swprintf_s(base_filename, sizeof(base_filename)/sizeof(base_filename[0]), L"img_%08x__%08x_%04d", cb_data_.g_scene_seed, time(0), cb_data_.g_frame);
  TCHAR filename[512];
  swprintf_s(filename, sizeof(filename)/sizeof(filename[0]), L"%s\\%s", szDir, base_filename);
  wcscat_s(filename, sizeof(filename)/sizeof(filename[0]), L".tga");

  ID3D11Texture2D* pBackBuffer = NULL;
  HRESULT hr = swap_chain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
  if (FAILED(hr)) {
    MessageBoxA(hwnd_, "SwapChain::GetBuffer() failed.", "Error", MB_ICONERROR); 
    return false;
  }

  // Create a temporary CPU-side texture that is a thin horizontal slice.
  int h = height_ / super_sample_;
  ID3D11Texture2D* temp_tex = NULL;
  D3D11_TEXTURE2D_DESC td;
  td.Width  = width_;
  td.Height = h;
  td.Format = DXGI_FORMAT_R8G8B8A8_UINT;
  td.MipLevels = 1;
  td.ArraySize = 1;
  td.SampleDesc.Count = 1;
  td.SampleDesc.Quality = 0;
  td.Usage = D3D11_USAGE_STAGING;
  td.BindFlags = 0;
  td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  td.MiscFlags = 0;
  device_->CreateTexture2D(&td, NULL, &temp_tex);

  // Open TGA file and write header.
  FILE* f = NULL;
  _wfopen_s(&f, filename, L"wb");
  if (!f) {
    MessageBox(hwnd_, filename, L"Unable to open file for output:", MB_ICONERROR); 
    SAFE_RELEASE(temp_tex);
    return false;
  }
  fprintf(f, "%c%c%c%c%c%c",  0,  0, 10,  0,  0,  0);
  fprintf(f, "%c%c%c%c%c%c",  0,  0,  0,  0,  0,  0);
  fprintf(f, "%c%c%c%c%c%c", width_ % 256, width_/256, height_ % 256, height_/256, 24, 32);
  BYTE* scanline = new BYTE[width_*4];

  for (int i=0; i<super_sample_; i++)
  {
    // Copy backbuffer to CPU-side memory.
    D3D11_BOX sourceRegion;
    sourceRegion.left   = 0;
    sourceRegion.right  = width_;
    sourceRegion.top    = (i  )*h;
    sourceRegion.bottom = (i+1)*h;
    sourceRegion.front  = 0;
    sourceRegion.back   = 1;
    immed_context_->CopySubresourceRegion(temp_tex, 0, 0, 0, 0, pBackBuffer, 0, &sourceRegion);

    //TODO: lock the texture
    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    hr = immed_context_->Map(temp_tex, 0, D3D11_MAP_READ, 0, &mapped_resource);
    if (FAILED(hr)) {
      MessageBoxA(hwnd_, "immed_context_->Map() failed.", "ERROR", MB_ICONERROR); 
      fclose(f);
      delete scanline;
      SAFE_RELEASE(temp_tex);
      return false;
    }

    // Write to file.
    DWORD* src_buf = (DWORD*)mapped_resource.pData;
    for (int y=0; y<h; y++)
    {
      DWORD* src  = &src_buf[ y*(mapped_resource.RowPitch/4) ];
      BYTE*  dest = scanline;
      int bytes_written = 0;
      for (int x=0; x<width_; x++)
      {
        // Analyze the data, and determine the best packet type & size.
        bool compress = false;
        int  pixels   = 1;
        if (x < width_ - 1) 
        {
          DWORD col0 = src[x+0] & 0xFFFFFF;
          DWORD col1 = src[x+1] & 0xFFFFFF;
          if (col0 == col1) 
          {
            // See how many pixels we can get to match.
            pixels = 2;
            int j = x + 2;
            while (j < width_ && pixels < 127 && (src[j] & 0xFFFFFF) == col0) {
              j++;
              pixels++;
            }
          }
          else 
          {
            // See how many pixels we can go until we FIND a match.
            pixels = 2;
            int j = x + 2;
            while (j < width_ && pixels < 127 && (src[j] & 0xFFFFFF) != (src[j-1] & 0xFFFFFF)) {
              j++;
              pixels++;
            }
          }
        }

        *dest++ = (compress ? 0x80 : 0x00) | (pixels - 1);
        bytes_written++;

        if (compress)   // Compress!
        { 
          DWORD col = src[x];
          *dest++ = (BYTE)((col    ) & 0xFF);
          *dest++ = (BYTE)((col>> 8) & 0xFF);
          *dest++ = (BYTE)((col>>16) & 0xFF);
          bytes_written += 3;
        }
        else    // No compression.
        {
          for (int i=0; i<pixels; i++)
          {
            DWORD col = src[x+i];
            *dest++ = (BYTE)((col    ) & 0xFF);
            *dest++ = (BYTE)((col>> 8) & 0xFF);
            *dest++ = (BYTE)((col>>16) & 0xFF);
          }
          bytes_written += pixels * 3;
        }
        x += pixels;
      }
      fwrite(scanline, bytes_written, 1, f);
    }
    
    // Unlock the texture.
    immed_context_->Unmap(temp_tex, 0);
  }

  fclose(f);
  delete scanline;
  SAFE_RELEASE(temp_tex);

  // Export the current scene (camera, lights, etc.) to a file. 
  TCHAR scene_file[2048];
  swprintf_s(scene_file, sizeof(scene_file)/sizeof(scene_file[0]), 
             L"%sbin\\_screenshot_.scn", 
             parent_dir_);
  SetCurrentDirectory(parent_dir_);
  bool bSuccess = Export(scene_file);

  // Finally, create a .zip with the .exe, shaders, code, scene file, etc.
  char cmdline[2048];
  sprintf_s(cmdline, sizeof(cmdline)/sizeof(cmdline[0]), 
    "bin\\rar.exe a screenshots\\%S.zip bin\\gpucaster*.exe bin\\_screenshot_.scn shaders\\*.* code\\*.*",
    base_filename);
  SetCurrentDirectory(parent_dir_);
  system(cmdline);  

  return true;
}
*/

}  // namespace gpucaster
