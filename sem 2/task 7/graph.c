#include "fractions.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct point {
    struct fraction x, y;
};

struct segment {
    struct point from, to;
};

struct edge {
    size_t i, j;
};

struct stack_frame {
    size_t index, stopped_at;
};


static bool point_invalid(struct point p);
static bool point_equal(struct point p1, struct point p2);
static size_t read_points_into(struct point **points);
static struct point intersection(struct segment seg1, struct segment seg2);
static struct fraction get_parameter(struct point origin, struct point direction, struct point point);
static bool on_segment(struct segment seg, struct point p);
static void *malloc_or_die(size_t size);
static void *calloc_or_die(size_t nmemb, size_t size);
static size_t point_to_index(struct point p, struct point *points, size_t points_count);
static bool decouple_segments(struct segment seg1, struct segment seg2, struct segment segs[3]);
static bool edge_same(struct edge e1, struct edge e2);

// Should be enough for static objects in this program
#define VALUES_TO_FREE_CAP 1024
void *values_to_free[VALUES_TO_FREE_CAP] = { 0 };
size_t vtf_index = 0;

#define darr_add(_ARR, _EL, _SIZE, _CAP, _VTF_INDEX)\
do {\
    if ((_SIZE) == (_CAP)) {\
        (_CAP) *= 2;\
        if (!(_CAP))\
            (_CAP)++;\
        void *_TMP = realloc((_ARR), sizeof(*(_ARR)) * (_CAP));\
        if (!_TMP) {\
            fprintf(stderr, "[FATAL]: Failed to reallocate vertices to size %zu bytes\n", sizeof(*(_ARR)) * (_CAP));\
            exit(EXIT_FAILURE);\
        }\
        (_ARR) = _TMP;\
        values_to_free[(_VTF_INDEX)] = _TMP;\
    }\
    (_ARR)[(_SIZE)++] = (_EL);\
} while(0)

void free_values(void) {
    for (size_t i = 0; i < vtf_index; i++)
        free(values_to_free[i]);
}

