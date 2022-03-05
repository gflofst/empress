#ifndef MYMETADATASERVER_HH
#define MYMETADATASERVER_HH

#include <my_metadata_args.h>
#include <sqlite3.h>

#ifndef MD_DB_TYPE_ENUM
#define MD_DB_TYPE_ENUM
enum md_db_type : unsigned short
{
    DB_IN_MEM = 0,
    DB_PREV = 1,
    DB_OUTPUT = 2,
};
#endif //MD_DB_CHECKPOINT_TYPE_ENUM


// static int metadata_database_init(bool load_db, uint64_t job_id, md_write_type write_type);
// static int metadata_database_init(sqlite3 *db, bool load_db, uint64_t job_id);
// static int metadata_database_init(sqlite3 **db, bool load_db, uint64_t job_id);
static int metadata_database_init(sqlite3 **db, bool load_db, bool create_db, uint64_t job_id, bool db_reset);

// static int setup_dirman(const std::string &dirman_hostname, const std::string &app_name, int rank, int dirman_port = 1990);
static int setup_dirman(const std::string &dirman_file_path, const std::string &app_name, int rank);

int sql_output_db(sqlite3 *pInMemory, uint64_t job_id, int rank);
// static int sql_load_db(sqlite3 *pInMemory, uint64_t job_id, int rank);
static int sql_load_db(sqlite3 *db, uint64_t job_id);
static int db_copy(sqlite3 *db, std::string filename);
static int db_incr_output(sqlite3 *db, std::string filename);
static int add_new_to_db_checkpoint_new_file(std::string output_filename, std::string prev_filename);
static int db_delete(sqlite3 *db);
static bool is_db_output_start(uint16_t code);
static int db_reset(sqlite3 *db, uint64_t job_id);
static int get_seq_values(sqlite3 *db, md_db_type checkpt_type, uint64_t &last_run_id, uint64_t &last_type_id, 
                    uint64_t &last_run_attr_id,uint64_t &last_timestep_attr_id, uint64_t &last_var_attr_id,  
                    bool &contains_types, bool &contains_run_attrs, bool &contains_timestep_attrs, 
                    bool &contains_var_attrs
                );
static int update_seq_values(sqlite3 *db, uint64_t last_run_id, uint64_t last_type_id, uint64_t last_run_attr_id, uint64_t last_timestep_attr_id, 
                    uint64_t last_var_attr_id, bool contains_types, bool contains_run_attrs,
                    bool contains_timestep_attrs, bool contains_var_attrs
                    );
static int get_last_timestep(sqlite3 *db, md_db_type db_type, uint64_t last_run_id, uint64_t &last_timestep_id);
static int insert_new(sqlite3 *db, uint64_t last_run_id, uint64_t last_timestep_id, uint64_t last_type_id, 
                    uint64_t last_run_attr_id, uint64_t last_timestep_attr_id, 
                    uint64_t last_var_attr_id, bool contains_types, bool contains_run_attrs,
                    bool contains_timestep_attrs, bool contains_var_attrs
                    );
// static int db_compact_and_output(uint64_t job_id, md_db_checkpoint_type checkpt_type);
static int db_compact_and_output(sqlite3 *db, uint64_t job_id);
static int db_compact_and_output(uint64_t job_id);
static int register_ops ();
static int create_reading_indices(sqlite3 *db);
sqlite3_int64 get_db_size (sqlite3 *db);
// static std::string get_filename_base(uint64_t job_id, md_db_checkpoint_type checkpt_type);
static std::string get_filename_base(uint64_t job_id);
static int db_incr_output_delete(sqlite3 *db, std::string filename);
static int insert_all_new(sqlite3 *db);

#endif //MYMETADATASERVER_HH
