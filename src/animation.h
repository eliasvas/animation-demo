#ifndef ANIMATION_H
#define ANIMATION_H
#include "tools.h"
#include "platform.h"
#include "shader.h"
#include "texture.h"


typedef struct AnimatedModel
{
    //skin
    GLuint vao;
    mat4 * transforms;
    Texture * diff_tex;
    //Texture * spec_tex;
    
    //skeleton
    Joint root;
    u32 joint_count;
    
}AnimatedModel;


typedef struct Animator
{
    AnimatedModel model;
    Animation* anim;
    f32 animation_time;
}Animator;


static void
calc_inv_bind_transform(Joint* joint, mat4 parent_bind_transform) //needs to be called only on the root joint of each model
{
    mat4 bind_transform = mul_mat4(joint->local_bind_transform, parent_bind_transform); //transform in relation to origin
    mat4 inv_bind_transform = inv_mat4(bind_transform);
    joint->inv_bind_transform = inv_bind_transform;
    //for (Joint& j : joint->children)
        //calc_inv_bind_transform(&j, bind_transform);
    for (i32 i = 0; i < joint->num_of_children; ++i)
        calc_inv_bind_transform(&joint->children[i], bind_transform);
}

static void
put_inv_bind_transforms_from_array(Joint* joint, mat4* transforms) //needs to be called only on the root joint of each model
{
    transforms[joint->index] = joint->animated_transform;//mul_mat4(joint->inv_bind_transform,transforms[joint->index]);
    //for (Joint& j : joint->children)
        //put_inv_bind_transforms_in_array(&j, transforms);
    for (i32 i = 0; i < joint->num_of_children; ++i)
       put_inv_bind_transforms_from_array(&joint->children[i], transforms);
}

static void
initialize_joint_pos_array(Joint* joint, mat4* transforms)
{
    transforms[joint->index] = m4d(1.f);
    //for(Joint& j : joint->children)
        //initialize_joint_pos_array(&j, transforms);
for (i32 i = 0; i < joint->num_of_children; ++i)
       initialize_joint_pos_array(&joint->children[i], transforms);

}




static JointTransform 
joint_transform(vec3 position, Quaternion rotation)
{
    JointTransform res;
    res.position = position;
    res.rotation = rotation;
    return res;
}

static mat4
get_joint_transform_matrix(JointTransform j)
{
    mat4 res;
    res = mul_mat4(translate_mat4(j.position), quat_to_mat4(j.rotation));
    return res;
}

static JointTransform
interpolate_joint_transforms(JointTransform l, JointTransform r, f32 time)
{
    JointTransform res = {0};
    vec3 pos = lerp_vec3(l.position,r.position, time);
    Quaternion q = nlerp(l.rotation, r.rotation, time); //maybe we need slerp??
    res.position = pos;
    res.rotation = q;
    return res;
}
static void 
increase_animation_time(Animator* anim)
{
    anim->animation_time += global_platform.dt; //this should be the Δt from global platform but its bugged rn
    if (anim->animation_time > anim->anim->length)
        anim->animation_time -= anim->anim->length;
}



static mat4 
calc_pose_of_joints(Animator* anim,JointKeyFrame current_pose, Joint *j, mat4 parent_transform)
{
    JointTransform local_joint_transform = current_pose.transform;
    
    //the local bone space transform of joint j
    mat4 current_local_transform = mul_mat4(translate_mat4(local_joint_transform.position), quat_to_mat4(local_joint_transform.rotation));
    
    //the world position of our joint j
    mat4 current_transform = mul_mat4(parent_transform, current_local_transform);//why parent transform first??

    for (i32 i = 0; i < j->num_of_children; ++i)
        calc_pose_of_joints(anim, current_pose, &j->children[i], current_transform);

    
    //the transform to go from the original joint pos to the desired in world space 
    current_transform = mul_mat4(current_transform, j->inv_bind_transform);
    j->animated_transform = current_transform;
    return current_transform;
}

/*
static void 
calc_pose_of_joints(Animator* anim,mat4 * transforms,JointKeyFrame current_pose, Joint *j, mat4 parent_transform)
{
    JointTransform local_joint_transform = current_pose.transform;
    
    //the local bone space transform of joint j
    mat4 current_local_transform = mul_mat4(translate_mat4(local_joint_transform.position), quat_to_mat4(local_joint_transform.rotation));
    
    //the world position of our joint j
    mat4 current_transform = mul_mat4(parent_transform, current_local_transform);//why parent transform first??
    for(Joint child_joint : j->children)
        calc_pose_of_joints(anim,transforms, current_pose, &child_joint, current_transform);
    
    //the transform to go from the original joint pos to the desired in world space 
    //current_transform = mul_mat4(current_transform, j.inv_bind_transform);
    //j.animated_transform = current_transform;
    transforms[j->index] = current_transform;
}
*/

