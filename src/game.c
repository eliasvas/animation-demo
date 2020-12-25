#include "platform.h"
#include "tools.h"
#include "quad.h" 
#include "camera.h"
#include "objloader.h"
#include "model.h"
#include "entity.h"
#include "text.h"
#include "collada_parser.h"
#include "skybox.h"
#include "physics.h"

static Quad q;
static Camera cam;
static BitmapFont bmf;
static Skybox skybox;

static mat4 view;
static mat4 proj;
static vec4 background_color;
static i32 state;

static Animator animator;
static void 
init(void)
{
    init_fullscreen_quad(&screen_quad, "../assets/red.png");
    init_camera(&cam);
    init_text(&bmf, "../assets/BMF.png");
    char *skybox_faces[6] = {"../assets/env/maplenight_rt.tga", "../assets/env/maplenight_lf.tga", "../assets/env/maplenight_dn.tga",
        "../assets/env/maplenight_up.tga", "../assets/env/maplenight_bk.tga", "../assets/env/maplenight_ft.tga" };
    init_skybox(&skybox, skybox_faces);

 
    animator = init_animator(str(&global_platform.frame_storage,"../assets/redead.png"), 
            str(&global_platform.frame_storage,"../assets/rumba-redead.dae"), str(&global_platform.frame_storage,"../assets/jazz-redead.dae"));  
    background_color = v4(0.4,0.7,0.7,1.f);
    state = 1;
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
    if (global_platform.key_pressed[KEY_DOWN] && state != 0)
    {
        *animator.anim = read_collada_animation(str(&global_platform.permanent_storage,"../assets/rumba-redead.dae"));
        animator.blend_percentage = 1.f;
        animator.blend_time = 0.2f;
        state = 0;
    }
    if (global_platform.key_pressed[KEY_UP] && state != 1)
    {
        //JointKeyFrame *prev_pose = animator.prev_pose;
        *animator.anim = read_collada_animation(str(&global_platform.permanent_storage,"../assets/jazz-redead.dae"));
        animator.blend_percentage = 1.f;
        animator.blend_time = 0.2f;
        state = 1;
    }
    if (global_platform.key_pressed[KEY_LEFT] && state != 2)
    {
        *animator.anim = read_collada_animation(str(&global_platform.permanent_storage,"../assets/wave-left-redead.dae"));
        animator.blend_percentage = 1.f;
        animator.blend_time = 0.2f;
        state = 2;
    }
    if (global_platform.key_pressed[KEY_RIGHT] && state != 3)
    {
        *animator.anim = read_collada_animation(str(&global_platform.permanent_storage,"../assets/block-right-redead.dae"));
        animator.blend_percentage = 1.f;
        animator.blend_time = 0.2f;
        state = 3;
    }



    render_skybox(&skybox);

    update_animator(&animator);
    render_animated_model(&animator.model, &anim_shader, proj, view);
    render_animated_model_static(&animator.model, &anim_shader, proj, view, v3(15,0,0), 1.f);
    render_animated_model_static(&animator.model, &anim_shader, proj, view, v3(-15,0,0), -1.f);

    if (global_platform.key_down[KEY_TAB])
        print_debug_info(&bmf);

}