int main(void) {
    atexit(free_values);

    // Getting user input into path
    struct point *path;
    size_t path_length = read_points_into(&path);
    values_to_free[vtf_index++] = path;

    // Creating vertices buffer from index
    size_t vertices_cap = path_length, vertices_count = path_length;
    struct point *vertices = malloc_or_die(sizeof(*vertices) * vertices_cap);
    memcpy(vertices, path, path_length * sizeof(*path));
    const size_t vertices_vtf = vtf_index++;
    values_to_free[vertices_vtf] = vertices;

    // Creating current edges buffer (contains pairs of indices into vertices)
    size_t edges_list_cap = path_length - 1, edges_list_count = 0;
    struct edge *edges_list = malloc_or_die(sizeof(*edges_list) * edges_list_cap);
    const size_t edges_list_vtf = vtf_index++;
    values_to_free[edges_list_vtf] = edges_list;

    // Setting up path into edges buffer
    for (size_t i = 0; i + 1 < path_length; i++)
        darr_add(edges_list, ((struct edge){ i, (i + 1) % vertices_count }), edges_list_count, edges_list_cap, edges_list_vtf);

    assert(edges_list_count == path_length - 1);

    // Getting rid of collinear intersecting pairs of edges
    // By splitting pairs of intersecting edges into three edges
    bool decoupled = false;
    while (!decoupled) {
        decoupled = true;
        const size_t current_boundary = edges_list_count;
        for (size_t i = 0; i < current_boundary; i++)
            for (size_t j = i + 1; j < current_boundary; j++) {
                assert(current_boundary <= edges_list_count);
                assert(j < edges_list_count);
                if (edges_list[i].i == edges_list[i].j || edges_list[j].i == edges_list[j].j)
                    continue;
                struct segment frs[3];
                if (decouple_segments((struct segment){.from=vertices[edges_list[i].i], .to=vertices[edges_list[i].j]}, (struct segment){.from=vertices[edges_list[j].i], .to=vertices[edges_list[j].j]}, frs)) {
                    edges_list[i] = edges_list[j] = (struct edge){ 0 };
                    decoupled = false;
                    size_t vis[4];
                    assert(point_equal(frs[0].to, frs[1].from));
                    assert(point_equal(frs[1].to, frs[2].from));
                    vis[0] = point_to_index(frs[0].from, vertices, vertices_count);
                    vis[1] = point_to_index(frs[1].from, vertices, vertices_count);
                    vis[2] = point_to_index(frs[2].from, vertices, vertices_count);
                    vis[3] = point_to_index(frs[2].to, vertices, vertices_count);
                    darr_add(edges_list, ((struct edge){vis[0], vis[1]}), edges_list_count, edges_list_cap, edges_list_vtf);
                    darr_add(edges_list, ((struct edge){vis[1], vis[2]}), edges_list_count, edges_list_cap, edges_list_vtf);
                    darr_add(edges_list, ((struct edge){vis[2], vis[3]}), edges_list_count, edges_list_cap, edges_list_vtf);
                }
            }
    }

    // Removing repeating and junk edges
    // (vertex - vertex is senseless and 0-0 is used as null for this reason)
    {
    size_t wi = 0;
    for (size_t i = 0; i < edges_list_count; i++) {
        if (edges_list[i].i == edges_list[i].j)
            continue;
        for (size_t j = 0; j < i; j++)
            if (edge_same(edges_list[i], edges_list[j]))
                goto continue1;
        edges_list[wi++] = edges_list[i];
continue1:
    }
    edges_list_count = wi;
    }

    // Getting rid of intersecting noncollinear pairs of edges
    // By splitting pairs into four edges and adding vertex
    bool intersected = true;
    while (intersected) {
        intersected = false;
        const size_t current_boundary = edges_list_count;
        for (size_t i = 0; i < current_boundary; i++)
            for (size_t j = i + 1; j < current_boundary; j++) {
                assert(current_boundary <= edges_list_count);
                assert(j < edges_list_count);
                if (edges_list[i].i == edges_list[i].j || edges_list[j].i == edges_list[j].j)
                    continue;
                struct segment seg1 = {.from=vertices[edges_list[i].i], .to=vertices[edges_list[i].j]};
                struct segment seg2 = {.from=vertices[edges_list[j].i], .to=vertices[edges_list[j].j]};
                struct point inter = intersection(seg1, seg2);
                assert(point_equal(intersection(seg1, seg2), intersection(seg2, seg1)));
                if (!point_invalid(inter)) {
                    intersected = true;
                    darr_add(vertices, inter, vertices_count, vertices_cap, vertices_vtf);
                    size_t new_vertex_index = point_to_index(inter, vertices, vertices_count);
                    if (edges_list[i].i != new_vertex_index) darr_add(edges_list, ((struct edge){edges_list[i].i, new_vertex_index}), edges_list_count, edges_list_cap, edges_list_vtf);
                    if (edges_list[j].i != new_vertex_index) darr_add(edges_list, ((struct edge){edges_list[j].i, new_vertex_index}), edges_list_count, edges_list_cap, edges_list_vtf);
                    if (edges_list[i].j != new_vertex_index) darr_add(edges_list, ((struct edge){edges_list[i].j, new_vertex_index}), edges_list_count, edges_list_cap, edges_list_vtf);
                    if (edges_list[j].j != new_vertex_index) darr_add(edges_list, ((struct edge){edges_list[j].j, new_vertex_index}), edges_list_count, edges_list_cap, edges_list_vtf);
                    edges_list[i] = edges_list[j] = (struct edge){ 0 };
                }
            }
    }

    // Removing repeating vertices and repointing edges to them
    {
    size_t wi = 0;
    for (size_t i = 0; i < vertices_count; i++) {
        for (size_t j = 0; j < i; j++)
            if (point_equal(vertices[i], vertices[j])) {
                for (size_t k = 0; k < edges_list_count; k++) {
                    if (edges_list[k].i == i) edges_list[k].i = j;
                    if (edges_list[k].j == i) edges_list[k].j = j;
                }
                goto continue2;
            }
        for (size_t k = 0; k < edges_list_count; k++) {
            if (edges_list[k].i == i) edges_list[k].i = wi;
            if (edges_list[k].j == i) edges_list[k].j = wi;
        }
        vertices[wi++] = vertices[i];
continue2:
    }
    vertices_count = wi;
    }

    // Creating adjacency matrix
    // (it was the easiest way to ignore repetitions of edges and then count them up)
    bool *edges = calloc_or_die(sizeof(*edges), vertices_count * vertices_count);
    values_to_free[vtf_index++] = edges;
    for (size_t i = 0; i < edges_list_count; i++) {
        struct edge e = edges_list[i];
        if (e.i == e.j) continue;
        edges[e.i * vertices_count + e.j] = true;
        edges[e.j * vertices_count + e.i] = true;
    }

    // Counting up unique edges
    size_t edges_count = 0;
    for (size_t i = 0; i < vertices_count; i++)
        for (size_t j = i + 1; j < vertices_count; j++)
            if (edges[i * vertices_count + j])
                edges_count++;

    // Some Euler formula for planar graphs, excluding infinite part
    size_t regions = edges_count - vertices_count + 1;

    printf("%zu regions\n", regions);

    exit(EXIT_SUCCESS);
}

static bool point_invalid(struct point p) {
    return fraction_invalid(p.x) || fraction_invalid(p.y);
}

static bool point_equal(struct point p1, struct point p2) {
    return fraction_equal(p1.x, p2.x) && fraction_equal(p1.y, p2.y);
}

