#include <intrin.h>

static Profile_Array profile_array;
static b32 profiler_initiated = false;

static void copy_func_name(char *dest, char *src, u32 size)
{
    // NOTE: Function names are hardcoded to be limited to 32 characters for now.
    // If we get a string that's longer, it simply gets truncated to 32 characters
    // and we add null ourselves.
    if(size > 32)
    {
        size = 32;
    }
    for(u32 i = 0; i < size; ++i)
    {
        *dest = *src;
        ++dest;
        ++src;
        if(i == 32)
        {
            *dest = '\0';
        }
    }
}

static u32 func_name_length(char *func_name)
{
    u32 length = 0;
    while(*func_name != '\0')
    {
        if(length > 32)
        {
            break;
        }

        ++length;
        ++func_name;
    }
    return(length);
}

static u64 get_os_timer_freq()
{
	LARGE_INTEGER Freq;
	QueryPerformanceFrequency(&Freq);
	return Freq.QuadPart;
}

static u64 read_os_timer()
{
	LARGE_INTEGER Value;
	QueryPerformanceCounter(&Value);
	return Value.QuadPart;
}

inline u64 read_cpu_timer()
{	
	return __rdtsc();
}

static u64 get_os_elapsed() 
{
    u64 os_freq = get_os_timer_freq();
    u64 os_start = read_os_timer();
    u64 os_end = 0;
    u64 os_elapsed = 0;
    u64 os_wait_time = os_freq * 100 / 1000;
    while(os_elapsed < os_wait_time)
    {
        os_end = read_os_timer();
        os_elapsed = os_end - os_start;
    }
    return(os_elapsed);
}

static void push_profile(Profile profile)
{
    if(profile_array.num_profiles < profile_array.size)
    {
        *profile_array.current_free_space = profile;
        ++profile_array.current_free_space;
        ++profile_array.num_profiles;
    }
    else
    {
        fprintf(stdout, "Warning: Out of free space in Profile_Array\n");
    }
}

static void print_profile(char *name, Profile *profile)
{

}

static void begin_profile()
{
    profiler_initiated = true;
    profile_array = {};
    void *data = VirtualAlloc(0, 6144, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    profile_array.profiles = (Profile *)data;
    profile_array.current_free_space = (Profile *)data;
    profile_array.program_begin_time = read_cpu_timer();
}

//TODO: Cycle through the profiles you've made and process/print them
static void end_profile()
{
    if(!profiler_initiated)
    {
        fprintf(stdout, "ERROR: Cannot end profile when no profile was initiated.");
        return;
    }
    profile_array.program_end_time = read_cpu_timer();

    u64 os_freq = get_os_timer_freq();
    u64 os_elapsed = get_os_elapsed();

    u64 program_elapsed = profile_array.program_end_time - profile_array.program_begin_time;

    u64 cpu_freq = 0;

    if(os_elapsed)
    {
        cpu_freq = os_freq * program_elapsed / os_elapsed;
    }
    
    fprintf(stdout, "  CPU Timer: %llu -> %llu = %llu elapsed\n", profile_array.program_begin_time, profile_array.program_end_time, program_elapsed);
	fprintf(stdout, "  CPU Freq: %llu (guessed)\n\n", cpu_freq);

    // NOTE: Formula for percentage is (program_section_elapsed*100) / program_elapsed
    fprintf(stdout, "  Program: %llu || (%%%f)\n", cpu_freq, (f64)(program_elapsed*100)/(f64)program_elapsed);
}

// TODO: Wrap the creation of these with a check of the profiler init bool
class Profiler
{
    Profile profile;

    Profiler(char *func_name)
    {
        profile = {};
        copy_func_name(profile.func_name, func_name, func_name_length(func_name));
        profile.begin_time = read_cpu_timer();
    }

    ~Profiler()
    {
        profile.end_time = read_cpu_timer();
        push_profile(this->profile);
    }
};