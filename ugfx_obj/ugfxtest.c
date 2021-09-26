#include <stdlib.h>
#include <libdragon.h>
#include "mesh.h"
#include "fast_obj_dfs.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <string.h>

#define RADIANS(deg) ((deg) * (M_PI/180.0f))

//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#include <stdio.h>
#define DBG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DBG(...)
#endif

static ugfx_light_t lights[] = {
    {
        .r = 40,
        .g = 20,
        .b = 30,
    },
    {
        .r = 0xFF,
        .g = 0xFF,
        .b = 0xFF,
        .x = 0,
        .y = 0,
        .z = 127
    }
};

void perspective(float fovy,
                 float aspect,
                 float nearZ,
                 float farZ,
                 float dest[4][4]) {
  float f, fn;

  memset(&dest[0], 0, sizeof(float) * 16);

  f  = 1.0f / tanf(fovy * 0.5f);
  fn = 1.0f / (nearZ - farZ);

  dest[0][0] = f / aspect;
  dest[1][1] = -f;
  dest[2][2] = (nearZ + farZ) * fn;
  dest[2][3] =-1.0f;
  dest[3][2] = 2.0f * nearZ * farZ * fn;
}

/*
 * This is a *bad* and greedy implementation but it works.
 *
 * Allocates and creates a ugfx_vertex_t array with vertices needed to
 * draw the fastObjMesh `mesh_in`. This may include redundant copies of
 * vertices. The order is determined by the order of the faces in the
 * obj file.
 *
 * Creates commands that will load and draw triangles using at most 32
 * vertices at a time.
 */