static size_t read_points_into(struct point **points) {
#define DIE_WITH_ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE); } while (0)
    size_t points_count;
    if (printf("Enter points count: "), scanf("%zu", &points_count) != 1) DIE_WITH_ERROR("[ERROR]: Failed to read points count\n");
    if (points_count < 5UL || points_count > 100UL) DIE_WITH_ERROR("[ERROR]: Points count has to be between 5 and 100 inclusive. Recieved %zu\n", points_count);
    if (!(*points = malloc(sizeof(**points) * points_count))) DIE_WITH_ERROR("[FATAL]: Failed to allocate points of size %zu bytes\n", sizeof(**points) * points_count);

    printf("Enter points coordinates in pairs, space separated: ");
    for (size_t i = 0; i < points_count; i++) {
        int px, py;
        if (scanf("%d%d", &px, &py) != 2) {
            free(*points);
            *points = NULL;
            DIE_WITH_ERROR("[ERROR]: Failed to read point coordinates\n");
        }
        if (px < -100 || px > 100 || py < -100 || py > 100) {
            free(*points);
            *points = NULL;
            DIE_WITH_ERROR("[ERROR]: Point's coordinates must be in range [-100, 100]\n");
        }
        (*points)[i].x = fraction_from_int(px);
        (*points)[i].y = fraction_from_int(py);
    }

    return points_count;
#undef DIE_WITH_ERROR
}

static struct point intersection(struct segment seg1, struct segment seg2) {
    if (point_equal(seg1.from, seg2.from) || point_equal(seg1.from, seg2.to)
        || point_equal(seg1.to, seg2.from) || point_equal(seg1.to, seg2.to))
        return (struct point){ 0 };

    // seg1 = v1*t+seg1.from, 0 <= t <= 1
    // seg2 = v2*q+seg2.from, 0 <= q <= 1
    struct fraction vx1 = fraction_sub(seg1.to.x, seg1.from.x);
    struct fraction vy1 = fraction_sub(seg1.to.y, seg1.from.y);
    struct fraction vx2 = fraction_sub(seg2.to.x, seg2.from.x);
    struct fraction vy2 = fraction_sub(seg2.to.y, seg2.from.y);

    // Check for collinearity
    // v2{x,y} -> v2'{-y,x} and then dot(v1,v2')
    struct fraction semicross = fraction_add(fraction_mul(vx1, vy2), fraction_mul(vy1, fraction_neg(vx2)));
    // Ignore collinear pairs
    if (fraction_equal(semicross, FRAC_FROM_INT(0))) return (struct point){ 0 };

    struct fraction right_t = fraction_sub(fraction_mul(vy2, fraction_sub(seg2.from.x, seg1.from.x)), fraction_mul(vx2, fraction_sub(seg2.from.y, seg1.from.y)));
    struct fraction t = fraction_div(right_t, semicross);
    struct fraction right_q = fraction_sub(fraction_mul(vx1, fraction_sub(seg1.from.y, seg2.from.y)), fraction_mul(vy1, fraction_sub(seg1.from.x, seg2.from.x)));
    struct fraction q = fraction_div(right_q, semicross);

    if (fraction_less(t, FRAC_FROM_INT(0)) || fraction_greater(t, FRAC_FROM_INT(1))
        || fraction_less(q, FRAC_FROM_INT(0)) || fraction_greater(q, FRAC_FROM_INT(1))) return (struct point){ 0 };

    struct point result = {
        .x = fraction_add(seg1.from.x, fraction_mul(t, vx1)),
        .y = fraction_add(seg1.from.y, fraction_mul(t, vy1)),
    };

    if (!on_segment(seg1, result) && !on_segment(seg2, result)) return (struct point){ 0 };

    return result;
}

static struct fraction get_parameter(struct point origin, struct point direction, struct point point) {
    struct fraction t = fraction_div(fraction_sub(point.y, origin.y), direction.y);
    if (fraction_invalid(t)) t = fraction_div(fraction_sub(point.x, origin.x), direction.x);
    return t;
}

static bool on_segment(struct segment seg, struct point p) {
    // seg = v*t+seg.from, 0 <= t <= 1
    struct fraction vx = fraction_sub(seg.to.x, seg.from.x);
    struct fraction vy = fraction_sub(seg.to.y, seg.from.y);
    if ((fraction_equal(vx, FRAC_FROM_INT(0)) && !fraction_equal(p.x, seg.from.x))
        || (fraction_equal(vy, FRAC_FROM_INT(0)) && !fraction_equal(p.y, seg.from.y)))
        return false;
    struct fraction t = fraction_div(fraction_sub(p.x, seg.from.x), vx);
    // vx == 0
    if (fraction_invalid(t)) {
        t = fraction_div(fraction_sub(p.y, seg.from.y), vy);
        if (!fraction_greater(t, FRAC_FROM_INT(0)) || !fraction_less(t, FRAC_FROM_INT(1)))
            return false;
        return fraction_equal(p.x, fraction_add(seg.from.x, fraction_mul(t, vx)));
    }
    if (!fraction_greater(t, FRAC_FROM_INT(0)) || !fraction_less(t, FRAC_FROM_INT(1)))
        return false;
    return fraction_equal(p.y, fraction_add(seg.from.y, fraction_mul(t, vy)));
}

