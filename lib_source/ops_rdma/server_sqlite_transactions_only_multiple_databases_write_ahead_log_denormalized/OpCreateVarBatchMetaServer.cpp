#include <OpCreateVarBatchMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database_for_write(uint64_t txn_id);
extern sqlite3 *main_db;
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


using namespace std;
// int md_create_var_batch_stub (sqlite3 *db, const vector<md_create_var_args> &args,
//                         uint64_t &row_id);
int md_create_var_batch_stub (sqlite3 *db, const vector<md_create_var_args> &args);
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);



WaitingType OpCreateVarBatchMeta::handleMessage(bool rdma, message_t *incoming_msg) {

    int rc;
    vector<md_create_var_args> sql_args;
    // uint64_t row_id;

    //convert the serialized string back into the args and count pointer
    deArchiveMsgFromClient(rdma, incoming_msg, sql_args); 
    add_timing_point(OP_CREATE_VAR_BATCH_DEARCHIVE_MSG_FROM_CLIENT);

    // sqlite3 *db = get_database(sql_args[0].txn_id);    
    sqlite3 *db = get_database_for_write(sql_args[0].txn_id);

    // //log("About to create var stub");
    // rc = md_create_var_batch_stub(db, sql_args, row_id);
    rc = md_create_var_batch_stub(db, sql_args);        
    add_timing_point(OP_CREATE_VAR_BATCH_MD_CREATE_VAR_BATCH_STUB);

    // std::string serial_str = serializeMsgToClient(row_id, rc);
    // std::string serial_str = serializeMsgToClient(rc);
    add_timing_point(OP_CREATE_VAR_BATCH_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          to_string(rc));
    add_timing_point(OP_CREATE_VAR_BATCH_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CREATE_VAR_BATCH_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}


// int md_create_var_batch_stub (sqlite3 *db, const vector<md_create_var_args> &args,
//                         uint64_t &row_id)
int md_create_var_batch_stub (sqlite3 *db, const vector<md_create_var_args> &all_args)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    char * ErrMsg = NULL;

    // rc = sqlite3_exec (db, "savepoint var;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error md_create_var_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    // }

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM var_catalog";
    // ////cout << "starting var catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // ////cout << "done with var catalog \n";

    // rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error begin md_create_var_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
    //     sqlite3_free (ErrMsg);
    //     sqlite3_close (db);
    //     goto cleanup;
    // }

    rc = sqlite3_prepare_v2 (db, "insert into var_catalog (id, run_id, timestep_id, name, path, version, data_size, num_dims, d0_min, d0_max, d1_min, d1_max, d2_min, d2_max) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of create_var_batch_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    for(int arg_indx = 0; arg_indx < all_args.size(); arg_indx++) {
        md_create_var_args args = all_args.at(arg_indx);
        // cout << "var_id: " << args.var_id << endl;
        // cout << "run_id: " << args.run_id << endl;
        // cout << "timestep_id: " << args.timestep_id << endl;

        rc = sqlite3_bind_int64 (stmt, 1, args.var_id); //assert (rc == SQLITE_OK);  
        // ////cout << "var id on create: " << args.var_id << endl; 
        rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK); 
        rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); //assert (rc == SQLITE_OK);   
        rc = sqlite3_bind_text (stmt, 4, strdup(args.name.c_str()), -1, free); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_text (stmt, 5, strdup(args.path.c_str()), -1, free); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 6, args.version); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 7, (int)args.data_size); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 8, args.num_dims); //assert (rc == SQLITE_OK);
        
        int i = 0;
        while( i < args.num_dims)
        {
            ////cout << "dim - i: " << i << endl;
            rc = sqlite3_bind_double (stmt, 9 + i * 2, args.dims.at(i).min); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_double (stmt, 10 + i * 2, args.dims.at(i).max); //assert (rc == SQLITE_OK);
            ////cout << args.dims.at(i).min << endl;
            ////cout << args.dims.at(i).max << endl;

            i++;
        }
        //explicitly bind null to any remaining dims so that there isn't any overflow from the prev insert
        while(i < 3) {
            ////cout << "null - i: " << i << endl;
            rc = sqlite3_bind_null (stmt, 9 + i * 2); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_null (stmt, 10 + i * 2); //assert (rc == SQLITE_OK);
            i++;
        }       

        rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
        
        // type_id = (int) sqlite3_last_insert_rowid (db);

        rc = sqlite3_reset(stmt); //assert (rc == SQLITE_OK);

        // row_id = (int) sqlite3_last_insert_rowid (db);
    }
//printf ("generated new global id:%d\n", *row_id);

    rc = sqlite3_finalize (stmt); 
    // if (rc != SQLITE_OK)
    // {
    //  cout << "rc: " << rc << endl;
    // }
    //assert (rc == SQLITE_OK);

    // rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    // if (rc != SQLITE_OK)
    // {
    //  // cout << "rc: " << rc << endl;
    //     fprintf (stderr, "Error end query md_create_var_batch_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
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