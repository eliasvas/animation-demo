 #include "platform.h"
#include "tools.h"
#include "quad.h" 
#include "camera.h"
#include "objloader.h"
#include "model.h"
#include "entity.h"
#include "text.h"
#include "collada_parser.h"
#include "animation.h"
#include "skybox.h"
/*          TODO 
 *  -Figure out what kind of game I wanna make!!!!
 *  -3D animations (collada)
 *  -Make a dedicated renderer!!!!
 *  -IMGUI layer? (microui based)
 *  -Fix Entities
 *  -Collision manager!
 *  -Scene Graph
 *  -----------------------------
 *  -Make good strings!!
*/

static Quad q;
static Model m;
static MeshInfo mesh;
static Camera cam;
static BitmapFont bmf;
static Skybox skybox;

static mat4 view;
static mat4 proj;
static vec4 background_color;


static MeshData sat_data;
static AnimatedModel animated_sat;
static Animation animation_to_play;
static Animator animator;
static Shader anim_shader;
static void 
init(void)
{
    init_quad(&q, "../assets/dirt.png");
    init_fullscreen_quad(&screen_quad, "../assets/red.png");
    init_camera(&cam);
    {
        mesh = load_obj("../assets/bunny/stanford_bunny.obj");
        //mesh = load_obj("../assets/utah_teapot.obj");
        init_model_textured_basic(&m, &mesh);
        load_texture(&(m.diff),"../assets/bunny/stanford_bunny.jpg");
    }
    m.position = v3(0,0,-2);
    init_text(&bmf, "../assets/BMF.png");
    char *skybox_faces[6] = {"../assets/nebula/neb_rt.tga", "../assets/nebula/neb_lf.tga", "../assets/nebula/neb_up.tga",
        "../assets/nebula/neb_dn.tga", "../assets/nebula/neb_bk.tga", "../assets/nebula/neb_ft.tga" };
    init_skybox(&skybox, skybox_faces);

 
    //Animated Model initialization
    {
        //BLYYEHEHEHEH delete dis shiet luuul
        Texture *anim_tex = malloc(sizeof(Texture));
        load_texture(anim_tex,"../assets/boy_11.png");
        sat_data = read_collada_maya(str(&global_platform.permanent_storage,"../assets/boy_maya_raw.dae"));
        animated_sat = init_animated_model(anim_tex, sat_data.root,&sat_data);
        shader_load(&anim_shader,"../assets/shaders/animated3d.vert", "../assets/shaders/animated3d.frag");
        animation_to_play = read_collada_animation(str(&global_platform.permanent_storage,"../assets/boy_maya_raw.dae"));
        animator = (Animator){animated_sat, &animation_to_play, 1.05f};
    }
}



static void 
update(void) {
    update_cam(&cam);
    view = get_view_mat(&cam);
    proj = perspective_proj(45.f,global_platform.window_width / (f32)global_platform.window_height, 0.1f,100.f); 
}


static void 
render(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(background_color.x, background_color.y, background_color.z,background_color.w);
    //render_skybox(&skybox);

    update_animator(&animator);
    render_animated_model(&animator.model, &anim_shader, proj, view);

    //render_model_textured_basic(&m,&proj, &view);
    //render_quad_mvp(&q, mul_mat4(proj,view));


    if (global_platform.key_down[KEY_TAB])
        print_debug_info(&bmf);
    if (global_platform.key_pressed[KEY_P])
        write_texture2D_to_disk(&q.texture.id);
}