static void sort_fractions(struct fraction frs[], size_t length) {
    bool sorted = false;
    while (!sorted) {
        sorted = true;
        for (size_t i = 0; i + 1 < length; i++) {
            assert(!fraction_invalid(frs[i]) && !fraction_invalid(frs[i + 1]));
            if (fraction_greater(frs[i], frs[i + 1])) {
                sorted = false;
                struct fraction tmp = frs[i];
                frs[i] = frs[i + 1];
                frs[i + 1] = tmp;
            }
        }
    }
}

static bool decouple_segments(struct segment seg1, struct segment seg2, struct segment segs[3]) {
    struct fraction vx1 = fraction_sub(seg1.to.x, seg1.from.x);
    struct fraction vy1 = fraction_sub(seg1.to.y, seg1.from.y);
    struct fraction vx2 = fraction_sub(seg2.to.x, seg2.from.x);
    struct fraction vy2 = fraction_sub(seg2.to.y, seg2.from.y);

    struct fraction semicross = fraction_add(fraction_mul(vx1, vy2), fraction_mul(vy1, fraction_neg(vx2)));
    if (!fraction_equal(semicross, FRAC_FROM_INT(0))) return false;

    if (on_segment(seg1, seg2.from) || on_segment(seg1, seg2.to) || on_segment(seg2, seg1.from) || on_segment(seg2, seg1.to)) {
        struct fraction ts[4] = {
            get_parameter(seg1.from, (struct point){vx1, vy1}, seg2.from),
            get_parameter(seg1.from, (struct point){vx1, vy1}, seg2.to),
            get_parameter(seg1.from, (struct point){vx1, vy1}, seg1.from),
            get_parameter(seg1.from, (struct point){vx1, vy1}, seg1.to),
        };

        // Intersecting at one point aka vertex, thus ignored, because don't really intersect
        if (fraction_equal(ts[1], ts[2]))
            return false;

        sort_fractions(ts, 4);
        segs[0].from = (struct point){
            .x = fraction_add(fraction_mul(ts[0], vx1), seg1.from.x),
            .y = fraction_add(fraction_mul(ts[0], vy1), seg1.from.y),
        };
        segs[0].to = (struct point){
            .x = fraction_add(fraction_mul(ts[1], vx1), seg1.from.x),
            .y = fraction_add(fraction_mul(ts[1], vy1), seg1.from.y),
        };
        segs[1].from = (struct point){
            .x = fraction_add(fraction_mul(ts[1], vx1), seg1.from.x),
            .y = fraction_add(fraction_mul(ts[1], vy1), seg1.from.y),
        };
        segs[1].to = (struct point){
            .x = fraction_add(fraction_mul(ts[2], vx1), seg1.from.x),
            .y = fraction_add(fraction_mul(ts[2], vy1), seg1.from.y),
        };
        segs[2].from = (struct point){
            .x = fraction_add(fraction_mul(ts[2], vx1), seg1.from.x),
            .y = fraction_add(fraction_mul(ts[2], vy1), seg1.from.y),
        };
        segs[2].to = (struct point){
            .x = fraction_add(fraction_mul(ts[3], vx1), seg1.from.x),
            .y = fraction_add(fraction_mul(ts[3], vy1), seg1.from.y),
        };

        return true;
    }

    return false;
}

static void *malloc_or_die(size_t size) {
    void *mem = malloc(size);
    if (!mem) {
        fprintf(stderr, "[FATAL]: Failed to allocate memory of size %zu bytes\n", size);
        exit(EXIT_FAILURE);
    }
    return mem;
}

static void *calloc_or_die(size_t nmemb, size_t size) {
    void *mem = calloc(nmemb, size);
    if (!mem) {
        fprintf(stderr, "[FATAL]: Failed to allocate memory of size %zu bytes\n", size);
        exit(EXIT_FAILURE);
    }
    return mem;
}

static size_t point_to_index(struct point p, struct point *points, size_t points_count) {
    for (size_t i = 0; i < points_count; i++)
        if (point_equal(points[i], p))
            return i;
    return points_count;
}

static bool edge_same(struct edge e1, struct edge e2) {
    return (e1.i == e2.i && e1.j == e2.j) || (e1.i == e2.j && e1.j == e2.i);
}