static void 
apply_pose_to_joints(Animator *animator, Joint *j, mat4 *transforms)
{
    i32 index = j->index;
    //the transform to go from the original joint pos to the desired in world space 
    mat4 current_transform = mul_mat4(transforms[index], j->inv_bind_transform);
    j->animated_transform = current_transform;
    //for (Joint& child : j->children)
        //apply_pose_to_joints(animator, &child, transforms); 
    for (i32 i = 0; i < j->num_of_children; ++i)
        apply_pose_to_joints(animator, &j->children[i], transforms);
}
static void 
concat_local_joint_transforms(Animator *animator, Joint *j, mat4 *local_transforms, mat4 parent_transform)
{
    i32 index = j->index;
    //we calculate the animated transform
    mat4 current_transform = mul_mat4(local_transforms[index], parent_transform);
    //for (Joint& child : j->children)
        //apply_pose_to_joints(animator, &child, transforms); 
    for (i32 i = 0; i < j->num_of_children; ++i)
        concat_local_joint_transforms(animator, &j->children[i], local_transforms, current_transform);
    j->animated_transform = mul_mat4(current_transform, j->inv_bind_transform);
}



static u32 
count_joints(Joint* j)
{
    assert(j);
    u32 sum = 1;
    //for (Joint& child : j->children)
        //sum += count_joints(&child); 
    for (i32 i = 0; i < j->num_of_children; ++i)
        sum += count_joints(&j->children[i]);
    return sum;
}



char joint_transforms[21] = "joint_transforms[00]";
char joint_transforms_one[20] = "joint_transforms[0]";
static void 
set_joint_transform_uniforms(AnimatedModel* model,Shader* s, Joint *j)
{
    if (j->index >= 10){
        joint_transforms[17] = '0' + (j->index/10);
        joint_transforms[18] = '0' + (j->index -(((int)(j->index/10)) * 10));
        setMat4fv(s, joint_transforms, (f32*)j->animated_transform.elements);
    }else
    {
        joint_transforms_one[17] = '0' + (j->index);
        setMat4fv(s, joint_transforms_one, (f32*)j->animated_transform.elements);
    }

    for (i32 i = 0; i < j->num_of_children; ++i)
        set_joint_transform_uniforms(model, s, &j->children[i]);
}




static JointKeyFrame* 
get_previous_and_next_keyframes(Animator* animator, i32 joint_animation_index)
{
    JointKeyFrame frames[2];
    JointKeyFrame* all_frames = animator->anim->joint_animations[joint_animation_index].keyframes;
    JointKeyFrame prev = all_frames[0];
    JointKeyFrame next = all_frames[0];
    for (i32 i = 1; i < animator->anim->joint_animations[joint_animation_index].keyframe_count; ++i)
    {
        next = all_frames[i];
        if (next.timestamp > animator->animation_time)
            break;
        prev = all_frames[i];
    }
    frames[0] = prev;
    frames[1] = next;
    return (frames);
}

static f32 calc_progress(Animator* animator, JointKeyFrame prev, JointKeyFrame next)
{
    f32 total_time = next.timestamp - prev.timestamp;
    f32 current_time = animator->animation_time - prev.timestamp;
    return current_time / total_time;
}

JointKeyFrame interpolate_poses(JointKeyFrame prev, JointKeyFrame next, f32 x)
{
    JointKeyFrame res;
    
    res.transform.position = lerp_vec3(prev.transform.position, next.transform.position, x);
    res.transform.rotation = nlerp(prev.transform.rotation, next.transform.rotation, x);
    res.joint_index = prev.joint_index;
    
    return res;
    
}

static JointKeyFrame calc_current_animation_pose(Animator* animator, u32 joint_animation_index)
{
    JointKeyFrame* frames = get_previous_and_next_keyframes(animator, joint_animation_index);
    f32 x = calc_progress(animator, frames[0],frames[1]);
    return interpolate_poses(frames[0],frames[1], x); //this has to be done!!!!!
}

//this is the meat and potatoes of the whole animator
static void
update_animator(Animator* animator)
{
    return;
    if (animator->anim == NULL)return;
    increase_animation_time(animator);
    //this is the array holding the animated local bind transforms for each joint,
    //if there is no animation in a certain joint its simply m4d(1.f)
    mat4 *local_animated_transforms= (mat4*)arena_alloc(&global_platform.frame_storage, sizeof(mat4) *44);
    for (i32 i = 0; i < 44; ++i)
    {
        local_animated_transforms[i] = m4d(1.f);
    }

    //we put the INTERPOLATED local(wrt parent) animated transforms in the array
    for (u32 i = 0; i < animator->anim->joint_anims_count; ++i)
    {
        JointKeyFrame current_pose = calc_current_animation_pose(animator, i); 
        mat4 local_animated_transform = mul_mat4(translate_mat4(current_pose.transform.position), quat_to_mat4(current_pose.transform.rotation));
        local_animated_transforms[current_pose.joint_index] = local_animated_transform;
    }

    //now we recursively apply the pose to get the animated bind(wrt world) transform
    concat_local_joint_transforms(animator, &animator->model.root, local_animated_transforms,m4d(1.f));
}



