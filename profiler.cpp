#include <intrin.h>

static Prof_Buffer global_prof_buffer;
static u32 global_profile_parent;

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

static void print_profile(Profile *profile, u64 program_elapsed)
{
    u64 elapsed = profile->elapsed - profile->elapsed_children;
    f64 percentage = (f64)(profile->elapsed*100)/(f64)program_elapsed;
    fprintf(stdout, "  %s[%llu]: %llu elapsed || (%%%.2f", profile->label, profile->hit_count, elapsed, percentage);
    if(profile->elapsed_children)
    {
        f64 percentage_with_children = (f64)(profile->elapsed*100) / (f64)program_elapsed;
        fprintf(stdout, ", %.2f%% w/children", percentage_with_children);
    }
    fprintf(stdout, ")\n");
}

static void begin_profile()
{
    global_prof_buffer.program_begin_time = read_cpu_timer();
}

static void end_profile()
{
    global_prof_buffer.program_end_time = read_cpu_timer();

    u64 os_freq = get_os_timer_freq();
    u64 os_elapsed = get_os_elapsed();

    u64 program_elapsed = global_prof_buffer.program_end_time - global_prof_buffer.program_begin_time;

    u64 cpu_freq = 0;

    if(os_elapsed)
    {
        cpu_freq = os_freq * program_elapsed / os_elapsed;
    }
    
    fprintf(stdout, "  CPU Timer: %llu -> %llu = %llu elapsed\n", global_prof_buffer.program_begin_time, global_prof_buffer.program_end_time, program_elapsed);
	fprintf(stdout, "  CPU Freq: %llu (guessed)\n\n", cpu_freq);

    fprintf(stdout, "  Program: %llu elapsed\n\n", cpu_freq);

    for(u32 i = 0; i < ArrayCount(global_prof_buffer.profiles); ++i)
    {
        if(global_prof_buffer.profiles[i].elapsed)
        {
            print_profile(&global_prof_buffer.profiles[i], program_elapsed);
        }
    }
    fprintf(stdout, "\n");
}

class Profiler
{
    public:
    char const *label;
    u64 begin;
    u64 end;
    u32 profile_index;
    u32 parent_index;

    Profiler(char const *label_, u32 profile_index_)
    {
        this->label = label_;
        this->parent_index = global_profile_parent;
        this->profile_index = profile_index_;
        global_profile_parent = this->profile_index;
        this->begin = read_cpu_timer();
    }

    ~Profiler()
    {
        this->end = read_cpu_timer();

        global_profile_parent = this->parent_index;

        Profile *parent = global_prof_buffer.profiles + this->parent_index;
        Profile *profile = global_prof_buffer.profiles + this->profile_index;

        u64 elapsed = this->end - this->begin;

        parent->elapsed_children += elapsed;
        profile->elapsed += elapsed;
        ++profile->hit_count;
        profile->label = this->label;
    }
};

#define PROFILE_FUNCTION PROFILE_BLOCK(__func__)
#define PROFILE_BLOCK(name) Profiler profiler(name, __COUNTER__ + 1)
