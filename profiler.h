struct Profile
{
    char func_name[32];
    u64 begin_time;
    u64 end_time;
};

struct Prof_Buffer
{
    Profile *profiles;
    Profile *current_free_space;
    u32 num_profiles;
    u32 size;
    u64 program_begin_time;
    u64 program_end_time;
};