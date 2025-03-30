#include <stdio.h>
#include <random>
#include <stdlib.h>
#include <Windows.h>
#include <math.h>
#include <assert.h>
#define WIN32_LEAN_AND_MEAN

typedef double f64;

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


int main(int arg_count, char **args) {

    int seed;
    int num_point_pairs;

    if(arg_count == 3)
    {
        char *a1 = args[1];
        seed = atoi(a1);
        fprintf(stdout, "seed:%d\n", seed);
        char *a2 = args[2];
        num_point_pairs = atoi(a2);
        assert(num_point_pairs > 0 && num_point_pairs < 16000001);
        fprintf(stdout, "npp:%d\n", num_point_pairs);
    }
    else
    {
        fprintf(stdout, "Usage: [seed] [number of pairs to generate]\n");
        return 1;
    }

    int num_points = num_point_pairs*4;
    size_t storage_size = sizeof(f64)*num_points;
    void *point_storage = VirtualAlloc(0, storage_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    size_t haversine_storage_size = sizeof(f64)*num_point_pairs;
    void *haversine_storage = VirtualAlloc(0, haversine_storage_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(-180.0, 180.0);

    f64 *pairs = (f64 *)point_storage;
    for(int i = 0; i < num_points; i++) 
    {
        pairs[i] = dist(rng);
    }

    FILE *file = fopen("points.json", "w");
    if(file == NULL) 
    {
        fprintf(stdout, "ERROR: Can't open/create file\n");
        return 1;
    }

    f64 *haversine_distances = (f64 *)haversine_storage;

    fprintf(file, "{\"pairs\":[\n");

    for(int i = 0; i < num_point_pairs; i++) 
    {
        if(i < num_point_pairs-1)
        {
            fprintf(file, "    {\"X0\":%f, \"Y0\":%f, \"X1\":%f, \"Y1\":%f},\n", pairs[i], pairs[i+1], pairs[i+2], pairs[i+3]);
            haversine_distances[i] = ReferenceHaversine(pairs[i], pairs[i+1], pairs[i+2], pairs[i+3], 6372.8);
        }
        else if(i == num_point_pairs-1)
        {
            fprintf(file, "    {\"X0\":%f, \"Y0\":%f, \"X1\":%f, \"Y1\":%f}\n", pairs[i], pairs[i+1], pairs[i+2], pairs[i+3]);
            haversine_distances[i] = ReferenceHaversine(pairs[i], pairs[i+1], pairs[i+2], pairs[i+3], 6372.8);
        }
    }

    f64 h_sum = 0;
    for(int i = 0; i < num_point_pairs; ++i)
    {
        h_sum += haversine_distances[i];
        //fprintf(stdout, "%f\n", haversine_distances[i]);
    }
    assert(h_sum > 0);
    f64 result = h_sum/num_point_pairs;

    fprintf(file, "]}");

    fclose(file);
    
    fprintf(stdout, "Final output: %f\nFile written successfully.\n", result);
    return 0;
}