void load_mesh_vertices(const fastObjMesh *mesh_in,
                        ugfx_vertex_t **mesh_vtx_out, uint32_t *mesh_vtx_len,
                        ugfx_command_t **mesh_cmd_out, uint32_t *mesh_cmd_len)
{
    uint32_t vtx_idx = 0;
    uint32_t cmd_idx = 0;
    uint32_t vtx_count = 0;
    uint32_t cmd_count = 0;
    uint32_t face_count = 0;

    // Count total number of triangles needed to draw all faces

    for (unsigned int ii = 0; ii < mesh_in->group_count; ii++) {
        const fastObjGroup *grp = &mesh_in->groups[ii];

        //  For each face in group
        for (unsigned int jj = 0; jj < grp->face_count; jj++) {
            unsigned int fv = mesh_in->face_vertices[grp->face_offset + jj];

            // Number of triangles required for this face
            if (fv > 2) {
                face_count += fv - 2;
                vtx_count += 3 * (fv - 2);
            }
        }
    }

    cmd_count  = face_count + vtx_count / 30; // Add one ugfx_load_vertices() per group of 30
    cmd_count += 2;                          // ugfx_finalize()

    fprintf(stderr, "vtx_count=%ld\n", vtx_count);
    fprintf(stderr, "cmd_count=%ld\n", cmd_count);
    fprintf(stderr, "face_count=%ld\n", face_count);

    ugfx_vertex_t *mesh_vtx = (ugfx_vertex_t *) malloc(vtx_count * sizeof(ugfx_vertex_t));
    ugfx_command_t *mesh_cmd = (ugfx_command_t *) malloc(cmd_count * sizeof(ugfx_command_t));

    uint32_t vtx_space_left = 0;
    uint32_t vtx_left = vtx_count;
    uint32_t vtx_idx_start = 0;

    // For each group in mesh
    for (unsigned int ii = 0; ii < mesh_in->group_count; ii++) {
        const fastObjGroup *grp = &mesh_in->groups[ii];
        uint32_t idx = 0;

        //  For each face in group
        for (unsigned int jj = 0; jj < grp->face_count; jj++) {
            unsigned int fv = mesh_in->face_vertices[grp->face_offset + jj];

            if (vtx_space_left < fv) {
                vtx_space_left = vtx_left < 32 ? vtx_left : 32; // Load at most 32 vertices
                DBG("+C[%3ld] ugfx_load_vertices(%d, %ld, %d, %ld)\n", cmd_idx, 1, vtx_idx * sizeof(ugfx_vertex_t), 0, vtx_space_left);
                mesh_cmd[cmd_idx++] = (ugfx_command_t) ugfx_load_vertices(1, vtx_idx * sizeof(ugfx_vertex_t), 0, vtx_space_left);
                vtx_idx_start = vtx_idx;
            }

            vtx_left -= fv;
            vtx_space_left -= fv;

            uint32_t vtx_idx_start_local = vtx_idx - vtx_idx_start;

            for (unsigned int kk = 0; kk < fv; kk++) {
                float vx = 0.0f;
                float vy = 0.0f;
                float vz = 0.0f;
                float vs = 0.0f;
                float vt = 0.0f;
                int8_t  nx = 0.0f;
                int8_t  ny = 0.0f;
                int8_t  nz = 0.0f;
                uint8_t va = 0;

                fastObjIndex *mi = &mesh_in->indices[grp->index_offset + idx];
                idx++;

                if (mi->p) {
                    vx = mesh_in->positions[3 * mi->p + 0];
                    vy = mesh_in->positions[3 * mi->p + 1];
                    vz = mesh_in->positions[3 * mi->p + 2];
                }

                if (mi->t) {
                    vs = mesh_in->positions[2 * mi->t + 0];
                    vt = mesh_in->positions[2 * mi->t + 1];
                    // TODO: what is va?
                    // va = ;
                }

                if (mi->n) {
                    nx = float_to_fixed(mesh_in->positions[3 * mi->n + 0], 6);
                    ny = float_to_fixed(mesh_in->positions[3 * mi->n + 1], 6);
                    nz = float_to_fixed(mesh_in->positions[3 * mi->n + 2], 6);
                }

                DBG(" V[%3ld] make_vertex_n(%+02.2f, %+02.2f, %+02.2f, %+02.2f, %+02.2f, %4d, %4d, %4d, %d)\n",
                        vtx_idx, vx, vy, vz, vs, vt, nx, ny, nz, va);
                mesh_vtx[vtx_idx++] = (ugfx_vertex_t) make_vertex_n(vx, vy, vz, vs, vt, nx, ny, nz, va);
            }

            for (unsigned int kk = 1; kk < fv - 1; kk++) {
                uint8_t v1 = vtx_idx_start_local;
                uint8_t v2 = vtx_idx_start_local + kk;
                uint8_t v3 = vtx_idx_start_local + kk + 1;
                DBG(" C[%3ld] ugfx_draw_triangle(%d, %d, %d)\n", cmd_idx, v1, v2, v3);
                mesh_cmd[cmd_idx++] = (ugfx_command_t) ugfx_draw_triangle(v1, v2, v3);
            }
        }
    }

    DBG(" C[%3ld] ugfx_finalize()\n", cmd_idx);
    mesh_cmd[cmd_idx++] = (ugfx_command_t) ugfx_finalize();

    *mesh_vtx_out = mesh_vtx;
    *mesh_cmd_out = mesh_cmd;
    *mesh_vtx_len = vtx_idx;
    *mesh_cmd_len = cmd_idx;

    fprintf(stderr, " cmd_idx: %ld\n", cmd_idx);
}

