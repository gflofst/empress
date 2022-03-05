#include <OpCatalogAllTimestepAttributesMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database(uint64_t txn_id);
extern sqlite3 *main_db;


using namespace std;

int md_catalog_all_timestep_attributes_stub (sqlite3 *db, const md_catalog_all_timestep_attributes_args &args,
                           std::vector<md_catalog_timestep_attribute_entry> &entries,
                           uint32_t &count);

static int get_matching_attribute_count (sqlite3 *db, const md_catalog_all_timestep_attributes_args &args, uint32_t &count);


WaitingType OpCatalogAllTimestepAttributesMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    md_catalog_all_timestep_attributes_args sql_args;
    std::vector<md_catalog_timestep_attribute_entry> entries;
    uint32_t count;

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);
    add_timing_point(OP_CATALOG_ALL_TIMESTEP_ATTRIBUTES_DEARCHIVE_MSG_FROM_CLIENT);

    if(sql_args.query_type == COMMITTED) {
        rc = md_catalog_all_timestep_attributes_stub(main_db, sql_args, entries, count);
    }
    else if(sql_args.query_type == UNCOMMITTED) {
        sqlite3 *db = get_database(sql_args.txn_id);
        rc = md_catalog_all_timestep_attributes_stub(db, sql_args, entries, count);
    }
    else if(sql_args.query_type == COMMITTED_AND_UNCOMMITTED) {
        uint32_t temp_count;
        
        sqlite3 *db = get_database(sql_args.txn_id);

        rc = md_catalog_all_timestep_attributes_stub(main_db, sql_args, entries, count);
        if(rc == RC_OK) {
            rc = md_catalog_all_timestep_attributes_stub(db, sql_args, entries, temp_count);
            count += temp_count;
        }
    }
    else {
        cout << "ERROR. Query type " << sql_args.query_type << " is not defined" << endl;
    }

    add_timing_point(OP_CATALOG_ALL_TIMESTEP_ATTRIBUTES_MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_STUB);


    std::string serial_str = serializeMsgToClient(entries, count, rc);
    // cout << "in catalog all, attr list is of size: " << entries.size() << endl;
    add_timing_point(OP_CATALOG_ALL_TIMESTEP_ATTRIBUTES_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_TIMESTEP_ATTRIBUTES_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_TIMESTEP_ATTRIBUTES_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}





int md_catalog_all_timestep_attributes_stub (sqlite3 *db, const md_catalog_all_timestep_attributes_args &args,
                           std::vector<md_catalog_timestep_attribute_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query_all = "select * from timestep_attribute_catalog tac where tac.run_id = ? ";

    const char * query = "select * from timestep_attribute_catalog tac where tac.run_id = ? "
    "and tac.timestep_id = ? ";

    rc = get_matching_attribute_count (db, args, count); //assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        if(args.timestep_id == ALL_TIMESTEPS) {
            rc = sqlite3_prepare_v2 (db, query_all, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
        }
        else {
            rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); //assert (rc == SQLITE_OK);
        }

        rc = sql_retrieve_timestep_attrs(stmt, entries);

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count (sqlite3 *db, const md_catalog_all_timestep_attributes_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query_all = "select count (*) from timestep_attribute_catalog tac where tac.run_id = ? ";

    const char * query = "select count (*) from timestep_attribute_catalog tac where tac.run_id = ? "
    "and tac.timestep_id = ? ";

    if(args.timestep_id == ALL_TIMESTEPS) {
        rc = sqlite3_prepare_v2 (db, query_all, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    }
    else {
        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); //assert (rc == SQLITE_OK);
    }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    return rc;
}