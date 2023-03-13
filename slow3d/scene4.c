// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#include "common.h"

#include "fast_obj_dfs.h"

void scene4(display_context_t disp, uint32_t t[8])
{
    static int initialized;
    static fastObjMesh *mesh;
    static int mesh_index;
    static int mesh_count;
    static fastObjMesh *meshes[256];
    static char *mesh_names[256];

    static int matrix_dirty = 1;

    static float tx = 0.0f;
    static float ty = 0.0f;
    static float tz = 6.0f;

    static float rx = 0; //M_PI/4;
    static float rz = 0; //M_PI/4;
    static float ry = 0;

    if (!initialized) {
        initialized = 1;
        char path[256];

        int flags = dfs_dir_findfirst("/", path);
        while (flags == FLAGS_FILE) {
            fprintf(stderr, "Loading %s\n", path);
            if (strstr(path, ".obj")) {
                mesh_names[mesh_count] = strdup(path);
                meshes[mesh_count] = fast_obj_read_with_callbacks(path, &fast_obj_dfs_cb, NULL);
                mesh_count++;
            }
            flags = dfs_dir_findnext(path);
        }

        mesh = meshes[0];
    }

    struct controller_data keys = get_keys_down();
    if (keys.c[0].right) {
        mesh_index = (mesh_index + 1) % mesh_count;
        mesh = meshes[mesh_index];
        fprintf(stderr, "Rendering mesh [%s]\n", mesh_names[mesh_index]);
    }

    keys = get_keys_pressed();
    if (keys.c[0].R) {
        tz -= 0.1f;
        matrix_dirty = 1;
    }

    if (keys.c[0].L) {
        tz += 0.1f;
        matrix_dirty = 1;
    }

    // TODO: Handle rotation with the stick

    rdp_sync(SYNC_PIPE);
    t[1] = TICKS_READ();

    rdp_set_default_clipping();
    rdp_attach_display(disp);
    rdp_sync(SYNC_PIPE);
    t[2] = TICKS_READ();

    rdp_enable_primitive_fill();
    rdp_set_primitive_color(0x00000000);
    rdp_draw_filled_rectangle(0, 0, __width, __height);
    t[3] = TICKS_READ();


    static Matrix4f camera;
    static Matrix4f translation;
    static Matrix4f rotation;
    static Matrix4f transRot;
    static Matrix4f viewProjection;
    static Matrix4f screenSpaceTransform;
    static Matrix4f mvp;

    static Vector4f lightDir = {
        .x = 0.0f,
        .y = 0.0f,
        .z = 1.0f,
        .w = 0.0f,
    };

    static float ambient = 0.1f;

    // keep spinning
    ry = g_frame / 30.0f;
    matrix_dirty = 1;

    if (matrix_dirty) {
        Matrix4f temp;
        float fov_degrees = 70;
        float fov = fov_degrees * (M_PI / 180.0f);
        float aspect = __width / ((float) __height);
        float near = 0.1f;
        float far = 100.0f;

        matrix_perspective(&camera, fov, aspect, near, far);

        matrix_translate(&translation, tx, ty, tz);

        // Rotate the cube 45 degrees around the x and z axis, then rotate over time around the y axis.
        Matrix4f rot_x;
        Matrix4f rot_y;
        Matrix4f rot_z;
        matrix_rotate_x(&rot_x, rx);
        matrix_rotate_z(&rot_z, rz);
        matrix_rotate_y(&rot_y, ry);
        matrix_mul(&rot_z, &rot_x, &transRot);
        matrix_mul(&rot_y, &transRot, &rotation);

        matrix_mul(&translation, &rotation, &temp);
        matrix_mul(&camera, &temp, &viewProjection);

        matrix_screen_space_transform(&screenSpaceTransform, __width / 2.0f, __height / 2.0f);

        // Precalculate screenSpaceTransform * transform so we don't have to do that for every vertex
        matrix_mul(&screenSpaceTransform, &viewProjection, &mvp);

        matrix_dirty = 0;
    }

    t[4] = TICKS_READ();

    // TODO: Add guards to protect against FPU exceptions
    rdp_enable_blend_fill();
    uint32_t last_color = 0;

    // For each group in mesh
    for (unsigned int ii = 0; ii < mesh->group_count; ii++) {
        const fastObjGroup *grp = &mesh->groups[ii];
        uint32_t idx = 0;

        //  For each face in group
        for (unsigned int jj = 0; jj < grp->face_count; jj++) {
            unsigned int fv = mesh->face_vertices[grp->face_offset + jj];

            // For each vertex in face
            assert(fv >= 3);
            Vertex face[fv];
            float lightAmount[fv];
            Vector4f flatLightNormal = {0.0f, 0.0f, 0.0f, 0.0f};
            float flatLightAmount;

            for (unsigned int kk = 0; kk < fv; kk++) {
                Vector4f transformedNormal;
                fastObjIndex *mi = &mesh->indices[grp->index_offset + idx];
                idx++;

                face[kk].col.r = jj % 256;
                face[kk].col.g = jj / 256;

                // Skip if there is no position data
                if (!mi->p) {
                    continue;
                }

                face[kk].pos.x = mesh->positions[3 * mi->p + 0];
                face[kk].pos.y = mesh->positions[3 * mi->p + 1];
                face[kk].pos.z = mesh->positions[3 * mi->p + 2];
                face[kk].pos.w = 1.0;

                // Skip if there is no position data
                if (!mi->n) {
                    lightAmount[kk] = 1.0f;
                    // fprintf(stderr, "Missing normal data!\n");
                    continue;
                }

                face[kk].normal.x = mesh->positions[3 * mi->n + 0];
                face[kk].normal.y = mesh->positions[3 * mi->n + 1];
                face[kk].normal.z = mesh->positions[3 * mi->n + 2];
                face[kk].normal.w = 0.0;

                vector_transform(&transRot, &face[kk].normal, &transformedNormal);

                lightAmount[kk] = saturatef(vector_dot(&transformedNormal, &lightDir)) * (1.0f - ambient) + ambient;
                vector_add(&transformedNormal, &flatLightNormal, &flatLightNormal);
            }

            for (int j = 0; j < fv; j++) {
                // vertex_transform(&transform, &face[j], &face[j]);
                // vertex_transform(&screenSpaceTransform, &face[j], &face[j]);
                vertex_transform(&mvp, &face[j], &face[j]);
                vertex_perspective_divide(&face[j]);
            }

            // Back-face culling. Only draw clock-wise/right hand triangles.
            if (vertex_triangle_area2(&face[0], &face[1], &face[2]) <= 0) {
                continue;
            }

            // Only use the first vertex' light for now
            // uint32_t color = (((int)(face[0].col.r * lightAmount[0])) << 24) | 0xFF;
            // uint32_t color = (((int)(240.0f * lightAmount[0])) << 24) | 0xFF;

            // Average the normals to do proper flat shading
            (void) lightAmount;
            flatLightAmount = saturatef(vector_dot(&flatLightNormal, &lightDir)) * (1.0f - ambient) + ambient;
            uint32_t color = (((int)(255.0f * flatLightAmount)) << 24) | 0xFF;

            // Sync pipe and set color if it is changed
            if (color != last_color) {
                rdp_sync(SYNC_PIPE);
                rdp_set_blend_color(color);
                last_color = color;
            }

            // Draw polygon as a triangle fan with a root at index 0.
            for (int i = 1; i < fv - 1; i++) {
                rdp_draw_filled_triangle((int)face[    0].pos.x, (int)face[    0].pos.y,
                                         (int)face[    i].pos.x, (int)face[    i].pos.y,
                                         (int)face[i + 1].pos.x, (int)face[i + 1].pos.y);
            }
        }
    }

    t[5] = TICKS_READ();
    t[6] = TICKS_READ();

    rdp_detach_display();
    t[7] = TICKS_READ();
}
