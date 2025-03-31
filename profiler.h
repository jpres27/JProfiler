struct Profile
{
    char const *label;
    u64 hit_count;
    u64 elapsed;
    u64 elapsed_children;
};

struct Prof_Buffer
{
    Profile profiles[4096];

    u64 program_begin_time;
    u64 program_end_time;
};