static GLuint 
create_animated_model_vao(MeshData* data)
{
    GLuint vao;
    GLuint ebo;
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    //glGenBuffers(1,&ebo);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); 
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(data->indices_size), &data->indices, GL_STATIC_DRAW);
    
    //positions
    glEnableVertexAttribArray(0);
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(AnimatedModel) * data->vertex_count, &data->vertices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11 + sizeof(int) * 3, (void*)(0));
    //normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11 + sizeof(int) * 3, (void*) (sizeof(float) * 3));
    //tex_coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 11 + sizeof(int) * 3, (void*) (sizeof(float) * 6));
    //joints (max 3)
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 3, GL_INT,sizeof(float) * 11 + sizeof(int) * 3, (void*) (sizeof(float) * 8));
    //joint weights (max 3)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 11 + sizeof(int) * 3, (void*) (sizeof(float) * 8 + sizeof(int) * 3));
    
    glBindVertexArray(0);
    
    
    return vao;
}

//this is so bad
static void 
render_animated_model(AnimatedModel* model, Shader* s, mat4 proj, mat4 view)
{
    use_shader(s);
    
    setMat4fv(s, "projection_matrix", (GLfloat*)proj.elements);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,model->diff_tex->id);
    setInt(s, "diffuse_map", 1); //we should really make the texture manager global or something(per Scene?)... sigh
    //for(i32 i = 0; i < model->joint_count; ++i)
    //to start things off, everything is identity!
    {
        mat4 identity = m4d(1.f);
        setMat4fv(s, "joint_transforms[0]", (GLfloat*)identity.elements);
        setMat4fv(s, "joint_transforms[1]", (GLfloat*)identity.elements);
        setMat4fv(s, "joint_transforms[2]", (GLfloat*)identity.elements);
        setMat4fv(s, "joint_transforms[3]", (GLfloat*)identity.elements);
        setMat4fv(s, "joint_transforms[4]", (GLfloat*)identity.elements);
        setMat4fv(s, "joint_transforms[5]", (GLfloat*)identity.elements);
        setMat4fv(s, "joint_transforms[6]", (GLfloat*)identity.elements);
        setMat4fv(s, "joint_transforms[7]", (GLfloat*)identity.elements);
        setMat4fv(s, "joint_transforms[8]", (GLfloat*)identity.elements);
        setMat4fv(s, "joint_transforms[9]", (GLfloat*)identity.elements);
        //@memleak
        char *str = "joint_transforms[xx]";

        for (i32 i = 10; i < 44; ++i)
        {
            str[17] = '0' + (i/10);
            str[18] = '0' + (i -(((int)(i/10)) * 10));
            setMat4fv(s, str, (GLfloat*)identity.elements);
        }
    }
    //TODO some matrices get loaded as [-nan], investigate and @fix
    //set_joint_transform_uniforms(model,s, &(model->root));
    setMat4fv(s, "view_matrix", (GLfloat*)view.elements);
    glUniform3f(glGetUniformLocation(s->ID, "light_direction"), 0.43,0.34,0.f); 
    //glUniform3f(glGetUniformLocation(s->ID, "light_direction"), 1.f,0.0f,0.0f); 
    //no need to set diffuse map .. whatever we get
    
    
    glBindVertexArray(model->vao);
    glDrawArrays(GL_TRIANGLES,0, 20000);
    //glDrawArrays(GL_LINES,0, 20000);
    glBindVertexArray(0);
    
}

static AnimatedModel
init_animated_model(Texture* diff, Joint root,MeshData* data)
{
    AnimatedModel model = {0};
    
    model.vao = create_animated_model_vao(data);
    model.diff_tex = diff;
    model.root = root;
    model.transforms = data->transforms;
    model.joint_count = count_joints(&root);
    //calc_inv_bind_transform(&model.root,m4d(1.f));
    
    //not sure about this one -- let's figure out the parser first
    data->transforms = arena_alloc(&global_platform.permanent_storage, sizeof(mat4) * model.root.num_of_children);
    //initialize_joint_pos_array(&model.root,data->transforms);
    return model;
}

/*
typedef struct Joint
{
    u32 index;
    String name;
    String sid;
    std::vector<Joint> children;
    Joint *parent;
    //Joint* children;
    //u32 num_of_children;
    mat4 animated_transform; //joint transform
    mat4 local_bind_transform;
    mat4 inv_bind_transform;
} Joint;


typedef struct vertex
{
   vec3 position; 
   vec3 normal;
   vec2 tex_coord;
}vertex;

static vertex vert(vec3 p, vec3 n, vec2 t)
{
    vertex res;
    res.position = p;
    res.normal = n;
    res.tex_coord = t;
    return res;
}

typedef struct AnimatedVertex
{
    vec3 position;
    vec3 normal;
    vec2 tex_coord;
    ivec3 joint_ids;
    vec3 weights;
}AnimatedVertex;

typedef struct MeshData{
    vec3* positions; 
    vec3* normals; 
    vec2* tex_coords; 
    vertex* verts; //just for rendering
    i32 vertex_count;
    i32* joint_ids; 
    vec3* weights; 
    u32 size;

    mat4* transforms;
    i32 transforms_count;

    Joint root;
    AnimatedVertex* vertices;

}MeshData;
*/




#endif

