#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <Windows.h>
#include <assert.h>
#define WIN32_LEAN_AND_MEAN

#include "h_calc.h"
#include "profiler.h"
#include "profiler.cpp"

static void allocate_buffer(Buffer *buffer, size_t size)
{
    void *data = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    buffer->data = data;
    buffer->size = size;
}

static void free_buffer(Buffer *buffer)
{
    if(buffer->data)
    {
        VirtualFree(buffer->data, 0, MEM_RELEASE);
    }
    *buffer = {};
}

static void load_memory_from_file(char *filename, Buffer *buffer)
{
    FILE *file = fopen(filename, "rb");
    if(file)
    {  
        struct __stat64 stat;
        _stat64(filename, &stat);
        allocate_buffer(buffer, stat.st_size);
        size_t bytes_read = fread(buffer->data, 1, buffer->size, file);
        fclose(file);
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open %s.\n", filename);
    }
}

static f64 Square(f64 A)
{
    f64 Result = (A*A);
    return Result;
}

static f64 RadiansFromDegrees(f64 Degrees)
{
    f64 Result = 0.01745329251994329577f * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    /* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
       Instead, it attempts to follow, as closely as possible, the formula used in the real-world
       question on which these homework exercises are loosely based.
    */
    
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;
    
    f64 dLat = RadiansFromDegrees(lat2 - lat1);
    f64 dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);
    
    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));
    
    f64 Result = EarthRadius * c;
    
    return Result;
}

static void *push_struct(void *dest, void* src, size_t size)
{
    char *d = (char *)dest;
    char *s = (char *)src;

    for(size_t i = 0; i < size; ++i)
    {
        d[i] = s[i];
    }

    d += size;
    return((void *)d);
}

static int parse_points_json(Buffer *file_buffer, Buffer *data_buffer)
{
    int result = 0;
    char *buffer = (char *)file_buffer->data;
    void *memory = data_buffer->data;
    char *end = strchr(buffer, ']');

    if(strncmp(buffer, "{\"pairs\":[", 10) == 0)
    {
        buffer += 10;
    }

    while(buffer < end)
    {
        Point p = {};

        sscanf(buffer, "{ %*[^:]:%lf , %*[^:]:%lf , %*[^:]:%lf , %*[^:]:%lf }", 
            &p.x0, &p.y0, &p.x1, &p.y1);

        memory = push_struct(memory, &p, sizeof(p));

        buffer += 1;
        char *test = strchr(buffer, '{');
        if(test)
        {
            ++result;
            buffer = test;
        }
        else
        {
            break;
        }
    }
    return(result);
}


int main(int arg_count, char **args) {

    u64 cpu_start = read_cpu_timer();

    size_t data_storage_size = (size_t)(1024*1024*1024*0.6);
    size_t result_storage_size = (size_t)(1024*1024*1024*0.15);

    Buffer data_buffer = {};
    allocate_buffer(&data_buffer, data_storage_size);
    Buffer result_buffer = {};
    allocate_buffer(&result_buffer, result_storage_size);

    Buffer file_buffer = {};
    load_memory_from_file("points.json", &file_buffer);

    u64 cpu_alloc_load_end = read_cpu_timer();

    int num_points = parse_points_json(&file_buffer, &data_buffer);

    u64 cpu_parse_end = read_cpu_timer();

    Point *data = (Point *)data_buffer.data;
    f64 *results = (f64 *)result_buffer.data;
    for(int i = 0; i <= num_points; ++i)
    {
        //fprintf(stdout, "X0:%f Y0:%f X1:%f Y1:%f\n", data[i].x0, data[i].y0, data[i].x1, data[i].y1);
        results[i] = ReferenceHaversine(data[i].x0, data[i].y0, data[i].x1, data[i].y1, 6372.8);
    }

    u64 cpu_h_calc_end = read_cpu_timer();

    f64 h_sum = 0;
    for(int i = 0; i <= num_points; ++i)
    {
        h_sum += results[i];
        //fprintf(stdout, "%f\n", results[i]);
    }
    
    f64 result = h_sum / num_points;

    u64 cpu_sum_end = read_cpu_timer();
    
    fprintf(stdout, "  Num points: %d\n", num_points);
    fprintf(stdout, "  Result: %f\n\n", result);

    free_buffer(&file_buffer);
    free_buffer(&data_buffer);
    free_buffer(&result_buffer);

    u64 cpu_end = read_cpu_timer();

    return 0;
}