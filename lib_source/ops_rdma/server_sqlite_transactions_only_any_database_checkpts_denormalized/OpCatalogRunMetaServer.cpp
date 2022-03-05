#include <OpCatalogRunMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database(uint64_t job_id, md_query_type query_type, bool &trans_active);
extern void close_database(uint64_t job_id);




using namespace std;

int md_catalog_run_stub (sqlite3 *db, const md_catalog_run_args &args, std::vector<md_catalog_run_entry> &entries, uint32_t &count);



WaitingType OpCatalogRunMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    md_catalog_run_args args;
    std::vector<md_catalog_run_entry> entries;
    uint32_t count = 0;    
    bool trans_active;

    deArchiveMsgFromClient(rdma, incoming_msg, args);
    add_timing_point(OP_CATALOG_RUN_DEARCHIVE_MSG_FROM_CLIENT);

    sqlite3 *db = get_database(args.job_id, args.query_type, trans_active);
    if(!trans_active) {
        rc = md_catalog_run_stub(db, args, entries, count);
     }
    else {
        rc = RC_DB_BUSY;
    }  
    add_timing_point(OP_CATALOG_RUN_MD_CATALOG_RUN_STUB);

    //log("num of entries is " + std::to_string(entries.size()));
    // if(entries.size()>0) {
    //     //log("in md_catalog_run_stub the first entry has name " + entries.at(0).name);

    // }

    std::string serial_str = serializeMsgToClient(entries, count, rc);
    add_timing_point(OP_CATALOG_RUN_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_RUN_CREATE_MSG_FOR_CLIENT);
    
    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_RUN_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}



// extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);

int md_catalog_run_stub (sqlite3 *db, const md_catalog_run_args &args, 
                     std::vector<md_catalog_run_entry> &entries,
                     uint32_t &count
                    )
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    // const char * query = "select * from run_catalog order by name, id";
    const char * query = "select * from run_catalog order by id";
    size_t size = 0;

    rc = sqlite3_prepare_v2 (db, "select count (*) from run_catalog ", -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

//printf ("rows in run_catalog: %d\n", count);
    if (count > 0) {
      entries.reserve(count);


    // char *err_msg = 0;
    // char *sql = "SELECT * FROM run_catalog";
    // cout << "starting run catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with run catalog \n";

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_catalog_run_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            close_database(args.job_id);
            goto cleanup;
        }
        rc = sql_retrieve_runs(stmt, entries);

        rc = sqlite3_finalize (stmt);  
    }
    

cleanup:

    return rc;
}
