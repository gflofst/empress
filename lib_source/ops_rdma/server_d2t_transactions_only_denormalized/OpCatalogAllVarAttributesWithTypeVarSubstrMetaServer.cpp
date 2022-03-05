#include <OpCatalogAllVarAttributesWithTypeVarSubstrMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_var_attributes_with_type_var_substr_stub (const md_catalog_all_var_attributes_with_type_var_substr_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

static int get_matching_attribute_count (const md_catalog_all_var_attributes_with_type_var_substr_args &args, uint32_t &count);



WaitingType OpCatalogAllVarAttributesWithTypeVarSubstrMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  

    md_catalog_all_var_attributes_with_type_var_substr_args sql_args;
    std::vector<md_catalog_var_attribute_entry> attribute_list;
    uint32_t count;
    

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);        
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_catalog_all_var_attributes_with_type_var_substr_stub(sql_args, attribute_list, count);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_STUB);


    std::string serial_str = serializeMsgToClient(attribute_list, count, rc);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}





int md_catalog_all_var_attributes_with_type_var_substr_stub (const md_catalog_all_var_attributes_with_type_var_substr_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query = "select vac.* from var_attribute_catalog vac "
    "inner join var_catalog vc on vac.run_id = vc.run_id and vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and vac.type_id = ? "
    // "and vac.run_id = ? "
    "and vc.name like ? "
    "and (vac.timestep_id = ?) ";


    rc = get_matching_attribute_count (args, count); //assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_var_attributes_with_type_var_substr: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        // // rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        // rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        // rc = sqlite3_bind_int64 (stmt, 2, args.type_id); //assert (rc == SQLITE_OK);
        // rc = sqlite3_bind_int64 (stmt, 3, args.run_id); //assert (rc == SQLITE_OK);        
        // rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); //assert (rc == SQLITE_OK);
        // // cout << "timestep_id: " << args.timestep_id << endl;
        // rc = sqlite3_bind_int64 (stmt, 5, args.timestep_id); //assert (rc == SQLITE_OK);


        // rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.type_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_text (stmt, 3, strdup(args.var_name_substr.c_str()), -1, free); //assert (rc == SQLITE_OK);
        // cout << "timestep_id: " << args.timestep_id << endl;
        rc = sqlite3_bind_int64 (stmt, 4, args.timestep_id); //assert (rc == SQLITE_OK);


        rc = sql_retrieve_var_attrs(stmt, attribute_list);


        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count (const md_catalog_all_var_attributes_with_type_var_substr_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    // const char * query = "select count (*) from var_attribute_catalog vac "
    // "inner join var_catalog vc on vac.run_id = vc.run_id and vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
    // "where (vac.txn_id = ? or vac.active = 1) "
    // "and vac.type_id = ? "
    // "and vac.run_id = ? "
    // "and vc.name like ? "
    // "and (vac.timestep_id = ?) ";

    const char * query = "select count (*) from var_attribute_catalog vac "
    "inner join var_catalog vc on vac.run_id = vc.run_id and vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and vac.type_id = ? "
    // "and vac.run_id = ? "
    "and vc.name like ? "
    "and (vac.timestep_id = ?) ";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.type_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.var_name_substr.c_str()), -1, free); //assert (rc == SQLITE_OK);
    // cout << "timestep_id: " << args.timestep_id << endl;
    rc = sqlite3_bind_int64 (stmt, 4, args.timestep_id); //assert (rc == SQLITE_OK);


    // rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    // rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
    // rc = sqlite3_bind_int64 (stmt, 2, args.type_id); //assert (rc == SQLITE_OK);
    // rc = sqlite3_bind_int64 (stmt, 3, args.run_id); //assert (rc == SQLITE_OK);        
    // rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); //assert (rc == SQLITE_OK);
    // // cout << "timestep_id: " << args.timestep_id << endl;
    // rc = sqlite3_bind_int64 (stmt, 5, args.timestep_id); //assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    return rc;
}