int main(void)
{
    /* enable interrupts (on the CPU) */
    init_interrupts();

    debug_init_isviewer();

    /* Initialize peripherals */
    // display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);
    display_init(RESOLUTION_640x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);
    ugfx_init(UGFX_DEFAULT_RDP_BUFFER_SIZE);
    dfs_init(DFS_DEFAULT_LOCATION);

    static fastObjMesh *mesh;
    static int mesh_count;
    static fastObjMesh *meshes[256];
    static char *mesh_names[256];
    char path[256];
    int flags = dfs_dir_findfirst("/", path);

    while (flags == FLAGS_FILE) {
        if (strstr(path, ".obj")) {
            fprintf(stderr, "Loading %d - %s\n", mesh_count, path);
            mesh_names[mesh_count] = strdup(path);
            fprintf(stderr, " 1\n");
            meshes[mesh_count] = fast_obj_read_with_callbacks(path, &fast_obj_dfs_cb, NULL);
            fprintf(stderr, " position_count: %d\n", meshes[mesh_count]->position_count);
            fprintf(stderr, " texcoord_count: %d\n", meshes[mesh_count]->texcoord_count);
            fprintf(stderr, " normal_count:   %d\n", meshes[mesh_count]->normal_count);
            fprintf(stderr, " face_count:     %d\n", meshes[mesh_count]->face_count);
            fprintf(stderr, " material_count: %d\n", meshes[mesh_count]->material_count);
            fprintf(stderr, " group_count:    %d\n", meshes[mesh_count]->group_count);
            mesh_count++;
        }
        flags = dfs_dir_findnext(path);
        fprintf(stderr, "flags=%08X\n", flags);
    }

    mesh = meshes[3]; // hardcode for now

    ugfx_vertex_t *mesh_vtx;
    ugfx_command_t *mesh_cmd;
    uint32_t mesh_cmd_len;
    uint32_t mesh_vtx_len;

    fprintf(stderr, "Loading mesh %s\n", mesh_names[0]);
    load_mesh_vertices(mesh, &mesh_vtx, &mesh_vtx_len, &mesh_cmd, &mesh_cmd_len);
    fprintf(stderr, "Loaded %s\n", mesh_names[0]);

    /* Load texture file */
    // int fp = dfs_open("/test.sprite");
    int fp = dfs_open("/white.sprite");
    sprite_t *sprite = malloc(dfs_size(fp));
    dfs_read(sprite, 1, dfs_size(fp), fp);
    dfs_close(fp);

    uint32_t display_width = display_get_width();
    uint32_t display_height = display_get_height();

    /* Create viewport */
    ugfx_viewport_t viewport;
    ugfx_viewport_init(&viewport, 0, 0, display_width, display_height);

    data_cache_hit_writeback(&viewport, sizeof(viewport));

    /* Construct view + projection matrix */
    ugfx_matrix_t pv_matrix;
    float pv_matrix_f[4][4];

    float near_plane = .1f;
    float far_plane = 100.f;

    perspective(RADIANS(70.f), 4.f/3.f, near_plane, far_plane, pv_matrix_f);
    ugfx_matrix_from_column_major(&pv_matrix, pv_matrix_f[0]);
    data_cache_hit_writeback(&pv_matrix, sizeof(pv_matrix));

    /* Calculate perspective normalization scale. This is needed to re-normalize W-coordinates
       after they have been distorted by the perspective matrix. */
    uint16_t perspective_normalization_scale = float_to_fixed(get_persp_norm_scale(near_plane, far_plane), 16);

    /* Allocate depth buffer */
    void *depth_buffer = malloc(display_width * display_height * 2);

    int rotation_counter = 0;

    while (1)
    {
        static display_context_t disp = 0;

        /* Grab a render buffer */
        while( !(disp = display_lock()) );

        ugfx_matrix_t m_matrix;

        /* Quick'n'dirty rotation + translation matrix */
        float z = -3.0f;
        float angle = RADIANS((float)(rotation_counter++) * 4.0f);
        float c = cosf(angle);
        float s = sinf(angle);
        float m_matrix_f[4][4] = {
            {c,    0.0f, -s,   0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {s,    0.0f, c,    0.0f},
            {0.0f, 0.0f, z,    1.0f}
        };

        ugfx_matrix_from_column_major(&m_matrix, m_matrix_f[0]);
        data_cache_hit_writeback(&m_matrix, sizeof(m_matrix));

        /* Prepare the command list to be executed by the microcode */
        ugfx_command_t commands [] = {
            /* Set general settings */
            ugfx_set_scissor(0, 0, display_width << 2, display_height << 2, UGFX_SCISSOR_DEFAULT),
            ugfx_load_viewport(0, &viewport),
            ugfx_set_z_image(depth_buffer),

            /* Prepare for buffer clearing */
            ugfx_set_other_modes(UGFX_CYCLE_FILL),

            /* Clear depth buffer */
            ugfx_set_color_image(depth_buffer, UGFX_FORMAT_RGBA, UGFX_PIXEL_SIZE_16B, display_width - 1),
            ugfx_set_fill_color(PACK_ZDZx2(0xFFFF, 0)),
            ugfx_fill_rectangle(0, 0, display_width << 2, display_height << 2),

            /* Clear color buffer (note that color buffer will still be set afterwards) */
            ugfx_set_display(disp),
            ugfx_set_fill_color(PACK_RGBA16x2(40, 20, 30, 255)),
            ugfx_fill_rectangle(0, 0, display_width << 2, display_height << 2),

            /* Set up projection matrix */
            ugfx_set_view_persp_matrix(0, &pv_matrix),
            ugfx_set_persp_norm(perspective_normalization_scale),

            /* Set lights */
            /* The ambient light is always active at index 0. ugfx_set_num_lights(n) only sets the number of directional lights. */
            ugfx_set_num_lights(1),
            ugfx_load_light(0, &lights[0], 0),
            ugfx_load_light(0, &lights[1], 1),

            /* Set render modes for drawing the mesh */
            ugfx_set_other_modes(
                UGFX_CYCLE_1CYCLE |
                ugfx_blend_1cycle(UGFX_BLEND_IN_RGB, UGFX_BLEND_IN_ALPHA, UGFX_BLEND_MEM_RGB, UGFX_BLEND_1_MINUS_A) |
                UGFX_SAMPLE_2x2 | UGFX_Z_OPAQUE | UGFX_Z_SOURCE_PIXEL | UGFX_CVG_CLAMP | UGFX_BI_LERP_0 | UGFX_BI_LERP_1 |
                UGFX_Z_COMPARE | UGFX_Z_UPDATE | UGFX_PERSP_TEX | UGFX_ALPHA_CVG_SELECT | UGFX_IMAGE_READ | UGFX_ANTIALIAS),
            ugfx_set_combine_mode(
                UGFX_CC_SHADE_COLOR, UGFX_CC_SUB_0, UGFX_CC_T0_COLOR, UGFX_CC_ADD_0, UGFX_AC_0, UGFX_AC_0, UGFX_AC_0, UGFX_AC_1,
                UGFX_CC_SHADE_COLOR, UGFX_CC_SUB_0, UGFX_CC_T0_COLOR, UGFX_CC_ADD_0, UGFX_AC_0, UGFX_AC_0, UGFX_AC_0, UGFX_AC_1),
            ugfx_set_cull_mode(UGFX_CULL_BACK),
            ugfx_set_geometry_mode(UGFX_GEOMETRY_SHADE | UGFX_GEOMETRY_ZBUFFER | UGFX_GEOMETRY_TEXTURE | UGFX_GEOMETRY_SMOOTH | UGFX_GEOMETRY_LIGHTING),
            // ugfx_set_geometry_mode(UGFX_GEOMETRY_SHADE | UGFX_GEOMETRY_ZBUFFER | UGFX_GEOMETRY_TEXTURE | UGFX_GEOMETRY_SMOOTH),
            ugfx_set_clip_ratio(2),

            /* Point RDP towards texture data and set tile settings */
            ugfx_set_texture_image(sprite->data, UGFX_FORMAT_RGBA, UGFX_PIXEL_SIZE_32B, sprite->width - 1),
            ugfx_set_tile(UGFX_FORMAT_RGBA, UGFX_PIXEL_SIZE_32B, (2 * sprite->width) >> 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
            ugfx_load_tile(0 << 2, 0 << 2, (sprite->width - 1) << 2, (sprite->height - 1) << 2, 0),

            /* The texture settings to use for the following primitives */
            ugfx_set_texture_settings(0x8000, 0x8000, 0, 0),

            /* Set model matrix and draw mesh by linking to a constant command list */

            ugfx_set_model_matrix(0, &m_matrix),
            // ugfx_set_address_slot(1, mesh_vertices),
            // ugfx_push_commands(0, mesh_commands, mesh_commands_length),
            ugfx_set_address_slot(1, mesh_vtx),
            ugfx_push_commands(0, mesh_cmd, mesh_cmd_len),
            ugfx_sync_pipe(),

            /* Finish up */
            ugfx_sync_full(),
            ugfx_finalize(),
        };

        data_cache_hit_writeback(commands, sizeof(commands));
        data_cache_hit_writeback(mesh_cmd, mesh_cmd_len * sizeof(ugfx_command_t));
        data_cache_hit_writeback(mesh_vtx, mesh_vtx_len * sizeof(ugfx_vertex_t));

        /* Load command list into RSP DMEM and run the microcode */
        ugfx_load(commands, sizeof(commands) / sizeof(*commands));
        rsp_run();

        /* Force backbuffer flip */
        display_show(disp);
    }
}
