#include <OpDeleteTypeByNameVerMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

int md_delete_type_by_name_ver_stub (const md_delete_type_by_name_ver_args &args);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


WaitingType OpDeleteTypeByNameVerMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  

    md_delete_type_by_name_ver_args sql_args;

    
    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);        
    add_timing_point(OP_DELETE_TYPE_BY_NAME_VER_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_delete_type_by_name_ver_stub(sql_args);
    add_timing_point(OP_DELETE_TYPE_BY_NAME_VER_MD_DELETE_TYPE_BY_NAME_VER_STUB);

    // std::string serial_str = serializeMsgToClient(rc);
    add_timing_point(OP_DELETE_TYPE_BY_NAME_VER_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          to_string(rc));
    add_timing_point(OP_DELETE_TYPE_BY_NAME_VER_CREATE_MSG_FOR_CLIENT);
 
    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_DELETE_TYPE_BY_NAME_VER_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}





int md_delete_type_by_name_ver_stub (const md_delete_type_by_name_ver_args &args)
{
    int rc = RC_OK;
    char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query1 = "delete from run_attribute_catalog where run_attribute_catalog.id in (select rac.id from run_attribute_catalog rac "
        "inner join type_catalog tc on rac.type_id = tc.id "
        "where tc.run_id = ? "
        "and tc.name = ? "
        "and tc.version = ? )";

    const char * query2 = "delete from timestep_attribute_catalog where timestep_attribute_catalog.id in (select tac.id from timestep_attribute_catalog tac "
        "inner join type_catalog tc on tac.type_id = tc.id "
        "where tc.run_id = ? "
        "and tc.name = ? "
        "and tc.version = ? )";

    const char * query3 = "delete from var_attribute_catalog where var_attribute_catalog.id in (select vac.id from var_attribute_catalog vac "
        "inner join type_catalog tc on vac.type_id = tc.id "
        "where tc.run_id = ? "
        "and tc.name = ? "
        "and tc.version = ? )";
    // const char * query1 = "delete from attribute_catalog where attribute_catalog.type_id = ? and attribute_catalog.run_id = ? ";
    // const char * query2 = "delete from type_catalog where type_catalog.id = ? and type_catalog.run_id = ? ";
    const char * query4 = "delete from type_catalog where run_id = ? and name = ? and version = ? ";

    // rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error begin query delete_type_by_name_ver_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    //     goto cleanup;
    // }

    rc = sqlite3_prepare_v2 (db, query1, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error first query delete_type_by_name_ver_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 2, strdup(args.name.c_str()), -1, free); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 3, args.version); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query2, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error second query delete_type_by_name_ver_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }
    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 2, strdup(args.name.c_str()), -1, free); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 3, args.version); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query3, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error third query delete_type_by_name_ver_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }
    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 2, strdup(args.name.c_str()), -1, free); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 3, args.version); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query4, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error fourth query delete_type_by_name_ver_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }
    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 2, strdup(args.name.c_str()), -1, free); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 3, args.version); //assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt);

    // rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error end query delete_type_by_name_ver_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    //     goto cleanup;
    // }

cleanup:
    // if (rc != SQLITE_OK)
    // {
    //     rc = sqlite3_exec (db, "rollback;", callback, 0, &ErrMsg);
    // }

    return rc;
}

